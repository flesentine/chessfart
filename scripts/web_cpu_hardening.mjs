import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-playtest/hardening';
fs.mkdirSync(outDir, { recursive: true });
const server = spawn('python3', ['-m', 'http.server', '8127', '--directory', 'build/web'], { stdio: 'ignore' });
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
const pieceValue = [0, 100, 320, 330, 500, 900, 20000];
const dirKeys = ['ArrowUp', 'PageUp', 'ArrowRight', 'PageDown', 'ArrowDown', 'End', 'ArrowLeft', 'Home'];
const dirNames = ['N','NE','E','SE','S','SW','W','NW'];
const statusNames = ['ONGOING','CHECK','CHECKMATE','STALEMATE','DRAW_50','DRAW_3FOLD','DRAW_MATERIAL'];
const difficultyNames = ['EASY','MEDIUM','HARD'];

async function call(page, name, ...args) {
  return await page.evaluate(({name,args}) => {
    const fn = Module[`_${name}`];
    if (!fn) throw new Error(`missing review export ${name}`);
    return fn(...args);
  }, {name,args});
}

async function historyLine(page, index) {
  return await page.evaluate((index) => Module.UTF8ToString(Module._cf_review_history_line(index)), index);
}

async function newestHistoryLine(page) {
  const count = await call(page, 'cf_review_history_count');
  return count > 0 ? await historyLine(page, count - 1) : '';
}

async function canvasShot(page, name) {
  const canvas = await page.$('#canvas');
  await canvas.screenshot({ path: path.join(outDir, `${name}.png`) });
}

async function canvasPoint(page, file, rank) {
  const canvas = await page.$('#canvas');
  const box = await canvas.boundingBox();
  const vx = 18 + file * 18 + 9;
  const vy = 27 + (7 - rank) * 18 + 9;
  return { x: box.x + vx / 320 * box.width, y: box.y + vy / 200 * box.height };
}

async function clickSquare(page, file, rank, button='left') {
  const p = await canvasPoint(page, file, rank);
  await page.mouse.click(p.x, p.y, {button, delay: 25});
  await sleep(80);
}

async function clickCanvasPixel(page, vx, vy, button='left') {
  const canvas = await page.$('#canvas');
  const box = await canvas.boundingBox();
  if (!box) throw new Error('canvas has no bounding box');
  await page.mouse.click(box.x + vx / 320 * box.width,
                         box.y + vy / 200 * box.height,
                         {button, delay:80});
  await sleep(140);
}

async function rewriteSaveAsLegacyV1(page) {
  await page.evaluate(() => {
    const savePath = '/persist/CHESSFRT.SAV';
    const current = Module.FS.readFile(savePath, {encoding:'utf8'});
    const legacy = current
      .replace(/^CHESSFART_SAVE 2$/m, 'CHESSFART_SAVE 1')
      .replace(/^MODE [^\n]*\n/m, '');
    if (legacy === current || /^MODE /m.test(legacy))
      throw new Error('failed to rewrite save as legacy v1');
    Module.FS.writeFile(savePath, legacy);
  });
  await sleep(100);
}

async function clickTitleItem(page, item) {
  const canvas = await page.$('#canvas');
  const box = await canvas.boundingBox();
  if (!box) throw new Error('canvas has no bounding box');
  const x = await call(page,'cf_review_title_menu_x');
  const w = await call(page,'cf_review_title_menu_width');
  const y = await call(page,'cf_review_title_menu_hit_y');
  const step = await call(page,'cf_review_title_menu_item_step');
  const vx = x + Math.floor(w / 2);
  const vy = y + item * step + Math.floor(step / 2);
  await page.mouse.click(box.x + vx / 320 * box.width,
                         box.y + vy / 200 * box.height,
                         {button:'left', delay:25});
  await sleep(180);
}

function decodePiece(code) {
  return { type: code & 15, color: (code >> 4) & 15, gas: (code >> 8) & 15 };
}

function decodeMove(code) {
  return { toFile: code & 7, toRank: (code >> 3) & 7, capture: (code >> 6) & 15,
           flags: (code >> 10) & 255, promotion: (code >> 18) & 15 };
}

function sq(f,r) { return String.fromCharCode(65+f) + String(r+1); }

async function boardState(page) {
  const board = [];
  for (let r=0;r<8;r++) {
    board[r] = [];
    for (let f=0;f<8;f++) board[r][f] = decodePiece(await call(page,'cf_review_piece',f,r));
  }
  return board;
}

function rngFor(seed0) {
  let seed = seed0 >>> 0;
  return () => {
    seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
    return seed / 4294967296;
  };
}

async function chooseAction(page, ply, rnd) {
  const board = await boardState(page);
  const actions = [];
  for (let r=0;r<8;r++) for (let f=0;f<8;f++) {
    const p = board[r][f];
    if (p.color !== 1) continue;
    const count = await call(page,'cf_review_move_count',f,r);
    for (let i=0;i<count;i++) {
      const m = decodeMove(await call(page,'cf_review_move_code',f,r,i));
      let score = pieceValue[m.capture] * 12;
      if (m.flags & 8) score += 8500;
      if (m.flags & 2 || m.flags & 4) score += 650;
      score += ((3.5-Math.abs(m.toFile-3.5)) + (3.5-Math.abs(m.toRank-3.5))) * 14;
      if ((p.type===2 || p.type===3) && r===0 && m.toRank>0) score += 180;
      if (p.type===1 && (m.toRank===3 || m.toRank===4)) score += 60;
      if (p.type===5 && ply<12) score -= 130;
      if (p.type===6 && !(m.flags&6) && ply<18) score -= 220;
      if (p.gas===1) score += 80;
      if (p.gas>=2) score -= 20;
      score += rnd()*100;
      actions.push({kind:'move',f,r,p,m,score,label:`${sq(f,r)}-${sq(m.toFile,m.toRank)}`});
    }
    if (p.gas >= 2) {
      for (let d=0;d<8;d++) {
        const preview = await call(page,'cf_review_fart_preview',f,r,d);
        if (preview===0) continue;
        const df=[0,1,1,1,0,-1,-1,-1][d], dr=[1,1,0,-1,-1,-1,0,1][d];
        const tf=f+df,tr=r+dr;
        const target=(tf>=0&&tf<8&&tr>=0&&tr<8)?board[tr][tf]:{type:0,color:0};
        let score;
        if (preview===4) score = 9000;
        else if (preview===2) score = 700 + (target.color===2 ? pieceValue[target.type]*1.35 : -pieceValue[target.type]*0.4);
        else if (preview===3) score = -220;
        else score = 35;
        score += rnd()*90;
        actions.push({kind:'fart',f,r,d,preview,score,label:`FART ${sq(f,r)} ${dirNames[d]} ${preview}`});
      }
    }
  }
  actions.sort((a,b)=>b.score-a.score);
  if (!actions.length) return null;
  const best = actions[0].score;
  const pool = actions.filter(a => a.score >= best - (best > 1000 ? 90 : 170)).slice(0,5);
  return pool[Math.floor(rnd()*pool.length)];
}

async function waitForCpu(page, timeoutMs) {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    const status = await call(page,'cf_review_status');
    const side = await call(page,'cf_review_side');
    const uiSynced = await call(page,'cf_review_ui_synced');
    if (uiSynced && (status >= 2 || side === 1)) return;
    await sleep(80);
  }
  throw new Error('CPU turn/UI sync timeout');
}

