import { spawn } from 'node:child_process';
import fs from 'node:fs';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-cpu-tactics';
fs.mkdirSync(outDir, { recursive: true });
const server = spawn('python3', ['-m', 'http.server', '8128', '--directory', 'build/web'], { stdio: 'ignore' });
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function call(page, name, ...args) {
  return await page.evaluate(({name,args}) => {
    const fn = Module[`_${name}`];
    if (!fn) throw new Error(`missing review export ${name}`);
    return fn(...args);
  }, {name,args});
}

function decodePiece(code) {
  return { type: code & 15, color: (code >> 4) & 15, gas: (code >> 8) & 15 };
}

async function runCase(page, id, label, verify) {
  const ok = await call(page, 'cf_review_run_tactic', id);
  if (ok !== 1) throw new Error(`${label}: tactic runner failed (${ok})`);
  const state = {
    type: await call(page, 'cf_review_action_type'),
    dir: await call(page, 'cf_review_action_direction'),
    result: await call(page, 'cf_review_action_fart_result'),
    promotion: await call(page, 'cf_review_action_promotion'),
    whiteCheck: await call(page, 'cf_review_check', 1),
    blackCheck: await call(page, 'cf_review_check', 2),
    castle: await call(page, 'cf_review_castling_rights')
  };
  await verify(state);
  return `${label}: type=${state.type} dir=${state.dir} result=${state.result} promo=${state.promotion} whiteCheck=${state.whiteCheck} blackCheck=${state.blackCheck} castle=${state.castle}`;
}

let browser;
try {
  await sleep(500);
  browser = await puppeteer.launch({ executablePath: chrome, headless: true, args: ['--no-sandbox','--disable-dev-shm-usage'] });
  const page = await browser.newPage();
  const errors = [];
  page.on('pageerror', e => errors.push(`PAGE ${String(e)}`));
  page.on('console', m => { if (m.type() === 'error') errors.push(`CONSOLE ${m.text()}`); });
  await page.goto('http://127.0.0.1:8128/?cpu-tactics=1', { waitUntil: 'domcontentloaded', timeout: 15000 });
  await page.waitForFunction(() => document.getElementById('status')?.textContent.startsWith('Ready'), { timeout: 15000 });
  await page.waitForFunction(() => typeof Module._cf_review_run_tactic === 'function', { timeout: 15000 });

  const lines = [];
  lines.push(await runCase(page, 1, 'escape-check', async (s) => {
    if (s.type !== 2 || s.dir !== 2 || s.result !== 2 || s.whiteCheck !== 0)
      throw new Error(`escape-check chose wrong action ${JSON.stringify(s)}`);
  }));

  lines.push(await runCase(page, 2, 'give-check', async (s) => {
    if (s.type !== 2 || s.dir !== 2 || s.result !== 2 || s.whiteCheck !== 1)
      throw new Error(`give-check chose wrong action ${JSON.stringify(s)}`);
  }));

  lines.push(await runCase(page, 3, 'wreck-castling', async (s) => {
    const king = decodePiece(await call(page, 'cf_review_piece_code', 5, 0));
    if (s.type !== 2 || s.dir !== 2 || s.result !== 2 || (s.castle & 3) !== 0 || king.type !== 6 || king.color !== 1)
      throw new Error(`wreck-castling chose wrong action ${JSON.stringify({...s,king})}`);
  }));

  lines.push(await runCase(page, 4, 'own-promotion', async (s) => {
    const promoted = decodePiece(await call(page, 'cf_review_piece_code', 5, 0));
    if (s.type !== 2 || s.dir !== 3 || s.result !== 4 || s.promotion !== 5 || promoted.type !== 5 || promoted.color !== 2)
      throw new Error(`own-promotion chose wrong action ${JSON.stringify({...s,promoted})}`);
  }));

  if (errors.length) throw new Error(errors.join(' | '));
  lines.push('PASS: all four CPU Fart tactics selected correctly in Chromium/WASM');
  fs.writeFileSync(`${outDir}/tactics.txt`, lines.join('\n') + '\n');
  console.log(lines.join('\n'));
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
