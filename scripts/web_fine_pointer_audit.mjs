import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-fine-pointer-audit';
fs.mkdirSync(outDir, { recursive: true });

const server = spawn('python3', ['-m', 'http.server', '8131', '--directory', 'build/web'], {
  stdio: 'ignore'
});
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

let browser;
let page;
const report = {
  schema: 1,
  purpose: 'Headful Chromium desktop fine-pointer CSS-path audit',
  commit: process.env.GITHUB_SHA || 'local',
  result: null,
  errors: []
};

try {
  await sleep(500);
  browser = await puppeteer.launch({
    executablePath: chrome,
    headless: false,
    args: ['--no-sandbox', '--disable-dev-shm-usage', '--window-size=1365,900']
  });
  page = await browser.newPage();
  await page.setViewport({ width: 1365, height: 900, deviceScaleFactor: 1 });
  page.on('pageerror', (error) => report.errors.push(`PAGE: ${String(error)}`));
  page.on('console', (msg) => {
    if (msg.type() === 'error') report.errors.push(`CONSOLE: ${msg.text()}`);
  });

  await page.goto('http://127.0.0.1:8131/?ux=desktop-fine-pointer-headful', {
    waitUntil: 'domcontentloaded',
    timeout: 15000
  });
  await page.waitForFunction(
    () => document.getElementById('status')?.textContent.startsWith('Ready'),
    { timeout: 15000 }
  );

  const fart = await page.$('.actions .fart');
  if (!fart) throw new Error('missing Fart action button');
  const fartBox = await fart.boundingBox();
  if (!fartBox) throw new Error('Fart action button has no bounding box');
  await page.mouse.move(fartBox.x + fartBox.width / 2, fartBox.y + fartBox.height / 2);
  await sleep(80);

  report.result = await page.evaluate(() => {
    const touch = document.querySelector('.touch');
    const actions = document.querySelector('.actions');
    const help = document.querySelector('.help');
    const canvas = document.getElementById('canvas');
    const fartButton = document.querySelector('.actions .fart');
    const canvasRect = canvas.getBoundingClientRect();
    return {
      viewport: { width: innerWidth, height: innerHeight },
      pointerFine: matchMedia('(pointer:fine)').matches,
      pointerCoarse: matchMedia('(pointer:coarse)').matches,
      hoverHover: matchMedia('(hover:hover)').matches,
      hoverNone: matchMedia('(hover:none)').matches,
      desktopFineRule: matchMedia(
        '(min-width:760px) and (hover:hover) and (pointer:fine)'
      ).matches,
      maxTouchPoints: navigator.maxTouchPoints,
      touchDisplay: getComputedStyle(touch).display,
      actionsDisplay: getComputedStyle(actions).display,
      helpDisplay: getComputedStyle(help).display,
      fartHovered: fartButton.matches(':hover'),
      canvas: {
        width: Math.round(canvasRect.width * 10) / 10,
        height: Math.round(canvasRect.height * 10) / 10
      }
    };
  });

  const r = report.result;
  if (!r.pointerFine || !r.hoverHover || !r.desktopFineRule)
    throw new Error(
      `headful desktop media mismatch: pointerFine=${r.pointerFine} hoverHover=${r.hoverHover} rule=${r.desktopFineRule}`
    );
  if (r.touchDisplay !== 'none')
    throw new Error(`desktop fine-pointer CSS did not hide touch controls: ${r.touchDisplay}`);
  if (r.actionsDisplay === 'none')
    throw new Error('desktop fine-pointer CSS hid action controls');
  if (r.helpDisplay === 'none')
    throw new Error('desktop fine-pointer CSS hid desktop keyboard help');
  if (!r.fartHovered)
    throw new Error('headful mouse did not activate :hover on the Fart button');
  if (Math.abs((r.canvas.width / r.canvas.height) - (320 / 200)) > 0.002)
    throw new Error(`desktop fine-pointer canvas aspect drifted: ${r.canvas.width}x${r.canvas.height}`);

  await page.screenshot({
    path: path.join(outDir, 'desktop-fine-pointer-1365x900.png'),
    fullPage: true
  });

  if (report.errors.length) throw new Error(report.errors.join(' | '));

  fs.writeFileSync(
    path.join(outDir, 'audit.json'),
    `${JSON.stringify(report, null, 2)}\n`
  );
  fs.writeFileSync(
    path.join(outDir, 'audit.txt'),
    [
      'PASS',
      `pointerFine=${r.pointerFine}`,
      `hoverHover=${r.hoverHover}`,
      `rule=${r.desktopFineRule}`,
      `touch=${r.touchDisplay}`,
      `actions=${r.actionsDisplay}`,
      `help=${r.helpDisplay}`,
      `fartHover=${r.fartHovered}`,
      `maxTouchPoints=${r.maxTouchPoints}`,
      `canvas=${r.canvas.width}x${r.canvas.height}`
    ].join('\n') + '\n'
  );
  console.log(fs.readFileSync(path.join(outDir, 'audit.txt'), 'utf8'));
} catch (error) {
  report.errors.push(String(error));
  fs.writeFileSync(path.join(outDir, 'audit.json'), `${JSON.stringify(report, null, 2)}\n`);
  fs.writeFileSync(path.join(outDir, 'audit.txt'), `FAIL: ${String(error)}\n`);
  throw error;
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