async function waitForMatchState(page, side, fullmove, timeoutMs=6000) {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    if (await call(page,'cf_review_side') === side &&
        await call(page,'cf_review_fullmove') === fullmove &&
        await call(page,'cf_review_ui_synced')) return;
    await sleep(80);
  }
  throw new Error(`match state timeout: side=${side} fullmove=${fullmove}`);
}

async function waitForStatus(page, status, side, fullmove, timeoutMs=6000) {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    if (await call(page,'cf_review_status') === status &&
        await call(page,'cf_review_side') === side &&
        await call(page,'cf_review_fullmove') === fullmove &&
        await call(page,'cf_review_ui_synced')) return;
    await sleep(80);
  }
  throw new Error(`status timeout: status=${status} side=${side} fullmove=${fullmove}`);
}

async function loadLocalFixture(page, fixture, side, fullmove=1) {
  if (await call(page,'cf_review_write_local_fixture',fixture) !== 1)
    throw new Error(`failed to write local fixture ${fixture}`);
  await page.keyboard.press('l');
  await sleep(220);
  await waitForMatchState(page,side,fullmove);
  if (await call(page,'cf_review_match_mode') !== 1)
    throw new Error(`fixture ${fixture} did not restore LOCAL mode`);
  if (await call(page,'cf_review_status') !== 0)
    throw new Error(`fixture ${fixture} did not load as ongoing`);
}

async function requireNewest(page, prefix) {
  const line = await newestHistoryLine(page);
  if (!line.startsWith(prefix))
    throw new Error(`history mismatch: expected ${prefix}, got ${line}`);
  return line;
}

