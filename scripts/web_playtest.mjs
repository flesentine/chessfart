import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-playtest';
fs.mkdirSync(outDir, { recursive: true });

const server = spawn('python3', ['-m', 'http.server', '8124', '--directory', 'build/web'], {
  stdio: 'ignore'
});
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function canvasSignature(page) {
  return await page.evaluate(() => {
    const c = document.getElementById('canvas');
    const d = c.getContext('2d').getImageData(0, 0, c.width, c.height).data;
    let hash = 2166136261 >>> 0;
    for (let i = 0; i < d.length; i += 97) {
      hash ^= d[i];
      hash = Math.imul(hash, 16777619) >>> 0;
    }
    return hash >>> 0;
  });
}

async function shot(page, name) {
  const canvas = await page.$('#canvas');
  await canvas.screenshot({ path: path.join(outDir, `${name}.png`) });
  return await canvasSignature(page);
}

async function canvasPoint(page, vx, vy) {
  const box = await (await page.$('#canvas')).boundingBox();
  if (!box) throw new Error('canvas has no bounding box');
  return {
    x: box.x + (vx / 320) * box.width,
    y: box.y + (vy / 200) * box.height
  };
}

async function clickSquare(page, file, rank, button = 'left') {
  const vx = 18 + file * 18 + 9;
  const vy = 27 + (7 - rank) * 18 + 9;
  const p = await canvasPoint(page, vx, vy);
  await page.mouse.click(p.x, p.y, { button, delay: 60 });
  await sleep(180);
}

async function press(page, key, pause = 120) {
  await page.keyboard.press(key);
  await sleep(pause);
}

let browser;
let page;
const report = [];
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

  await page.goto('http://127.0.0.1:8124/', { waitUntil: 'domcontentloaded', timeout: 15000 });
  await page.waitForFunction(
    () => document.getElementById('status')?.textContent.startsWith('Ready'),
    { timeout: 15000 }
  );

  report.push(`title=${await shot(page, '01-title')}`);
  await press(page, 'Enter', 700);
  report.push(`start=${await shot(page, '02-start')}`);

  /* Mouse play: 1. e2-e4 */
  await clickSquare(page, 4, 1);
  await clickSquare(page, 4, 3);
  await sleep(1200);
  report.push(`e4=${await shot(page, '03-after-e4')}`);

  /* 2. Ng1-f3 */
  await clickSquare(page, 6, 0);
  await clickSquare(page, 5, 2);
  await sleep(1200);
  report.push(`nf3=${await shot(page, '04-after-nf3')}`);

  /* 3. Nf3-g5. The knight should now have two Gas. */
  await clickSquare(page, 5, 2);
  await clickSquare(page, 6, 4);
  await sleep(1200);
  report.push(`ng5=${await shot(page, '05-knight-g5-gas2')}`);

  /* Right click the charged knight, aim north, then fire. */
  await clickSquare(page, 6, 4, 'right');
  await sleep(250);
  report.push(`fartmode=${await shot(page, '06-fart-mode')}`);
  await press(page, 'ArrowUp');
  await press(page, 'Enter', 1200);
  report.push(`fart=${await shot(page, '07-after-fart')}`);

  /* Review the action log as a player would. */
  await press(page, 'm', 250);
  report.push(`history=${await shot(page, '08-history')}`);
  await press(page, 'Enter', 250);

  /* Verify save/load controls during an active game. */
  await press(page, 's', 250);
  await press(page, 'l', 450);
  report.push(`reload=${await shot(page, '09-after-load')}`);

  if (errors.length) throw new Error(errors.join(' | '));
  fs.writeFileSync(path.join(outDir, 'playtest.txt'), `PASS\n${report.join('\n')}\n`);
  console.log(`Chromium gameplay playtest passed\n${report.join('\n')}`);
} catch (error) {
  if (page) {
    try { await shot(page, '99-failure'); } catch (_) {}
  }
  fs.writeFileSync(path.join(outDir, 'playtest.txt'), `FAIL: ${String(error)}\n${report.join('\n')}\n${errors.join('\n')}\n`);
  throw error;
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
