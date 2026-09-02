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
    if (status >= 2) return;
    if (await call(page,'cf_review_side') === 1) return;
    await sleep(80);
  }
  throw new Error('CPU turn timeout');
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
  const cases = [
    ['easy-a',0,0x00E451A1],
    ['medium-a',1,0x00C0FFEE],
    ['hard-a',2,0x0BADC0DE]
  ];
  const results = [];
  for (const [label,difficulty,seed] of cases) results.push(await playGame(browser,label,difficulty,seed));
  const summary = [];
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