async function verifyLocalEdgeHardening(browser) {
  const summary = [];
  const page = await browser.newPage();
  const errors = [];
  page.on('pageerror',e=>errors.push(`PAGE ${String(e)}`));
  page.on('console',m=>{ if(m.type()==='error') errors.push(`CONSOLE ${m.text()}`); });
  await page.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await page.goto('http://127.0.0.1:8127/?hardening=local-edge',
                  {waitUntil:'domcontentloaded',timeout:15000});
  await page.waitForFunction(
    ()=>document.getElementById('status')?.textContent.startsWith('Ready'),
    {timeout:15000}
  );
  await page.waitForFunction(
    ()=>typeof Module._cf_review_write_local_fixture==='function',
    {timeout:15000}
  );
  await page.keyboard.press('Enter');
  await sleep(450);

  /* Real Black Fart: D5 south pushes White D4 -> D3. */
  await loadLocalFixture(page,0,2,1);
  await clickSquare(page,3,4,'right');
  if (await call(page,'cf_review_fart_mode') !== 1)
    throw new Error('Black local Fart mode did not activate');
  await page.keyboard.press('ArrowDown');
  await sleep(100);
  await page.keyboard.press('Enter');
  await waitForMatchState(page,1,2);
  let p = decodePiece(await call(page,'cf_review_piece',3,2));
  let actor = decodePiece(await call(page,'cf_review_piece',3,4));
  if (p.type !== 1 || p.color !== 1 || p.gas !== 1)
    throw new Error('Black local Fart did not push White pawn D4-D3 with gas');
  if (actor.type !== 2 || actor.color !== 2 || actor.gas !== 1)
    throw new Error('Black local Fart actor/gas result incorrect');
  await requireNewest(page,'BLACK FART D5 S PUSH');
  await canvasShot(page,'local-black-fart-after');
  summary.push('LOCAL_BLACK_FART=PASS D5-S PUSH D4-D3 actorGas=1');

  /* Ordinary White promotion: prove Escape cancels instead of quitting, then
   * re-enter the same pending promotion and choose rook. */
  await loadLocalFixture(page,1,1,1);
  await clickSquare(page,1,6);
  await clickSquare(page,1,7);
  if (await call(page,'cf_review_promotion_pending') !== 1)
    throw new Error('White ordinary promotion did not enter choice state');
  if (await call(page,'cf_review_promotion_choice') !== 5)
    throw new Error('White ordinary promotion did not default to queen');
  await canvasShot(page,'local-white-promotion-pending');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===1,
    {timeout:3000}
  );
  if (await call(page,'cf_review_replay_count') !== 1 ||
      await call(page,'cf_review_replay_viewer_index') !== 0)
    throw new Error('pending ordinary promotion replay baseline mismatch');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===0,
    {timeout:3000}
  );
  if (await call(page,'cf_review_promotion_pending') !== 1 ||
      await call(page,'cf_review_promotion_choice') !== 5)
    throw new Error('replay did not restore pending ordinary promotion');
  await clickSquare(page,4,3,'left');
  await clickSquare(page,5,4,'right');
  await sleep(180);
  if (await call(page,'cf_review_promotion_pending') !== 1 ||
      await call(page,'cf_review_promotion_choice') !== 5 ||
      await call(page,'cf_review_side') !== 1 ||
      await call(page,'cf_review_fullmove') !== 1)
    throw new Error('board mouse input changed ordinary promotion choice/state');
  p = decodePiece(await call(page,'cf_review_piece',1,6));
  if (p.type !== 1 || p.color !== 1)
    throw new Error('board mouse input moved White pawn during promotion choice');
  await page.keyboard.press('Escape');
  await sleep(180);
  if (await call(page,'cf_review_promotion_pending') !== 0)
    throw new Error('Escape did not cancel ordinary promotion');
  if (await call(page,'cf_review_side') !== 1 ||
      await call(page,'cf_review_fullmove') !== 1)
    throw new Error('promotion cancel changed turn state');
  p = decodePiece(await call(page,'cf_review_piece',1,6));
  if (p.type !== 1 || p.color !== 1)
    throw new Error('promotion cancel moved the White pawn');
  await page.keyboard.press('Enter');
  await sleep(120);
  if (await call(page,'cf_review_promotion_pending') !== 1)
    throw new Error('White promotion could not be re-entered after cancel');
  await page.keyboard.press('ArrowRight');
  await sleep(80);
  await page.keyboard.press('Enter');
  await waitForMatchState(page,2,1);
  p = decodePiece(await call(page,'cf_review_piece',1,7));
  if (p.type !== 4 || p.color !== 1)
    throw new Error('White promotion did not produce rook on B8');
  await requireNewest(page,'WHITE B7-B8=R');
  if (await call(page,'cf_review_replay_count') !== 2 ||
      await call(page,'cf_review_replay_status',1) !== 1)
    throw new Error('ordinary promotion replay frame did not preserve CHECK');
  p = decodePiece(await call(page,'cf_review_replay_piece',1,1,7));
  if (p.type !== 4 || p.color !== 1)
    throw new Error('ordinary promotion replay frame lost promoted rook');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===1 &&
       Module._cf_review_replay_viewer_index()===1,
    {timeout:3000}
  );
  await canvasShot(page,'replay-white-promotion');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===0,
    {timeout:3000}
  );
  summary.push('LOCAL_WHITE_PROMOTION=PASS mouse-locked replay-pending+committed B7-B8=R');

  /* Ordinary Black promotion through the same local input path. */
  await loadLocalFixture(page,2,2,1);
  await clickSquare(page,1,1);
  await clickSquare(page,1,0);
  if (await call(page,'cf_review_promotion_pending') !== 1)
    throw new Error('Black ordinary promotion did not enter choice state');
  await page.keyboard.press('ArrowRight');
  await sleep(80);
  await page.keyboard.press('Enter');
  await waitForMatchState(page,1,2);
  p = decodePiece(await call(page,'cf_review_piece',1,0));
  if (p.type !== 4 || p.color !== 2)
    throw new Error('Black promotion did not produce rook on B1');
  await requireNewest(page,'BLACK B2-B1=R');
  summary.push('LOCAL_BLACK_PROMOTION=PASS B2-B1=R');

  /* White Fart-push promotion D7 -> D8. */
  await loadLocalFixture(page,3,1,1);
  await clickSquare(page,3,5,'right');
  if (await call(page,'cf_review_fart_mode') !== 1)
    throw new Error('White Fart-promotion mode did not activate');
  await page.keyboard.press('ArrowUp');
  await sleep(80);
  await page.keyboard.press('Enter');
  await sleep(140);
  if (await call(page,'cf_review_fart_promotion_pending') !== 1)
    throw new Error('White Fart-push promotion did not enter choice state');
  if (await call(page,'cf_review_fart_promotion_choice') !== 5)
    throw new Error('White Fart-push promotion did not default to queen');
  await canvasShot(page,'local-white-fart-promotion-pending');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===1,
    {timeout:3000}
  );
  if (await call(page,'cf_review_replay_count') !== 1 ||
      await call(page,'cf_review_replay_viewer_index') !== 0)
    throw new Error('pending Fart-promotion replay baseline mismatch');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===0,
    {timeout:3000}
  );
  if (await call(page,'cf_review_fart_promotion_pending') !== 1 ||
      await call(page,'cf_review_fart_promotion_choice') !== 5)
    throw new Error('replay did not restore pending Fart promotion');
  await clickSquare(page,0,0,'left');
  await clickSquare(page,7,7,'right');
  await sleep(180);
  if (await call(page,'cf_review_fart_promotion_pending') !== 1 ||
      await call(page,'cf_review_fart_promotion_choice') !== 5 ||
      await call(page,'cf_review_side') !== 1 ||
      await call(page,'cf_review_fullmove') !== 1)
    throw new Error('board mouse input changed Fart-promotion choice/state');
  await page.keyboard.press('ArrowRight');
  await sleep(80);
  await page.keyboard.press('Enter');
  await waitForMatchState(page,2,1);
  p = decodePiece(await call(page,'cf_review_piece',3,7));
  actor = decodePiece(await call(page,'cf_review_piece',3,5));
  if (p.type !== 4 || p.color !== 1 || p.gas !== 2)
    throw new Error('White Fart-push promotion result/gas incorrect');
  if (actor.type !== 2 || actor.color !== 1 || actor.gas !== 1)
    throw new Error('White Fart-promotion actor gas incorrect');
  await requireNewest(page,'WHITE FART D6 N =R');
  if (await call(page,'cf_review_replay_count') !== 2 ||
      await call(page,'cf_review_replay_status',1) !== 1)
    throw new Error('Fart promotion replay frame did not preserve CHECK');
  p = decodePiece(await call(page,'cf_review_replay_piece',1,3,7));
  actor = decodePiece(await call(page,'cf_review_replay_piece',1,3,5));
  if (p.type !== 4 || p.color !== 1 || p.gas !== 2 ||
      actor.type !== 2 || actor.color !== 1 || actor.gas !== 1)
    throw new Error('Fart promotion replay frame piece/Gas mismatch');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===1 &&
       Module._cf_review_replay_viewer_index()===1,
    {timeout:3000}
  );
  await canvasShot(page,'replay-white-fart-promotion');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===0,
    {timeout:3000}
  );
  summary.push('LOCAL_WHITE_FART_PROMOTION=PASS mouse-locked replay-pending+committed D7-D8=R');

  /* Black Fart-push promotion D2 -> D1. */
  await loadLocalFixture(page,4,2,1);
  await clickSquare(page,3,2,'right');
  if (await call(page,'cf_review_fart_mode') !== 1)
    throw new Error('Black Fart-promotion mode did not activate');
  await page.keyboard.press('ArrowDown');
  await sleep(80);
  await page.keyboard.press('Enter');
  await sleep(140);
  if (await call(page,'cf_review_fart_promotion_pending') !== 1)
    throw new Error('Black Fart-push promotion did not enter choice state');
  await page.keyboard.press('ArrowRight');
  await sleep(80);
  await page.keyboard.press('Enter');
  await waitForMatchState(page,1,2);
  p = decodePiece(await call(page,'cf_review_piece',3,0));
  actor = decodePiece(await call(page,'cf_review_piece',3,2));
  if (p.type !== 4 || p.color !== 2 || p.gas !== 2)
    throw new Error('Black Fart-push promotion result/gas incorrect');
  if (actor.type !== 2 || actor.color !== 2 || actor.gas !== 1)
    throw new Error('Black Fart-promotion actor gas incorrect');
  await requireNewest(page,'BLACK FART D3 S =R');
  await canvasShot(page,'local-black-fart-promotion-after');
  summary.push('LOCAL_BLACK_FART_PROMOTION=PASS D2-D1=R');

  /* White delivers checkmate, then Black cannot select/Fart/move. */
  await loadLocalFixture(page,5,1,1);
  await clickSquare(page,6,5);
  await clickSquare(page,6,6);
  await waitForStatus(page,2,2,1);
  await requireNewest(page,'WHITE G6-G7');
  const whiteMateHistory = await call(page,'cf_review_history_count');
  await canvasShot(page,'local-white-checkmate');
  if (await call(page,'cf_review_replay_count') !== 2 ||
      await call(page,'cf_review_replay_status',1) !== 2)
    throw new Error('checkmate replay frame status mismatch');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===1 &&
       Module._cf_review_replay_viewer_index()===1,
    {timeout:3000}
  );
  await canvasShot(page,'replay-white-checkmate');
  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===0,
    {timeout:3000}
  );
  if (await call(page,'cf_review_status') !== 2 ||
      await call(page,'cf_review_history_count') !== whiteMateHistory)
    throw new Error('postgame replay did not restore terminal game');
  await clickSquare(page,7,7);
  await page.keyboard.press('f');
  await page.keyboard.press('Enter');
  await sleep(180);
  if (await call(page,'cf_review_status') !== 2 ||
      await call(page,'cf_review_side') !== 2 ||
      await call(page,'cf_review_fullmove') !== 1 ||
      await call(page,'cf_review_history_count') !== whiteMateHistory)
    throw new Error('terminal guard allowed action after White checkmate');
  p = decodePiece(await call(page,'cf_review_piece',6,6));
  if (p.type !== 5 || p.color !== 1)
    throw new Error('White checkmate board changed after terminal input');
  summary.push('LOCAL_WHITE_CHECKMATE=PASS terminal-lock');

  /* Black delivers the mirrored checkmate and White is likewise locked. */
  await loadLocalFixture(page,6,2,1);
  await clickSquare(page,6,2);
  await clickSquare(page,6,1);
  await waitForStatus(page,2,1,2);
  await requireNewest(page,'BLACK G3-G2');
  const blackMateHistory = await call(page,'cf_review_history_count');
  await canvasShot(page,'local-black-checkmate');
  await clickSquare(page,7,0);
  await page.keyboard.press('f');
  await page.keyboard.press('Enter');
  await sleep(180);
  if (await call(page,'cf_review_status') !== 2 ||
      await call(page,'cf_review_side') !== 1 ||
      await call(page,'cf_review_fullmove') !== 2 ||
      await call(page,'cf_review_history_count') !== blackMateHistory)
    throw new Error('terminal guard allowed action after Black checkmate');
  p = decodePiece(await call(page,'cf_review_piece',6,1));
  if (p.type !== 5 || p.color !== 2)
    throw new Error('Black checkmate board changed after terminal input');
  summary.push('LOCAL_BLACK_CHECKMATE=PASS terminal-lock');

  if (errors.length) throw new Error(`local-edge: ${errors.join(' | ')}`);
  await page.close();
  return summary;
}

