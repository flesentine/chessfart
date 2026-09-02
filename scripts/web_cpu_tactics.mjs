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
  if (ok !== 1) return { line: `${label}: runner failed (${ok})`, error: true };
  const state = {
    type: await call(page, 'cf_review_action_type'),
    fromFile: await call(page, 'cf_review_action_from_file'),
    fromRank: await call(page, 'cf_review_action_from_rank'),
    toFile: await call(page, 'cf_review_action_to_file'),
    toRank: await call(page, 'cf_review_action_to_rank'),
    dir: await call(page, 'cf_review_action_direction'),
    result: await call(page, 'cf_review_action_fart_result'),
    promotion: await call(page, 'cf_review_action_promotion'),
    score: await call(page, 'cf_review_action_score'),
    whiteCheck: await call(page, 'cf_review_check', 1),
    blackCheck: await call(page, 'cf_review_check', 2),
    castle: await call(page, 'cf_review_castling_rights')
  };
  let error = null;
  try { await verify(state); } catch (e) { error = String(e.message || e); }
  const base = `${label}: type=${state.type} from=${state.fromFile},${state.fromRank} to=${state.toFile},${state.toRank} dir=${state.dir} result=${state.result} promo=${state.promotion} score=${state.score} whiteCheck=${state.whiteCheck} blackCheck=${state.blackCheck} castle=${state.castle}`;
  return { line: error ? `${base} FAIL ${error}` : `${base} PASS`, error: !!error };
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

  const results = [];
  results.push(await runCase(page, 1, 'escape-check', async (s) => {
    if (s.type !== 2 || s.dir !== 2 || s.result !== 2 || s.whiteCheck !== 0)
      throw new Error(`expected Fart E PUSH that escapes check`);
  }));

  results.push(await runCase(page, 2, 'give-check', async (s) => {
    if (s.type !== 2 || s.dir !== 2 || s.result !== 2 || s.whiteCheck !== 1)
      throw new Error(`expected discovered-check Fart E PUSH`);
  }));

  results.push(await runCase(page, 3, 'wreck-castling', async (s) => {
    const king = decodePiece(await call(page, 'cf_review_piece_code', 5, 0));
    if (s.type !== 2 || s.dir !== 2 || s.result !== 2 || (s.castle & 3) !== 0 || king.type !== 6 || king.color !== 1)
      throw new Error(`expected king-displacing Fart E PUSH and lost white rights`);
  }));

  results.push(await runCase(page, 4, 'own-promotion', async (s) => {
    const promoted = decodePiece(await call(page, 'cf_review_piece_code', 5, 0));
    if (s.type !== 2 || s.dir !== 3 || s.result !== 4 || s.promotion !== 5 || promoted.type !== 5 || promoted.color !== 2)
      throw new Error(`expected own-pawn Fart SE promotion to queen`);
  }));

  const lines = results.map(r => r.line);
  if (errors.length) lines.push(...errors);
  const failures = results.filter(r => r.error).length + errors.length;
  lines.push(`TACTIC_FAILURES=${failures}`);
  fs.writeFileSync(`${outDir}/tactics.txt`, lines.join('\n') + '\n');
  console.log(lines.join('\n'));
  if (failures) throw new Error(`${failures} tactical CPU case(s) failed`);
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
