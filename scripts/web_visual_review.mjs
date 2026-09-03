import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-visual-review';
fs.mkdirSync(outDir, { recursive: true });

const server = spawn('python3', ['-m', 'http.server', '8128', '--directory', 'build/web'], {
  stdio: 'ignore'
});
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function call(page, name, ...args) {
  return await page.evaluate(({ name, args }) => {
    const fn = Module[`_${name}`];
    if (!fn) throw new Error(`missing review export ${name}`);
    return fn(...args);
  }, { name, args });
}

async function nativeShot(page, name, source, states) {
  const capture = await page.evaluate(() => {
    const canvas = document.getElementById('canvas');
    if (!canvas) throw new Error('missing canvas');
    const ctx = canvas.getContext('2d');
    const rgba = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
    let hash = 2166136261 >>> 0;
    for (let i = 0; i < rgba.length; i += 97) {
      hash ^= rgba[i];
      hash = Math.imul(hash, 16777619) >>> 0;
    }
    return {
      width: canvas.width,
      height: canvas.height,
      signature: hash >>> 0,
      dataUrl: canvas.toDataURL('image/png')
    };
  });

  if (capture.width !== 320 || capture.height !== 200) {
    throw new Error(`canonical canvas must be 320x200, got ${capture.width}x${capture.height}`);
  }
  const prefix = 'data:image/png;base64,';
  if (!capture.dataUrl.startsWith(prefix)) throw new Error('unexpected canvas PNG encoding');
  fs.writeFileSync(path.join(outDir, `${name}.png`),
                   Buffer.from(capture.dataUrl.slice(prefix.length), 'base64'));
  states.push({
    name,
    source,
    width: capture.width,
    height: capture.height,
    signature: capture.signature
  });
}

async function canvasPoint(page, file, rank) {
  const canvas = await page.$('#canvas');
  const box = await canvas.boundingBox();
  if (!box) throw new Error('canvas has no bounding box');
  const vx = 18 + file * 18 + 9;
  const vy = 27 + (7 - rank) * 18 + 9;
  return {
    x: box.x + (vx / 320) * box.width,
    y: box.y + (vy / 200) * box.height
  };
}

async function clickSquare(page, file, rank, button = 'left') {
  const point = await canvasPoint(page, file, rank);
  await page.mouse.click(point.x, point.y, { button, delay: 50 });
  await sleep(180);
}

async function press(page, key, pause = 150) {
  await page.keyboard.press(key);
  await sleep(pause);
}

let browser;
let page;
const states = [];
const errors = [];
try {
  await sleep(500);
  browser = await puppeteer.launch({
    executablePath: chrome,
    headless: true,
    args: ['--no-sandbox', '--disable-dev-shm-usage']
  });
  page = await browser.newPage();
  await page.setViewport({ width: 1100, height: 850, deviceScaleFactor: 1 });
  page.on('pageerror', (error) => errors.push(`PAGE: ${String(error)}`));
  page.on('console', (msg) => {
    if (msg.type() === 'error') errors.push(`CONSOLE: ${msg.text()}`);
  });

  await page.goto('http://127.0.0.1:8128/', { waitUntil: 'domcontentloaded', timeout: 15000 });
  await page.waitForFunction(
    () => document.getElementById('status')?.textContent.startsWith('Ready'),
    { timeout: 15000 }
  );

  await nativeShot(page, '01-title', 'real', states);
  await press(page, 'Enter', 700);
  await nativeShot(page, '02-normal-cursor', 'real', states);

  await press(page, 'h', 250);
  await nativeShot(page, '03-help', 'real', states);
  await press(page, 'Enter', 250);

  /* Selected piece + legal-move markers: the same real UI state. */
  await clickSquare(page, 4, 1);
  await nativeShot(page, '04-selected-legal-moves', 'real', states);
  await clickSquare(page, 4, 3);
  await sleep(1200);
  await nativeShot(page, '05-normal-after-e4', 'real', states);

  /* Charge H-pawn to 2 gas through normal gameplay. */
  await clickSquare(page, 7, 1);
  await clickSquare(page, 7, 2);
  await sleep(1200);
  await clickSquare(page, 7, 2);
  await clickSquare(page, 7, 3);
  await sleep(1200);
  await nativeShot(page, '06-gas-ready-h4', 'real', states);

  /* Enter and execute real Fart mode through normal input. */
  await clickSquare(page, 7, 3, 'right');
  await sleep(250);
  await nativeShot(page, '07-real-fart-mode', 'real', states);
  await press(page, 'ArrowUp');
  await press(page, 'Enter', 1200);
  await nativeShot(page, '08-after-fart', 'real', states);

  await press(page, 'm', 250);
  await nativeShot(page, '09-history', 'real', states);
  await press(page, 'Enter', 250);

  await press(page, 's', 250);
  await nativeShot(page, '10-save-confirmation', 'real', states);
  await press(page, 'l', 450);
  await nativeShot(page, '11-load-confirmation', 'real', states);

  /* Rare-state fixtures are CF_WEB_REVIEW-only and must validate themselves. */
  if (await call(page, 'cf_review_render_fixture', 0) !== 1)
    throw new Error('Fart push-preview fixture validation failed');
  await nativeShot(page, '12-fart-push-preview', 'fixture', states);

  if (await call(page, 'cf_review_render_fixture', 1) !== 1)
    throw new Error('CHECK fixture validation failed');
  await nativeShot(page, '13-check', 'fixture', states);

  if (await call(page, 'cf_review_render_fixture', 2) !== 1)
    throw new Error('CHECKMATE fixture validation failed');
  await nativeShot(page, '14-checkmate', 'fixture', states);

  if (errors.length) throw new Error(errors.join(' | '));

  const manifest = {
    schema: 1,
    purpose: 'Chess Fart Build 13 canonical visual review states',
    native_resolution: '320x200',
    commit: process.env.GITHUB_SHA || 'local',
    states
  };
  fs.writeFileSync(path.join(outDir, 'manifest.json'), `${JSON.stringify(manifest, null, 2)}\n`);
  fs.writeFileSync(
    path.join(outDir, 'visual-review.txt'),
    `PASS\nnative=320x200\nstates=${states.length}\n` +
      states.map((state) => `${state.name}=${state.signature} (${state.source})`).join('\n') + '\n'
  );
  console.log(`Build 13 visual review passed: ${states.length} native 320x200 states`);
  for (const state of states) console.log(`${state.name}=${state.signature} (${state.source})`);
} catch (error) {
  fs.writeFileSync(
    path.join(outDir, 'visual-review.txt'),
    `FAIL: ${String(error)}\n${errors.join('\n')}\n`
  );
  throw error;
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