async function verifyMatchModes(browser) {
  const summary = [];

  const keyboard = await browser.newPage();
  await keyboard.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await keyboard.goto('http://127.0.0.1:8127/?hardening=title-keyboard-local',{waitUntil:'domcontentloaded',timeout:15000});
  await keyboard.waitForFunction(()=>document.getElementById('status')?.textContent.startsWith('Ready'),{timeout:15000});
  await keyboard.waitForFunction(()=>typeof Module._cf_review_match_mode==='function',{timeout:15000});
  await keyboard.keyboard.press('ArrowDown');
  await keyboard.keyboard.press('Enter');
  await sleep(500);
  if (await call(keyboard,'cf_review_match_mode') !== 1)
    throw new Error('keyboard 2 PLAYERS selection did not enter local mode');
  summary.push('TITLE_KEYBOARD_LOCAL=PASS');
  await keyboard.close();

  const local = await browser.newPage();
  const localErrors = [];
  local.on('pageerror',e=>localErrors.push(`PAGE ${String(e)}`));
  local.on('console',m=>{ if(m.type()==='error') localErrors.push(`CONSOLE ${m.text()}`); });
  await local.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await local.goto('http://127.0.0.1:8127/?hardening=match-local',{waitUntil:'domcontentloaded',timeout:15000});
  await local.waitForFunction(()=>document.getElementById('status')?.textContent.startsWith('Ready'),{timeout:15000});
  await local.waitForFunction(()=>typeof Module._cf_review_match_mode==='function',{timeout:15000});
  if (await call(local,'cf_review_match_mode') !== 0) throw new Error('default match mode is not CPU');
  await clickTitleItem(local,1);
  if (await call(local,'cf_review_match_mode') !== 1)
    throw new Error('mouse 2 PLAYERS selection did not enter local mode');
  await waitForMatchState(local,1,1);
  summary.push('TITLE_MOUSE_LOCAL=PASS');

  /* Save a local game while Black is to move. Loading it must not wake the CPU. */
  await clickSquare(local,4,1);
  await clickSquare(local,4,3);
  await waitForMatchState(local,2,1);
  let piece = decodePiece(await call(local,'cf_review_piece',4,3));
  if (piece.type !== 1 || piece.color !== 1) throw new Error('local White e2-e4 did not land');
  await canvasShot(local,'match-local-black-to-move-hud');
  await local.keyboard.press('s');
  await sleep(250);

  await clickSquare(local,4,6);
  await clickSquare(local,4,4);
  await waitForMatchState(local,1,2);
  piece = decodePiece(await call(local,'cf_review_piece',4,4));
  if (piece.type !== 1 || piece.color !== 2) throw new Error('local Black e7-e5 did not land');
  if (await call(local,'cf_review_history_has_local_pair') !== 1)
    throw new Error('local history did not record WHITE then BLACK');
  await local.keyboard.press('m');
  await sleep(220);
  await canvasShot(local,'match-local-history-white-black');
  await local.keyboard.press('Enter');
  await sleep(180);

  await call(local,'cf_review_set_match_mode',0);
  if (await call(local,'cf_review_match_mode') !== 0)
    throw new Error('pre-load CPU mode switch did not stick');
  await local.keyboard.press('l');
  await waitForMatchState(local,2,1);
  if (await call(local,'cf_review_match_mode') !== 1)
    throw new Error('local v2 save did not restore LOCAL mode');
  piece = decodePiece(await call(local,'cf_review_piece',4,6));
  if (piece.type !== 1 || piece.color !== 2) throw new Error('local load did not restore Black pawn on e7');
  piece = decodePiece(await call(local,'cf_review_piece',4,4));
  if (piece.type !== 0) throw new Error('local load did not remove later Black e7-e5');
  await canvasShot(local,'match-local-after-load-black-to-move');

  /* The restored Black turn must remain playable by the second local player. */
  await clickSquare(local,4,6);
  await clickSquare(local,4,4);
  await waitForMatchState(local,1,2);
  if (localErrors.length) throw new Error(`match-local: ${localErrors.join(' | ')}`);
  summary.push('MATCH_LOCAL=PASS white=e2-e4 black=e7-e5 history=WHITE/BLACK v2-restored=LOCAL');
  await local.close();

  const cpu = await browser.newPage();
  const cpuErrors = [];
  cpu.on('pageerror',e=>cpuErrors.push(`PAGE ${String(e)}`));
  cpu.on('console',m=>{ if(m.type()==='error') cpuErrors.push(`CONSOLE ${m.text()}`); });
  await cpu.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await cpu.goto('http://127.0.0.1:8127/?hardening=match-cpu',{waitUntil:'domcontentloaded',timeout:15000});
  await cpu.waitForFunction(()=>document.getElementById('status')?.textContent.startsWith('Ready'),{timeout:15000});
  await cpu.waitForFunction(()=>typeof Module._cf_review_match_mode==='function',{timeout:15000});
  if (await call(cpu,'cf_review_match_mode') !== 0) throw new Error('fresh CPU match mode changed');

  /* Create a Black-to-move board under local control, save it explicitly as
   * CPU mode, then switch to LOCAL before loading. The v2 save must restore
   * CPU mode before the load hook decides whether Black should auto-move. */
  await cpu.keyboard.press('Enter');
  await sleep(500);
  await call(cpu,'cf_review_set_match_mode',1);
  if (await call(cpu,'cf_review_match_mode') !== 1)
    throw new Error('review local mode did not stick after CPU title selection');
  await clickSquare(cpu,4,1);
  await clickSquare(cpu,4,3);
  await waitForMatchState(cpu,2,1);
  await call(cpu,'cf_review_set_match_mode',0);
  await cpu.keyboard.press('s');
  await sleep(250);
  await call(cpu,'cf_review_set_match_mode',1);
  await cpu.keyboard.press('l');
  await waitForMatchState(cpu,1,2);
  if (await call(cpu,'cf_review_match_mode') !== 0)
    throw new Error('CPU v2 save did not restore CPU mode');
  if (await call(cpu,'cf_review_replay_count') !== 2 ||
      await call(cpu,'cf_review_replay_total') !== 2 ||
      await call(cpu,'cf_review_replay_mode',0) !== 0 ||
      await call(cpu,'cf_review_replay_mode',1) !== 0 ||
      await call(cpu,'cf_review_replay_side',0) !== 2 ||
      await call(cpu,'cf_review_replay_side',1) !== 1 ||
      await call(cpu,'cf_review_replay_fullmove',0) !== 1 ||
      await call(cpu,'cf_review_replay_fullmove',1) !== 2 ||
      await call(cpu,'cf_review_replay_last_matches_current') !== 1)
    throw new Error('CPU black-to-move load replay did not reset to baseline+reply');
  await canvasShot(cpu,'match-cpu-after-loading-black-turn');
  if (cpuErrors.length) throw new Error(`match-cpu: ${cpuErrors.join(' | ')}`);
  summary.push('MATCH_CPU=PASS v2-restored=CPU black-to-move-load-triggered-reply=1');
  await cpu.close();

  const legacy = await browser.newPage();
  const legacyErrors = [];
  legacy.on('pageerror',e=>legacyErrors.push(`PAGE ${String(e)}`));
  legacy.on('console',m=>{ if(m.type()==='error') legacyErrors.push(`CONSOLE ${m.text()}`); });
  await legacy.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await legacy.goto('http://127.0.0.1:8127/?hardening=match-legacy-v1',{waitUntil:'domcontentloaded',timeout:15000});
  await legacy.waitForFunction(()=>document.getElementById('status')?.textContent.startsWith('Ready'),{timeout:15000});
  await legacy.waitForFunction(()=>typeof Module._cf_review_match_mode==='function',{timeout:15000});
  await legacy.keyboard.press('Enter');
  await sleep(500);
  await call(legacy,'cf_review_set_match_mode',1);
  await clickSquare(legacy,4,1);
  await clickSquare(legacy,4,3);
  await waitForMatchState(legacy,2,1);
  await legacy.keyboard.press('s');
  await sleep(250);
  await rewriteSaveAsLegacyV1(legacy);
  if (await call(legacy,'cf_review_match_mode') !== 1)
    throw new Error('legacy pre-load LOCAL mode changed unexpectedly');
  await legacy.keyboard.press('l');
  await waitForMatchState(legacy,1,2);
  if (await call(legacy,'cf_review_match_mode') !== 0)
    throw new Error('legacy v1 save did not default to CPU mode');
  await canvasShot(legacy,'match-legacy-v1-after-load');
  if (legacyErrors.length) throw new Error(`match-legacy-v1: ${legacyErrors.join(' | ')}`);
  summary.push('MATCH_LEGACY_V1=PASS defaulted=CPU black-to-move-load-triggered-reply=1');
  await legacy.close();

  return summary;
}

