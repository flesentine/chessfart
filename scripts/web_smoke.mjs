import { spawn } from 'node:child_process';
import fs from 'node:fs';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const server = spawn('python3', ['-m', 'http.server', '8123', '--directory', 'build/web'], {
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

let browser;
let page;
let errors = [];
try {
  await sleep(500);
  browser = await puppeteer.launch({
    executablePath: chrome,
    headless: true,
    args: ['--no-sandbox', '--disable-dev-shm-usage']
  });
  page = await browser.newPage();
  page.on('pageerror', (error) => errors.push(`PAGE: ${String(error)}`));
  page.on('console', (msg) => {
    if (msg.type() === 'error' || msg.type() === 'warning') {
      errors.push(`${msg.type().toUpperCase()}: ${msg.text()}`);
    }
  });

  await page.goto('http://127.0.0.1:8123/', {
    waitUntil: 'domcontentloaded',
    timeout: 15000
  });
  await page.waitForFunction(
    () => document.getElementById('status')?.textContent.startsWith('Ready'),
    { timeout: 15000 }
  );

  const titleSig = await canvasSignature(page);
  await page.keyboard.press('Enter');
  await sleep(800);
  const gameSig = await canvasSignature(page);
  if (titleSig === gameSig) throw new Error('canvas did not leave the title screen');
  if (errors.length) throw new Error(`browser errors: ${errors.join(' | ')}`);

  await page.screenshot({ path: 'build/web-smoke.png', fullPage: true });
  fs.writeFileSync('build/web-smoke.txt', `READY\ntitle=${titleSig}\ngame=${gameSig}\n`);
  console.log(`Web smoke passed: title ${titleSig} -> game ${gameSig}`);
} catch (error) {
  let status = '(unavailable)';
  let calledRun = '(unavailable)';
  let canvasSig = '(unavailable)';
  if (page) {
    try { status = await page.$eval('#status', (el) => el.textContent); } catch (_) {}
    try { calledRun = await page.evaluate(() => String(Boolean(Module && Module.calledRun))); } catch (_) {}
    try { canvasSig = String(await canvasSignature(page)); } catch (_) {}
    try { await page.screenshot({ path: 'build/web-smoke.png', fullPage: true }); } catch (_) {}
  }
  const report = [
    `FAIL: ${String(error)}`,
    `status=${status}`,
    `calledRun=${calledRun}`,
    `canvas=${canvasSig}`,
    ...errors
  ].join('\n') + '\n';
  fs.writeFileSync('build/web-smoke.txt', report);
  console.error(report);
  throw error;
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
