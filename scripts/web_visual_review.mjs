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
let reviewGeometry = null;

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
  return capture.signature;
}

async function canonicalGeometry(page) {
  if (reviewGeometry === null) {
    reviewGeometry = {
      boardX: await call(page, 'cf_review_board_x'),
      boardY: await call(page, 'cf_review_board_y'),
      square: await call(page, 'cf_review_square_size')
    };
    if (reviewGeometry.square <= 0) throw new Error('invalid canonical square size');
  }
  return reviewGeometry;
}

async function canvasPoint(page, file, rank) {
  const canvas = await page.$('#canvas');
  const box = await canvas.boundingBox();
  const geometry = await canonicalGeometry(page);
  if (!box) throw new Error('canvas has no bounding box');
  const vx = geometry.boardX + file * geometry.square + Math.floor(geometry.square / 2);
  const vy = geometry.boardY + (7 - rank) * geometry.square + Math.floor(geometry.square / 2);
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

async function loadLocalFixture(page, fixture, side, fullmove=1) {
  if (await call(page, 'cf_review_write_local_fixture', fixture) !== 1)
    throw new Error(`failed to write local fixture ${fixture}`);
  await press(page, 'l', 220);
  const deadline = Date.now() + 6000;
  while (Date.now() < deadline) {
    if (await call(page, 'cf_review_match_mode') === 1 &&
        await call(page, 'cf_review_side') === side &&
        await call(page, 'cf_review_fullmove') === fullmove &&
        await call(page, 'cf_review_status') === 0 &&
        await call(page, 'cf_review_ui_synced')) return;
    await sleep(80);
  }
  throw new Error(`fixture load timeout ${fixture}`);
}

async function waitForStatus(page, status, side, fullmove) {
  const deadline = Date.now() + 6000;
  while (Date.now() < deadline) {
    if (await call(page, 'cf_review_status') === status &&
        await call(page, 'cf_review_side') === side &&
        await call(page, 'cf_review_fullmove') === fullmove &&
        await call(page, 'cf_review_ui_synced')) return;
    await sleep(80);
  }
  throw new Error(`status timeout ${status}/${side}/${fullmove}`);
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

  if (await call(page, 'cf_review_render_fixture', 3) !== 1)
    throw new Error('Fart edge-invalid fixture validation failed');
  await nativeShot(page, '15-fart-edge-invalid', 'fixture', states);

  if (await call(page, 'cf_review_render_fixture', 4) !== 1)
    throw new Error('STALEMATE fixture validation failed');
  await nativeShot(page, '16-stalemate', 'fixture', states);

  if (await call(page, 'cf_review_render_ui_fixture', 0) !== 1)
    throw new Error('Help page 2 fixture validation failed');
  await nativeShot(page, '17-help-page-2', 'fixture', states);

  if (await call(page, 'cf_review_render_ui_fixture', 1) !== 1)
    throw new Error('Credits fixture validation failed');
  await nativeShot(page, '18-credits', 'fixture', states);

  if (await call(page, 'cf_review_render_ui_fixture', 2) !== 1)
    throw new Error('Credits title-selection fixture validation failed');
  await nativeShot(page, '19-title-credits-selected', 'fixture', states);

  if (await call(page, 'cf_review_render_ui_fixture', 3) !== 1)
    throw new Error('2 PLAYERS title-selection fixture validation failed');
  await nativeShot(page, '20-title-2-players-selected', 'fixture', states);

  /* Build 14.2 real local-play presentation states. */
  await page.goto('http://127.0.0.1:8128/?visual=local-presentation',
                  { waitUntil: 'domcontentloaded', timeout: 15000 });
  await page.waitForFunction(
    () => document.getElementById('status')?.textContent.startsWith('Ready'),
    { timeout: 15000 }
  );
  await press(page, 'ArrowDown');
  await press(page, 'Enter', 400);
  if (await call(page, 'cf_review_match_mode') !== 1)
    throw new Error('visual local title selection did not enter local mode');

  await clickSquare(page, 4, 1);
  await clickSquare(page, 4, 3);
  await sleep(250);
  if (await call(page, 'cf_review_side') !== 2)
    throw new Error('visual local White move did not leave Black to move');
  await nativeShot(page, '21-local-black-to-move-hud', 'real', states);

  await clickSquare(page, 4, 6);
  await clickSquare(page, 4, 4);
  await sleep(250);
  if (await call(page, 'cf_review_history_has_local_pair') !== 1)
    throw new Error('visual local history did not record WHITE then BLACK');
  await press(page, 'm', 220);
  await nativeShot(page, '22-local-history-white-black', 'real', states);
  await press(page, 'Enter', 180);

  /* Charge a local White pawn to 2 gas, alternate real Black replies, then
   * prove the local Fart HUD also presents MODE / LOCAL 2P. */
  await clickSquare(page, 7, 1);
  await clickSquare(page, 7, 2);
  await clickSquare(page, 7, 6);
  await clickSquare(page, 7, 5);
  await clickSquare(page, 7, 2);
  await clickSquare(page, 7, 3);
  await clickSquare(page, 7, 5);
  await clickSquare(page, 7, 4);
  await clickSquare(page, 7, 3, 'right');
  await sleep(220);
  if (await call(page, 'cf_review_fart_mode') !== 1)
    throw new Error('visual local Fart mode did not activate');
  const localFartSig =
    await nativeShot(page, '23-local-fart-mode-hud', 'real', states);

  await press(page, 'Escape', 180);
  if (await call(page, 'cf_review_fart_mode') !== 0)
    throw new Error('Escape did not cancel local Fart mode');
  await press(page, 'h', 220);
  await press(page, 'ArrowRight', 180);
  const localHelpSig =
    await nativeShot(page, '24-local-help-page-2', 'real', states);
  if (localHelpSig === localFartSig)
    throw new Error('local Help capture did not leave Fart HUD');
  const cpuHelp = states.find((state) => state.name === '17-help-page-2');
  if (!cpuHelp || localHelpSig === cpuHelp.signature)
    throw new Error('local Help page 2 did not differ from CPU Help');

  await press(page, 'Enter', 180);

  /* Build 14.4 real local edge states. */
  await loadLocalFixture(page, 0, 2, 1);
  await clickSquare(page, 3, 4, 'right');
  await press(page, 'ArrowDown', 90);
  await press(page, 'Enter', 220);
  if (await call(page, 'cf_review_side') !== 1)
    throw new Error('visual Black Fart did not pass turn to White');
  await nativeShot(page, '25-local-black-fart-after', 'real', states);

  await loadLocalFixture(page, 1, 1, 1);
  await clickSquare(page, 1, 6);
  await clickSquare(page, 1, 7);
  if (await call(page, 'cf_review_promotion_pending') !== 1)
    throw new Error('visual White promotion did not enter choice state');
  await nativeShot(page, '26-local-white-promotion-choice', 'real', states);
  await press(page, 'ArrowRight', 80);
  await press(page, 'Enter', 220);
  if (await call(page, 'cf_review_side') !== 2)
    throw new Error('visual White promotion did not complete');
  await nativeShot(page, '27-local-white-promotion-rook', 'real', states);

  await loadLocalFixture(page, 2, 2, 1);
  await clickSquare(page, 1, 1);
  await clickSquare(page, 1, 0);
  await press(page, 'ArrowRight', 80);
  await press(page, 'Enter', 220);
  if (await call(page, 'cf_review_side') !== 1)
    throw new Error('visual Black promotion did not complete');
  await nativeShot(page, '28-local-black-promotion-rook', 'real', states);

  await loadLocalFixture(page, 3, 1, 1);
  await clickSquare(page, 3, 5, 'right');
  await press(page, 'ArrowUp', 80);
  await press(page, 'Enter', 120);
  if (await call(page, 'cf_review_fart_promotion_pending') !== 1)
    throw new Error('visual White Fart promotion did not enter choice state');
  await nativeShot(page, '29-local-white-fart-promotion-choice', 'real', states);

  await loadLocalFixture(page, 4, 2, 1);
  await clickSquare(page, 3, 2, 'right');
  await press(page, 'ArrowDown', 80);
  await press(page, 'Enter', 120);
  if (await call(page, 'cf_review_fart_promotion_pending') !== 1)
    throw new Error('visual Black Fart promotion did not enter choice state');
  await press(page, 'ArrowRight', 80);
  await press(page, 'Enter', 220);
  if (await call(page, 'cf_review_side') !== 1)
    throw new Error('visual Black Fart promotion did not complete');
  await nativeShot(page, '30-local-black-fart-promotion-rook', 'real', states);

  await loadLocalFixture(page, 5, 1, 1);
  await clickSquare(page, 6, 5);
  await clickSquare(page, 6, 6);
  await waitForStatus(page, 2, 2, 1);
  await nativeShot(page, '31-local-white-checkmate', 'real', states);

  await loadLocalFixture(page, 6, 2, 1);
  await clickSquare(page, 6, 2);
  await clickSquare(page, 6, 1);
  await waitForStatus(page, 2, 1, 2);
  await nativeShot(page, '32-local-black-checkmate', 'real', states);

  if (errors.length) throw new Error(errors.join(' | '));

  const manifest = {
    schema: 1,
    purpose: 'Chess Fart Build 14 canonical visual review states',
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
  console.log(`Build 14 visual review passed: ${states.length} native 320x200 states`);
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