async function verifyReplayTimeline(browser) {
  const local = await browser.newPage();
  const errors = [];
  local.on('pageerror',e=>errors.push(`PAGE ${String(e)}`));
  local.on('console',m=>{ if(m.type()==='error') errors.push(`CONSOLE ${m.text()}`); });
  await local.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await local.goto('http://127.0.0.1:8127/?hardening=replay-local',
                   {waitUntil:'domcontentloaded',timeout:15000});
  await local.waitForFunction(
    ()=>document.getElementById('status')?.textContent.startsWith('Ready'),
    {timeout:15000}
  );
  await local.waitForFunction(
    ()=>typeof Module._cf_review_replay_count==='function',
    {timeout:15000}
  );
  await local.keyboard.press('ArrowDown');
  await local.keyboard.press('Enter');
  await sleep(450);
  await waitForMatchState(local,1,1);

  if (await call(local,'cf_review_replay_count') !== 1 ||
      await call(local,'cf_review_replay_total') !== 1 ||
      await call(local,'cf_review_replay_truncated') !== 0 ||
      await call(local,'cf_review_replay_mode',0) !== 1 ||
      await call(local,'cf_review_replay_side',0) !== 1 ||
      await call(local,'cf_review_replay_fullmove',0) !== 1 ||
      await call(local,'cf_review_replay_last_matches_current') !== 1)
    throw new Error('local replay baseline mismatch');

  let p = decodePiece(await call(local,'cf_review_replay_piece',0,4,1));
  if (p.type !== 1 || p.color !== 1 || p.gas !== 0)
    throw new Error('local replay baseline lost White e2 pawn');

  await clickSquare(local,4,1);
  await clickSquare(local,4,3);
  await waitForMatchState(local,2,1);
  if (await call(local,'cf_review_replay_count') !== 2 ||
      await call(local,'cf_review_replay_side',1) !== 2 ||
      await call(local,'cf_review_replay_fullmove',1) !== 1 ||
      await call(local,'cf_review_replay_last_matches_current') !== 1)
    throw new Error('local replay did not record White e2-e4');
  p = decodePiece(await call(local,'cf_review_replay_piece',1,4,3));
  if (p.type !== 1 || p.color !== 1 || p.gas !== 1)
    throw new Error('local replay White e4 snapshot piece/Gas mismatch');

  await clickSquare(local,4,6);
  await clickSquare(local,4,4);
  await waitForMatchState(local,1,2);
  if (await call(local,'cf_review_replay_count') !== 3 ||
      await call(local,'cf_review_replay_side',2) !== 1 ||
      await call(local,'cf_review_replay_fullmove',2) !== 2 ||
      await call(local,'cf_review_replay_last_matches_current') !== 1)
    throw new Error('local replay did not record Black e7-e5');

  /* Selection, History and Save are not committed turns. */
  await clickSquare(local,6,0);
  if (await call(local,'cf_review_replay_count') !== 3)
    throw new Error('piece selection created a replay frame');
  await clickSquare(local,6,0);
  if (await call(local,'cf_review_replay_count') !== 3)
    throw new Error('selection cancel created a replay frame');
  await local.keyboard.press('m');
  await sleep(140);
  await local.keyboard.press('Enter');
  await sleep(120);
  await local.keyboard.press('s');
  await sleep(180);
  if (await call(local,'cf_review_replay_count') !== 3)
    throw new Error('modal/save activity created a replay frame');

  /* Make one more turn, then Load. Save v2 does not serialize replay
   * history, so a successful load intentionally becomes a new baseline. */
  await clickSquare(local,6,0);
  await clickSquare(local,5,2);
  await waitForMatchState(local,2,2);
  if (await call(local,'cf_review_replay_count') !== 4)
    throw new Error('local replay did not record Nf3');
  await local.keyboard.press('l');
  await sleep(220);
  await waitForMatchState(local,1,2);
  if (await call(local,'cf_review_replay_count') !== 1 ||
      await call(local,'cf_review_replay_total') !== 1 ||
      await call(local,'cf_review_replay_mode',0) !== 1 ||
      await call(local,'cf_review_replay_side',0) !== 1 ||
      await call(local,'cf_review_replay_fullmove',0) !== 2 ||
      await call(local,'cf_review_replay_last_matches_current') !== 1)
    throw new Error('successful local load did not reset replay baseline');

  if (errors.length) throw new Error(`replay-local: ${errors.join(' | ')}`);
  await local.close();

  const cpu = await browser.newPage();
  const cpuErrors = [];
  cpu.on('pageerror',e=>cpuErrors.push(`PAGE ${String(e)}`));
  cpu.on('console',m=>{ if(m.type()==='error') cpuErrors.push(`CONSOLE ${m.text()}`); });
  await cpu.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await cpu.goto('http://127.0.0.1:8127/?hardening=replay-cpu',
                 {waitUntil:'domcontentloaded',timeout:15000});
  await cpu.waitForFunction(
    ()=>document.getElementById('status')?.textContent.startsWith('Ready'),
    {timeout:15000}
  );
  await cpu.waitForFunction(
    ()=>typeof Module._cf_review_replay_count==='function',
    {timeout:15000}
  );
  await cpu.keyboard.press('Enter');
  await sleep(450);
  await waitForMatchState(cpu,1,1);
  if (await call(cpu,'cf_review_replay_count') !== 1 ||
      await call(cpu,'cf_review_replay_mode',0) !== 0)
    throw new Error('CPU replay baseline mismatch');

  await clickSquare(cpu,4,1);
  await clickSquare(cpu,4,3);
  await waitForMatchState(cpu,1,2);
  if (await call(cpu,'cf_review_replay_count') !== 3 ||
      await call(cpu,'cf_review_replay_total') !== 3 ||
      await call(cpu,'cf_review_replay_mode',1) !== 0 ||
      await call(cpu,'cf_review_replay_mode',2) !== 0 ||
      await call(cpu,'cf_review_replay_side',1) !== 2 ||
      await call(cpu,'cf_review_replay_side',2) !== 1 ||
      await call(cpu,'cf_review_replay_last_matches_current') !== 1)
    throw new Error('CPU replay did not record human+CPU turn pair');

  if (cpuErrors.length) throw new Error(`replay-cpu: ${cpuErrors.join(' | ')}`);
  await cpu.close();

  return 'REPLAY_TIMELINE=PASS local-start+2=3 passive-input=no-frame load-reset=1 cpu-start+pair=3';
}

