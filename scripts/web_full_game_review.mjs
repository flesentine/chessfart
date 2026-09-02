import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-full-game';
fs.mkdirSync(outDir, { recursive: true });
const server = spawn('python3', ['-m', 'http.server', '8125', '--directory', 'build/web'], { stdio: 'ignore' });
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
const pieceValue = [0, 100, 320, 330, 500, 900, 20000];
const dirKeys = ['ArrowUp', 'PageUp', 'ArrowRight', 'PageDown', 'ArrowDown', 'End', 'ArrowLeft', 'Home'];
const statusNames = ['ONGOING','CHECK','CHECKMATE','STALEMATE','DRAW_50','DRAW_3FOLD','DRAW_MATERIAL'];
let seed = 0xC0FFEE;
function rnd() { seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0; return seed / 4294967296; }

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
async function canvasShot(page, name) {
  const canvas = await page.$('#canvas');
  await canvas.screenshot({ path: path.join(outDir, `${name}.png`) });
}
async function canvasPoint(page, file, rank) {
  const box = await (await page.$('#canvas')).boundingBox();
  const vx = 18 + file * 18 + 9;
  const vy = 27 + (7 - rank) * 18 + 9;
  return { x: box.x + vx / 320 * box.width, y: box.y + vy / 200 * box.height };
}
async function clickSquare(page, file, rank, button='left') {
  const p = await canvasPoint(page, file, rank);
  await page.mouse.click(p.x, p.y, {button, delay: 35});
  await sleep(100);
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

async function chooseAction(page, ply, fartStats) {
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
      const center = (3.5-Math.abs(m.toFile-3.5)) + (3.5-Math.abs(m.toRank-3.5));
      score += center * 14;
      if ((p.type===2 || p.type===3) && r===0 && m.toRank>0) score += 180;
      if (p.type===1 && (m.toRank===3 || m.toRank===4)) score += 60;
      if (p.type===5 && ply<12) score -= 130;
      if (p.type===6 && !(m.flags&6) && ply<18) score -= 220;
      if (p.gas===1) score += 85; /* charging a piece to fart-ready is useful */
      if (p.gas>=2) score -= 20; /* spend charged pieces rather than cycling them */
      score += rnd()*90;
      actions.push({kind:'move',f,r,p,m,score,label:`${sq(f,r)}-${sq(m.toFile,m.toRank)}`});
    }
    if (p.gas >= 2) {
      for (let d=0;d<8;d++) {
        const preview = await call(page,'cf_review_fart_preview',f,r,d);
        if (preview===0) continue;
        const df=[0,1,1,1,0,-1,-1,-1][d], dr=[1,1,0,-1,-1,-1,0,1][d];
        const tf=f+df,tr=r+dr;
        const target=(tf>=0&&tf<8&&tr>=0&&tr<8)?board[tr][tf]:{type:0,color:0,gas:0};
        let score = 0;
        if (preview===4) score = 9000;
        else if (preview===2) score = 720 + (target.color===2 ? pieceValue[target.type]*1.4 : -pieceValue[target.type]*0.35);
        else if (preview===3) score = -150;
        else if (preview===1) score = fartStats.puffs<2 ? 145 : -40;
        if (target.color===2 && (tf===0||tf===7||tr===0||tr===7)) score += 120;
        score += rnd()*80;
        actions.push({kind:'fart',f,r,d,preview,target,score,label:`FART ${sq(f,r)} ${['N','NE','E','SE','S','SW','W','NW'][d]} p${preview}`});
      }
    }
  }
  actions.sort((a,b)=>b.score-a.score);
  if (!actions.length) return null;
  /* Avoid robotic determinism: choose among near-best moves, but never throw away a big tactic. */
  const best = actions[0].score;
  const pool = actions.filter(a => a.score >= best - (best>1000 ? 100 : 180)).slice(0,5);
  return pool[Math.floor(rnd()*pool.length)];
}

async function executeAction(page, a) {
  if (a.kind==='move') {
    await clickSquare(page,a.f,a.r);
    await clickSquare(page,a.m.toFile,a.m.toRank);
    if (a.m.flags & 8) { await page.keyboard.press('Enter'); await sleep(120); }
  } else {
    await clickSquare(page,a.f,a.r,'right');
    await sleep(100);
    await page.keyboard.press(dirKeys[a.d]);
    await sleep(80);
    await page.keyboard.press('Enter');
    if (a.preview===4) { await sleep(100); await page.keyboard.press('Enter'); }
  }
  await sleep(450);
}

