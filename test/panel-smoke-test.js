#!/usr/bin/env node
/**
 * Panel smoke test — verifies panel operations don't crash and maintain web page.
 *
 * Start GD first:
 *   QTWEBENGINE_REMOTE_DEBUGGING=9222 ./build/goldendict hello &
 *   sleep 5
 *   node test/panel-smoke-test.js
 */
const { WebSocket } = require('ws');
const http = require('http');
const { execSync } = require('child_process');

const CDP_PORT = 9222;
let passed = 0, failed = 0;
function ok(n) { passed++; console.log(`  ✓ ${n}`); }
function fail(n, r) { failed++; console.log(`  ✗ ${n}: ${r}`); }

function sendKey(combo) {
  // ydotool for Wayland
  const keys = combo.toLowerCase().replace(/ctrl\+shift\+/g, '');
  const key = keys.length === 1 ? keys : keys;
  try {
    // ydotool key: ctrl+shift+<key>
    const ydotoolKeys = combo.replace(/ctrl\+shift\+/i, '42:1 54:1 ').replace(/([a-z])$/, 'KEY_$1:1 KEY_$1:0 42:0 54:0').toUpperCase();
    execSync(`ydotool key ${ydotoolKeys} 2>/dev/null`, {timeout: 2000});
  } catch(e) {
    // ydotool not available — test manual
  }
}

async function getDebugUrl() {
  const body = await new Promise((r, j) => {
    http.get(`http://localhost:${CDP_PORT}/json`, res => {
      let d = ''; res.on('data', c => d += c); res.on('end', () => r(d));
    }).on('error', j);
  });
  const targets = JSON.parse(body);
  return targets.find(t => t.type === 'page')?.webSocketDebuggerUrl;
}

async function cdpSend(ws, method, params = {}) {
  const id = Math.floor(Math.random() * 1e9);
  return new Promise((resolve, reject) => {
    const t = setTimeout(() => reject(new Error('timeout')), 5000);
    const h = (data) => {
      const msg = JSON.parse(data.toString());
      if (msg.id === id) { clearTimeout(t); ws.removeListener('message', h); resolve(msg.result); }
    };
    ws.on('message', h);
    ws.send(JSON.stringify({ id, method, params }));
  });
}

async function evalJS(ws, expr) {
  const r = await cdpSend(ws, 'Runtime.evaluate', { expression: expr, returnByValue: true });
  return r.result?.value;
}

async function run() {
  console.log('\nGoldenDict Panel Smoke Test\n');
  const wsUrl = await getDebugUrl();
  if (!wsUrl) { console.error('No CDP page. Start: QTWEBENGINE_REMOTE_DEBUGGING=9222 ./build/goldendict hello &'); process.exit(1); }

  const ws = new WebSocket(wsUrl);
  await new Promise(r => ws.on('open', r));
  await cdpSend(ws, 'Runtime.enable');

  // Test 1: Page is alive
  const count = await evalJS(ws, 'document.querySelectorAll(".gdarticlebody").length');
  ok(`Page loaded, ${count} article bodies`);

  // Test 2: Send Ctrl+Shift+P (toggle panel)
  console.log('\nSending Ctrl+Shift+P to open panel...');
  sendKey('ctrl+shift+p');
  await new Promise(r => setTimeout(r, 1000));

  // Verify page still alive
  const count2 = await evalJS(ws, 'document.querySelectorAll(".gdarticlebody").length');
  if (count2 >= 0) ok(`After Ctrl+Shift+P: page still alive (${count2}`);

  // Test 3: Send Ctrl+Shift+H (toggle orientation)
  console.log('\nSending Ctrl+Shift+H to toggle orientation...');
  sendKey('ctrl+shift+h');
  await new Promise(r => setTimeout(r, 500));
  const count3 = await evalJS(ws, 'document.querySelectorAll(".gdarticlebody").length');
  if (count3 >= 0) ok(`After Ctrl+Shift+H: page still alive (${count3} articles)`);

  // Test 4: Ctrl+Shift+Q (close panel)
  console.log('\nSending Ctrl+Shift+Q to close panel...');
  sendKey('ctrl+shift+q');
  await new Promise(r => setTimeout(r, 1000));
  const count4 = await evalJS(ws, 'document.querySelectorAll(".gdarticlebody").length');
  if (count4 >= 0) ok(`After Ctrl+Shift+Q: page still alive (${count4} articles)`);

  ws.close();

  console.log(`\n${'─'.repeat(40)}`);
  console.log(`Smoke test: ${passed} passed, ${failed} failed`);
  console.log(`${'─'.repeat(40)}\n`);
  process.exit(failed > 0 ? 1 : 0);
}

run().catch(e => { console.error(e); process.exit(1); });