async function verifyReplayViewer(browser) {
  const page = await browser.newPage();
  const errors = [];
  page.on('pageerror',e=>errors.push(`PAGE ${String(e)}`));
  page.on('console',m=>{ if(m.type()==='error') errors.push(`CONSOLE ${m.text()}`); });
  await page.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await page.goto('http://127.0.0.1:8127/?hardening=replay-viewer',
                  {waitUntil:'domcontentloaded',timeout:15000});
  await page.waitForFunction(
    ()=>document.getElementById('status')?.textContent.startsWith('Ready'),
    {timeout:15000}
  );
  await page.waitForFunction(
    ()=>typeof Module._cf_review_replay_viewer_active==='function',
    {timeout:15000}
  );

  await page.keyboard.press('ArrowDown');
  await page.keyboard.press('Enter');
  await sleep(450);
  await waitForMatchState(page,1,1);
  await clickSquare(page,4,1);
  await clickSquare(page,4,3);
  await waitForMatchState(page,2,1);
  await clickSquare(page,4,6);
  await clickSquare(page,4,4);
  await waitForMatchState(page,1,2);

  const before = await persistedState(page);
  const count = await call(page,'cf_review_replay_count');
  const total = await call(page,'cf_review_replay_total');
  if (count !== 3 || total !== 3)
    throw new Error('replay viewer setup expected three frames');

  await page.keyboard.press('r');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===1,
    {timeout:5000}
  );
  if (await call(page,'cf_review_replay_viewer_index') !== 2)
    throw new Error('replay viewer did not open on latest frame');
  await canvasShot(page,'replay-viewer-latest');

  await page.keyboard.press('ArrowLeft');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_index()===1,
    {timeout:3000}
  );
  let p = decodePiece(await call(page,'cf_review_replay_piece',1,4,3));
  if (p.type !== 1 || p.color !== 1 || p.gas !== 1)
    throw new Error('replay viewer step did not land on White e4 frame');

  /* Replay owns input while open. Gameplay keys and board clicks must be
   * consumed without touching the live game, history, or replay position. */
  for (const key of ['f','s','l','h']) {
    await page.keyboard.press(key);
    await sleep(70);
  }
  await clickCanvasPixel(page,63,72);
  await clickCanvasPixel(page,153,126,'right');
  if (await call(page,'cf_review_replay_viewer_active') !== 1 ||
      await call(page,'cf_review_replay_viewer_index') !== 1)
    throw new Error('gameplay input escaped the replay viewer');
  const locked = await persistedState(page);
  if (!samePersistedState(before,locked) ||
      await call(page,'cf_review_replay_count') !== count ||
      await call(page,'cf_review_replay_total') !== total)
    throw new Error('replay viewer gameplay lock mutated live state');

  /* Keyboard bounds remain clamped. */
  await page.keyboard.press('ArrowLeft');
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_index()===0,
    {timeout:3000}
  );
  await page.keyboard.press('ArrowLeft');
  await sleep(80);
  if (await call(page,'cf_review_replay_viewer_index') !== 0)
    throw new Error('replay viewer stepped before oldest available frame');

  /* Footer mouse zones own Previous and Next. */
  await clickCanvasPixel(page,50,190);
  if (await call(page,'cf_review_replay_viewer_index') !== 0)
    throw new Error('replay mouse previous stepped before oldest frame');
  await clickCanvasPixel(page,150,190);
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_index()===1,
    {timeout:3000}
  );
  await clickCanvasPixel(page,150,190);
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_index()===2,
    {timeout:3000}
  );
  await clickCanvasPixel(page,150,190);
  if (await call(page,'cf_review_replay_viewer_index') !== 2)
    throw new Error('replay mouse next stepped past newest frame');
  await clickCanvasPixel(page,50,190);
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_index()===1,
    {timeout:3000}
  );

  const during = await persistedState(page);
  if (!samePersistedState(before,during))
    throw new Error('replay viewer mutated live persisted state while open');
  if (await call(page,'cf_review_replay_count') !== count ||
      await call(page,'cf_review_replay_total') !== total)
    throw new Error('replay viewer created or removed replay frames');

  /* Close through the footer mouse region and require exact live restore. */
  await clickCanvasPixel(page,270,190);
  await page.waitForFunction(
    ()=>Module._cf_review_replay_viewer_active()===0,
    {timeout:5000}
  );
  await sleep(120);
  const after = await persistedState(page);
  if (!samePersistedState(before,after) ||
      await call(page,'cf_review_replay_last_matches_current') !== 1 ||
      await call(page,'cf_review_ui_synced') !== 1)
    throw new Error('replay viewer did not restore exact live game on close');

  if (errors.length) throw new Error(`replay-viewer: ${errors.join(' | ')}`);
  await page.close();
  return 'REPLAY_VIEWER=PASS open=latest step=bounded mouse=prev-next-close live-input=locked close=restored';
}

function samePersistedState(a, b) {
  return JSON.stringify(a) === JSON.stringify(b);
}

async function persistedState(page) {
  return await page.evaluate(() => {
    const piece = Module._cf_review_piece;
    const historyCount = Module._cf_review_gas_history_count();
    const squares = [];
    const history = [];
    for (let rank=0; rank<8; ++rank) {
      const row = [];
      for (let file=0; file<8; ++file) {
        row.push({
          piece:piece(file,rank),
          gas:Module._cf_review_gas(file,rank)
        });
      }
      squares.push(row);
    }
    for (let i=0; i<historyCount; ++i) {
      const keySquares = [];
      for (let s=0; s<64; ++s)
        keySquares.push(Module._cf_review_gas_history_square(i,s));
      history.push({
        state:Module._cf_review_gas_history_state(i),
        ep:Module._cf_review_gas_history_ep(i),
        squares:keySquares
      });
    }
    return {
      mode:Module._cf_review_match_mode(),
      side:Module._cf_review_side(),
      castling:Module._cf_review_castling_rights(),
      epFile:Module._cf_review_en_passant_file(),
      epRank:Module._cf_review_en_passant_rank(),
      halfmove:Module._cf_review_halfmove(),
      fullmove:Module._cf_review_fullmove(),
      squares,
      history
    };
  });
}