let browser;
let page;
const report=[];
const fullLog=[];
const errors=[];
const fartStats={total:0,pushes:0,puffs:0,blocked:0,promotions:0};
try {
  await sleep(500);
  browser=await puppeteer.launch({executablePath:chrome,headless:true,args:['--no-sandbox','--disable-dev-shm-usage']});
  page=await browser.newPage();
  await page.setViewport({width:1100,height:850,deviceScaleFactor:1});
  page.on('pageerror',e=>errors.push(`PAGE ${String(e)}`));
  page.on('console',m=>{if(m.type()==='error')errors.push(`CONSOLE ${m.text()}`);});
  await page.goto('http://127.0.0.1:8125/',{waitUntil:'domcontentloaded',timeout:15000});
  await page.waitForFunction(()=>document.getElementById('status')?.textContent.startsWith('Ready'),{timeout:15000});
  await page.waitForFunction(()=>typeof Module._cf_review_status==='function',{timeout:15000});
  await page.keyboard.press('Enter');
  await sleep(700);
  await canvasShot(page,'00-opening');

  let turn=0;
  let terminal=await call(page,'cf_review_status');
  while (terminal < 2 && turn < 220) {
    if (await call(page,'cf_review_side') !== 1) { await sleep(100); terminal=await call(page,'cf_review_status'); continue; }
    const a=await chooseAction(page,turn,fartStats);
    if (!a) break;
    const beforeCount=await call(page,'cf_review_history_count');
    await executeAction(page,a);
    terminal=await call(page,'cf_review_status');
    const afterCount=await call(page,'cf_review_history_count');
    const lines=[];
    for(let i=Math.max(0,afterCount-2);i<afterCount;i++) lines.push(await historyLine(page,i));
    fullLog.push(`${String(turn+1).padStart(3,'0')} ${a.label} | ${lines.join(' | ')}`);
    if (a.kind==='fart') {
      fartStats.total++;
      if(a.preview===1)fartStats.puffs++;
      else if(a.preview===2)fartStats.pushes++;
      else if(a.preview===3)fartStats.blocked++;
      else if(a.preview===4)fartStats.promotions++;
    }
    if (afterCount===beforeCount) fullLog.push(`WARN no history change after ${a.label}`);
    turn++;
    if (turn===5 || turn===10 || turn===20 || turn===35 || turn===50 || turn%30===0 || terminal>=2)
      await canvasShot(page,`${String(turn).padStart(3,'0')}-position`);
  }

  terminal=await call(page,'cf_review_status');
  const finalBoard=await boardState(page);
  const material={white:0,black:0,whitePieces:0,blackPieces:0};
  for(let r=0;r<8;r++)for(let f=0;f<8;f++){
    const p=finalBoard[r][f];
    if(p.color===1){material.white+=pieceValue[p.type];material.whitePieces++;}
    if(p.color===2){material.black+=pieceValue[p.type];material.blackPieces++;}
  }
  await canvasShot(page,'98-final');
  await page.keyboard.press('m'); await sleep(250); await canvasShot(page,'99-final-log');
  report.push(`RESULT=${statusNames[terminal]||terminal}`);
  report.push(`WHITE_TURNS=${turn}`);
  report.push(`FULLMOVE=${await call(page,'cf_review_fullmove')}`);
  report.push(`FARTS=${JSON.stringify(fartStats)}`);
  report.push(`MATERIAL=${JSON.stringify(material)}`);
  report.push(`ERRORS=${errors.length}`);
  fs.writeFileSync(path.join(outDir,'full-game.txt'),`${report.join('\n')}\n\n${fullLog.join('\n')}\n${errors.join('\n')}\n`);
  if(errors.length)throw new Error(errors.join(' | '));
  if(terminal<2)throw new Error(`game did not terminate after ${turn} white turns`);
  console.log(report.join('\n'));
} catch(e) {
  try{if(page)await canvasShot(page,'97-failure');}catch(_){ }
  fs.writeFileSync(path.join(outDir,'failure.txt'),String(e)+'\n'+report.join('\n')+'\n'+fullLog.join('\n'));
  throw e;
} finally {
  if(browser)await browser.close();
  server.kill('SIGTERM');
}
