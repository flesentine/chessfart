import { spawn } from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const chrome = process.env.CHROME_BIN || '/usr/bin/google-chrome';
const outDir = 'build/web-ux-audit';
fs.mkdirSync(outDir, { recursive: true });

const server = spawn('python3', ['-m', 'http.server', '8130', '--directory', 'build/web'], {
  stdio: 'ignore'
});
const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

const viewports = [
  { name: 'desktop-1365x900', width: 1365, height: 900 },
  { name: 'tablet-768x1024', width: 768, height: 1024 },
  { name: 'mobile-390x844', width: 390, height: 844 },
  { name: 'mobile-320x568', width: 320, height: 568 }
];

async function waitReady(page) {
  await page.waitForFunction(
    () => document.getElementById('status')?.textContent.startsWith('Ready'),
    { timeout: 15000 }
  );
}

async function layoutMetrics(page) {
  return await page.evaluate(() => {
    const rect = (el) => {
      const r = el.getBoundingClientRect();
      return {
        x: Math.round(r.x * 10) / 10,
        y: Math.round(r.y * 10) / 10,
        width: Math.round(r.width * 10) / 10,
        height: Math.round(r.height * 10) / 10,
        right: Math.round(r.right * 10) / 10,
        bottom: Math.round(r.bottom * 10) / 10
      };
    };
    const machine = document.querySelector('.machine');
    const canvas = document.getElementById('canvas');
    const canvasStyle = getComputedStyle(canvas);
    const touch = document.querySelector('.touch');
    const buttons = Array.from(document.querySelectorAll('button')).map((b) => ({
      text: b.textContent.trim(),
      key: Number(b.dataset.k || 0),
      ariaLabel: b.getAttribute('aria-label'),
      rect: rect(b)
    }));
    const viewport = document.querySelector('meta[name="viewport"]')?.content || '';
    return {
      viewport: { width: innerWidth, height: innerHeight },
      document: {
        scrollWidth: document.documentElement.scrollWidth,
        scrollHeight: document.documentElement.scrollHeight,
        clientWidth: document.documentElement.clientWidth,
        clientHeight: document.documentElement.clientHeight
      },
      bodyScrollWidth: document.body.scrollWidth,
      machine: rect(machine),
      canvas: rect(canvas),
      canvasBox: {
        clientWidth: canvas.clientWidth,
        clientHeight: canvas.clientHeight,
        borderTop: parseFloat(canvasStyle.borderTopWidth) || 0,
        borderRight: parseFloat(canvasStyle.borderRightWidth) || 0,
        borderBottom: parseFloat(canvasStyle.borderBottomWidth) || 0,
        borderLeft: parseFloat(canvasStyle.borderLeftWidth) || 0
      },
      touchDisplay: getComputedStyle(touch).display,
      canvasAccessibleName: canvas.getAttribute('aria-label'),
      viewportMeta: viewport,
      buttons
    };
  });
}

async function inputProbe(page) {
  return await page.evaluate(async () => {
    const original = Module.cfPushKey;
    const seen = [];
    Module.cfPushKey = function (code) { seen.push(code | 0); };

    async function tick() {
      await new Promise((resolve) => setTimeout(resolve, 40));
    }

    const result = { pointer: [], keyboard: [], tab: null };
    for (const b of document.querySelectorAll('.actions button')) {
      seen.length = 0;
      b.dispatchEvent(new PointerEvent('pointerdown', {
        bubbles: true, cancelable: true, pointerId: 1, pointerType: 'mouse',
        button: 0, buttons: 1
      }));
      await tick();
      result.pointer.push({
        label: b.textContent.trim(),
        expected: Number(b.dataset.k),
        seen: [...seen]
      });
    }

    for (const b of document.querySelectorAll('.actions button')) {
      seen.length = 0;
      b.focus();
      b.dispatchEvent(new KeyboardEvent('keydown', {
        key: ' ', code: 'Space', bubbles: true, cancelable: true
      }));
      b.dispatchEvent(new KeyboardEvent('keyup', {
        key: ' ', code: 'Space', bubbles: true, cancelable: true
      }));
      await tick();
      result.keyboard.push({
        label: b.textContent.trim(),
        expected: Number(b.dataset.k),
        seen: [...seen],
        active: document.activeElement === b
      });
    }

    seen.length = 0;
    Module.canvas.focus();
    Module.canvas.dispatchEvent(new KeyboardEvent('keydown', {
      key: 'Tab', code: 'Tab', bubbles: true, cancelable: true
    }));
    await tick();
    result.tab = {
      seen: [...seen],
      activeTag: document.activeElement?.tagName || '',
      activeId: document.activeElement?.id || ''
    };

    Module.cfPushKey = original;
    return result;
  });
}

let browser;
let page;
const report = {
  schema: 1,
  purpose: 'Deep Chromium UX audit for Chess Fart web wrapper and input surface',
  commit: process.env.GITHUB_SHA || 'local',
  layouts: [],
  input: null,
  findings: [],
  notes: [],
  errors: []
};