function operaExpectedStatus(ply) {
  if (ply === 20 || ply === 28 || ply === 30) return 1;
  if (ply === 32) return 2;
  return 0;
}

async function playLocalMove(page, fromFile, fromRank, toFile, toRank,
                             actor, expectedSide, expectedFullmove,
                             expectedStatus=0) {
  await clickSquare(page,fromFile,fromRank);
  await clickSquare(page,toFile,toRank);
  if (expectedStatus >= 2)
    await waitForStatus(page,expectedStatus,expectedSide,expectedFullmove);
  else
    await waitForMatchState(page,expectedSide,expectedFullmove);

  if (await call(page,'cf_review_match_mode') !== 1)
    throw new Error('full local game left LOCAL mode');
  const status = await call(page,'cf_review_status');
  if (status !== expectedStatus)
    throw new Error(`full local status mismatch after ${actor} move: ${status} != ${expectedStatus}`);
  const line = await newestHistoryLine(page);
  if (!line.startsWith(`${actor} `))
    throw new Error(`full local history actor mismatch: expected ${actor}, got ${line}`);
  return line;
}

async function verifyLocalFullGame(browser) {
  const page = await browser.newPage();
  const errors = [];
  page.on('pageerror',e=>errors.push(`PAGE ${String(e)}`));
  page.on('console',m=>{ if(m.type()==='error') errors.push(`CONSOLE ${m.text()}`); });
  await page.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await page.goto('http://127.0.0.1:8127/?hardening=local-full-game',
                  {waitUntil:'domcontentloaded',timeout:15000});
  await page.waitForFunction(
    ()=>document.getElementById('status')?.textContent.startsWith('Ready'),
    {timeout:15000}
  );
  await page.waitForFunction(
    ()=>typeof Module._cf_review_match_mode==='function',
    {timeout:15000}
  );

  /* Select 2 PLAYERS from the actual title with keyboard input. */
  await page.keyboard.press('ArrowDown');
  await page.keyboard.press('Enter');
  await sleep(450);
  await waitForMatchState(page,1,1);
  if (await call(page,'cf_review_match_mode') !== 1)
    throw new Error('full local game did not enter LOCAL mode');
  await canvasShot(page,'local-full-00-opening');

  /*
   * Paul Morphy's Opera Game, played entirely through real board mouse input.
   * This is 33 plies from the normal starting position to checkmate and
   * exercises captures, both colors, queenside castling, check and mate.
   */
  const moves = [
    [4,1,4,3,'WHITE'], [4,6,4,4,'BLACK'],
    [6,0,5,2,'WHITE'], [3,6,3,5,'BLACK'],
    [3,1,3,3,'WHITE'], [2,7,6,3,'BLACK'],
    [3,3,4,4,'WHITE'], [6,3,5,2,'BLACK'],
    [3,0,5,2,'WHITE'], [3,5,4,4,'BLACK'],
    [5,0,2,3,'WHITE'], [6,7,5,5,'BLACK'],
    [5,2,1,2,'WHITE'], [3,7,4,6,'BLACK'],
    [1,0,2,2,'WHITE'], [2,6,2,5,'BLACK'],
    [2,0,6,4,'WHITE'], [1,6,1,4,'BLACK'],
    [2,2,1,4,'WHITE'], [2,5,1,4,'BLACK'],
    [2,3,1,4,'WHITE'], [1,7,3,6,'BLACK'],
    [4,0,2,0,'WHITE'], [0,7,3,7,'BLACK'],
    [3,0,3,6,'WHITE'], [3,7,3,6,'BLACK'],
    [7,0,3,0,'WHITE'], [4,6,4,5,'BLACK'],
    [1,4,3,6,'WHITE'], [5,5,3,6,'BLACK'],
    [1,2,1,7,'WHITE'], [3,6,1,7,'BLACK'],
    [3,0,3,7,'WHITE']
  ];

  let ply = 0;
  for (; ply < 16; ++ply) {
    const m = moves[ply];
    const afterSide = m[4] === 'WHITE' ? 2 : 1;
    const afterFullmove = m[4] === 'WHITE' ? 1 + Math.floor(ply / 2) :
                                              2 + Math.floor(ply / 2);
    await playLocalMove(page,m[0],m[1],m[2],m[3],m[4],
                        afterSide,afterFullmove,operaExpectedStatus(ply));
  }

  /* After 8...c6 it is White to move on move 9. Save the exact local game,
   * play two more plies, then load and prove the full persisted state is exact. */
  if (await call(page,'cf_review_side') !== 1 ||
      await call(page,'cf_review_fullmove') !== 9)
    throw new Error('Opera checkpoint is not White move 9');
  const checkpointState = await persistedState(page);
  await page.keyboard.press('s');
  await sleep(180);
  await canvasShot(page,'local-full-16-checkpoint-save');

  for (; ply < 18; ++ply) {
    const m = moves[ply];
    const afterSide = m[4] === 'WHITE' ? 2 : 1;
    const afterFullmove = m[4] === 'WHITE' ? 9 : 10;
    await playLocalMove(page,m[0],m[1],m[2],m[3],m[4],
                        afterSide,afterFullmove,operaExpectedStatus(ply));
  }
  if (samePersistedState(checkpointState, await persistedState(page)))
    throw new Error('Opera checkpoint did not change after two plies');

  await page.keyboard.press('l');
  await sleep(220);
  await waitForMatchState(page,1,9);
  if (await call(page,'cf_review_match_mode') !== 1)
    throw new Error('Opera checkpoint load did not restore LOCAL');
  if (!samePersistedState(checkpointState, await persistedState(page)))
    throw new Error('Opera checkpoint load did not restore exact persisted state');
  await canvasShot(page,'local-full-18-after-load');

  /* Replay move 9 after rollback, then continue the game to mate. */
  ply = 16;
  for (; ply < moves.length; ++ply) {
    const m = moves[ply];
    const moveNo = 1 + Math.floor(ply / 2);
    const afterSide = m[4] === 'WHITE' ? 2 : 1;
    const afterFullmove = m[4] === 'WHITE' ? moveNo : moveNo + 1;
    await playLocalMove(page,m[0],m[1],m[2],m[3],m[4],
                        afterSide,afterFullmove,operaExpectedStatus(ply));

    if (ply === 22) {
      const king = decodePiece(await call(page,'cf_review_piece',2,0));
      const rook = decodePiece(await call(page,'cf_review_piece',3,0));
      const e1 = decodePiece(await call(page,'cf_review_piece',4,0));
      const a1 = decodePiece(await call(page,'cf_review_piece',0,0));
      const castleLine = await newestHistoryLine(page);
      if (king.type !== 6 || king.color !== 1 ||
          rook.type !== 4 || rook.color !== 1 ||
          e1.type !== 0 || a1.type !== 0 ||
          !castleLine.startsWith('WHITE O-O-O'))
        throw new Error('Opera queenside castling state/history mismatch');
      await canvasShot(page,'local-full-23-after-queenside-castle');
    }

    if (ply === 23) {
      await page.keyboard.press('m');
      await sleep(180);
      await canvasShot(page,'local-full-24-history');
      await page.keyboard.press('Enter');
      await sleep(160);
    }
  }

  if (await call(page,'cf_review_status') !== 2 ||
      await call(page,'cf_review_side') !== 2 ||
      await call(page,'cf_review_fullmove') !== 17)
    throw new Error('Opera Game did not end at expected White mate on move 17');
  const finalLine = await newestHistoryLine(page);
  if (!finalLine.startsWith('WHITE D1-D8'))
    throw new Error(`unexpected Opera final history line: ${finalLine}`);
  await canvasShot(page,'local-full-33-checkmate');

  /* Terminal lock after the complete match. */
  const finalState = await persistedState(page);
  const finalHistoryCount = await call(page,'cf_review_history_count');
  await clickSquare(page,7,7);
  await page.keyboard.press('f');
  await page.keyboard.press('Enter');
  await sleep(180);
  if (await call(page,'cf_review_status') !== 2 ||
      await call(page,'cf_review_side') !== 2 ||
      await call(page,'cf_review_fullmove') !== 17 ||
      await call(page,'cf_review_history_count') !== finalHistoryCount ||
      !samePersistedState(finalState, await persistedState(page)))
    throw new Error('Opera terminal persisted state changed after gameplay input');

  if (errors.length) throw new Error(`local-full-game: ${errors.join(' | ')}`);
  await page.close();
  return 'LOCAL_FULL_GAME=PASS opera-game plies=33 rollback=exact-persisted-state replay=2 castling=O-O-O result=CHECKMATE winner=WHITE terminal-lock';
}

async function executeAction(page, a) {
  if (a.kind==='move') {
    await clickSquare(page,a.f,a.r);
    await clickSquare(page,a.m.toFile,a.m.toRank);
    if (a.m.flags & 8) { await page.keyboard.press('Enter'); await sleep(80); }
  } else {
    await clickSquare(page,a.f,a.r,'right');
    await sleep(80);
    await page.keyboard.press(dirKeys[a.d]);
    await sleep(60);
    await page.keyboard.press('Enter');
    if (a.preview===4) { await sleep(80); await page.keyboard.press('Enter'); }
  }
  await waitForCpu(page, 6000);
  await sleep(100);
}

async function playGame(browser, label, difficulty, seed) {
  const page = await browser.newPage();
  const errors = [];
  page.on('pageerror',e=>errors.push(`PAGE ${String(e)}`));
  page.on('console',m=>{ if(m.type()==='error') errors.push(`CONSOLE ${m.text()}`); });
  await page.setViewport({width:1100,height:850,deviceScaleFactor:1});
  await page.goto(`http://127.0.0.1:8127/?hardening=${label}`,{waitUntil:'domcontentloaded',timeout:15000});
  await page.waitForFunction(()=>document.getElementById('status')?.textContent.startsWith('Ready'),{timeout:15000});
  await page.waitForFunction(()=>typeof Module._cf_review_status==='function',{timeout:15000});
  await call(page,'cf_review_set_cpu_difficulty',difficulty);
  if (await call(page,'cf_review_cpu_difficulty') !== difficulty) throw new Error('difficulty probe mismatch');
  await page.keyboard.press('Enter');
  await sleep(500);
  await canvasShot(page,`${label}-00-opening`);

  const rnd = rngFor(seed);
  let turns = 0;
  let cpuFarts = 0;
  let cpuPushes = 0;
  let humanFarts = 0;
  const fartLines = [];
  let terminal = await call(page,'cf_review_status');

  while (terminal < 2 && turns < 120) {
    if (await call(page,'cf_review_side') !== 1) {
      await waitForCpu(page,6000);
      terminal = await call(page,'cf_review_status');
      continue;
    }
    const action = await chooseAction(page, turns, rnd);
    if (!action) break;
    if (action.kind === 'fart') humanFarts++;
    await executeAction(page, action);
    const newest = await newestHistoryLine(page);
    if (newest.startsWith('CPU FART')) {
      cpuFarts++;
      if (newest.includes('PUSH')) cpuPushes++;
      fartLines.push(`${turns+1}: ${newest}`);
    }
    turns++;
    terminal = await call(page,'cf_review_status');
    if (turns===10 || turns===25 || turns===50 || terminal>=2)
      await canvasShot(page,`${label}-${String(turns).padStart(3,'0')}`);
  }

  await canvasShot(page,`${label}-99-final`);
  const result = {
    label,
    difficulty:difficultyNames[difficulty],
    seed:`0x${(seed>>>0).toString(16)}`,
    result:statusNames[terminal] || String(terminal),
    whiteTurns:turns,
    fullmove:await call(page,'cf_review_fullmove'),
    cpuFarts,
    cpuPushes,
    humanFarts,
    errors:errors.length,
    fartLines
  };
  await page.close();
  if (errors.length) throw new Error(`${label}: ${errors.join(' | ')}`);
  if (terminal < 2) throw new Error(`${label}: game did not terminate after ${turns} white turns`);
  return result;
}

let browser;
try {
  await sleep(500);
  browser = await puppeteer.launch({executablePath:chrome,headless:true,args:['--no-sandbox','--disable-dev-shm-usage']});
  const matchSummary = await verifyMatchModes(browser);
  const localEdgeSummary = await verifyLocalEdgeHardening(browser);
  const replaySummary = await verifyReplayTimeline(browser);
  const replayViewerSummary = await verifyReplayViewer(browser);
  const localFullSummary = await verifyLocalFullGame(browser);
  const cases = [
    ['easy-a',0,0x00E451A1],
    ['medium-a',1,0x00C0FFEE],
    ['hard-a',2,0x0BADC0DE]
  ];
  const results = [];
  for (const [label,difficulty,seed] of cases) results.push(await playGame(browser,label,difficulty,seed));
  const summary = [...matchSummary, ...localEdgeSummary,
                   replaySummary, replayViewerSummary, localFullSummary];
  for (const r of results) {
    summary.push(`${r.label}: difficulty=${r.difficulty} result=${r.result} turns=${r.whiteTurns} cpuFarts=${r.cpuFarts} cpuPushes=${r.cpuPushes} humanFarts=${r.humanFarts} errors=${r.errors}`);
    for (const line of r.fartLines) summary.push(`  ${line}`);
  }
  const totalCpuFarts = results.reduce((n,r)=>n+r.cpuFarts,0);
  const totalPushes = results.reduce((n,r)=>n+r.cpuPushes,0);
  const totalHumanFarts = results.reduce((n,r)=>n+r.humanFarts,0);
  summary.push(`TOTAL_CPU_FARTS=${totalCpuFarts}`);
  summary.push(`TOTAL_CPU_PUSHES=${totalPushes}`);
  summary.push(`TOTAL_HUMAN_FARTS=${totalHumanFarts}`);
  fs.writeFileSync(path.join(outDir,'hardening.txt'),summary.join('\n')+'\n');
  console.log(summary.join('\n'));
  if (results.some(r => r.humanFarts < 1))
    throw new Error('each full-game hardening run must exercise at least one human Fart');
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