try {
  await sleep(500);
  browser = await puppeteer.launch({
    executablePath: chrome,
    headless: true,
    args: ['--no-sandbox', '--disable-dev-shm-usage']
  });
  page = await browser.newPage();
  page.on('pageerror', (error) => report.errors.push(`PAGE: ${String(error)}`));
  page.on('console', (msg) => {
    if (msg.type() === 'error') report.errors.push(`CONSOLE: ${msg.text()}`);
  });

  for (const vp of viewports) {
    await page.setViewport({ width: vp.width, height: vp.height, deviceScaleFactor: 1 });
    await page.goto(`http://127.0.0.1:8130/?ux=${vp.name}`, {
      waitUntil: 'domcontentloaded',
      timeout: 15000
    });
    await waitReady(page);
    const metrics = await layoutMetrics(page);
    report.layouts.push({ name: vp.name, ...metrics });
    await page.screenshot({
      path: path.join(outDir, `${vp.name}.png`),
      fullPage: true
    });
  }

  await page.setViewport({ width: 1100, height: 850, deviceScaleFactor: 1 });
  await page.goto('http://127.0.0.1:8130/?ux=input', {
    waitUntil: 'domcontentloaded',
    timeout: 15000
  });
  await waitReady(page);
  report.input = await inputProbe(page);

  const mobileLayouts = report.layouts.filter((x) => x.name.startsWith('mobile-'));
  for (const layout of mobileLayouts) {
    if (layout.document.scrollWidth > layout.viewport.width) {
      report.findings.push({
        severity: 'high',
        id: 'mobile-horizontal-overflow',
        state: layout.name,
        detail: `document width ${layout.document.scrollWidth}px exceeds ${layout.viewport.width}px viewport`
      });
    }
  }

  const borderedCanvasLayouts = report.layouts.filter((layout) => {
    const box = layout.canvasBox;
    return Math.max(box.borderTop, box.borderRight, box.borderBottom, box.borderLeft) > 0;
  });
  if (borderedCanvasLayouts.length) {
    report.findings.push({
      severity: 'medium',
      id: 'canvas-border-hitbox',
      detail: 'CSS borders must stay outside the VGA canvas so the 8:5 bitmap and mouse coordinate mapping use the same box',
      states: borderedCanvasLayouts.map((layout) => layout.name)
    });
  }

  const distortedCanvasLayouts = report.layouts.filter((layout) =>
    Math.abs((layout.canvas.width / layout.canvas.height) - (320 / 200)) > 0.002
  );
  if (distortedCanvasLayouts.length) {
    report.findings.push({
      severity: 'medium',
      id: 'canvas-aspect-ratio',
      detail: 'rendered VGA canvas deviates from the native 320:200 aspect ratio',
      states: distortedCanvasLayouts.map((layout) => layout.name)
    });
  }

  const actionKeys = new Set(
    report.layouts[0].buttons
      .map((button) => button.key)
      .filter((key) => key > 0)
  );
  const missingTouchCommands = [
    [18, 'REPLAY'],
    [19, 'UNDO'],
    [20, 'THEME']
  ].filter(([key]) => !actionKeys.has(key));
  if (missingTouchCommands.length) {
    report.findings.push({
      severity: 'medium',
      id: 'touch-command-coverage',
      detail: `no wrapper buttons for ${missingTouchCommands.map((x) => x[1]).join(', ')}`,
      missing: missingTouchCommands
    });
  }

  const keyboardMismatches = report.input.keyboard.filter(
    (probe) => probe.seen.length !== 1 || probe.seen[0] !== probe.expected
  );
  if (keyboardMismatches.length) {
    report.findings.push({
      severity: 'medium',
      id: 'button-keyboard-activation',
      detail: 'focused wrapper buttons do not dispatch their own command on Space',
      mismatches: keyboardMismatches
    });
  }

  if (report.input.tab.activeId === 'canvas' &&
      report.input.tab.seen.includes(16)) {
    report.notes.push({
      id: 'tab-reserved-by-game',
      detail: 'Tab remains intentionally reserved for the action log while the game canvas is focused; M provides the same game command.'
    });
  }

  const unlabeledDirectionButtons = report.layouts[0].buttons.filter(
    (button) => ['↖','↑','↗','←','→','↙','↓','↘'].includes(button.text) &&
                !button.ariaLabel
  );
  if (unlabeledDirectionButtons.length) {
    report.findings.push({
      severity: 'low',
      id: 'direction-button-names',
      detail: 'symbol-only direction controls have no explicit accessible names',
      count: unlabeledDirectionButtons.length
    });
  }

  if (!report.layouts[0].canvasAccessibleName) {
    report.findings.push({
      severity: 'low',
      id: 'canvas-accessible-name',
      detail: 'game canvas has no accessible name'
    });
  }

  if (/user-scalable\s*=\s*no|maximum-scale\s*=\s*1/i.test(report.layouts[0].viewportMeta)) {
    report.findings.push({
      severity: 'low',
      id: 'viewport-zoom-lock',
      detail: `viewport meta restricts browser zoom: ${report.layouts[0].viewportMeta}`
    });
  }

  if (report.errors.length) throw new Error(report.errors.join(' | '));

  fs.writeFileSync(
    path.join(outDir, 'audit.json'),
    `${JSON.stringify(report, null, 2)}\n`
  );
  fs.writeFileSync(
    path.join(outDir, 'audit.txt'),
    [
      'PASS',
      `findings=${report.findings.length}`,
      ...report.findings.map((f) => `${f.severity.toUpperCase()} ${f.id}: ${f.detail}`),
      ...report.notes.map((n) => `NOTE ${n.id}: ${n.detail}`),
      ...report.layouts.map((l) =>
        `${l.name}: viewport=${l.viewport.width}x${l.viewport.height} scroll=${l.document.scrollWidth}x${l.document.scrollHeight} canvas=${l.canvas.width}x${l.canvas.height} touch=${l.touchDisplay}`
      )
    ].join('\n') + '\n'
  );
  console.log(fs.readFileSync(path.join(outDir, 'audit.txt'), 'utf8'));
} catch (error) {
  report.errors.push(String(error));
  fs.writeFileSync(
    path.join(outDir, 'audit.json'),
    `${JSON.stringify(report, null, 2)}\n`
  );
  fs.writeFileSync(
    path.join(outDir, 'audit.txt'),
    `FAIL: ${String(error)}\n`
  );
  throw error;
} finally {
  if (browser) await browser.close();
  server.kill('SIGTERM');
}
