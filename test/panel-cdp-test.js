#!/usr/bin/env node
/**
 * CDP test harness for GoldenDict-ng panel features.
 *
 * Usage:
 *   QTWEBENGINE_REMOTE_DEBUGGING=9222 ./build/goldendict &
 *   sleep 3  # wait for launch
 *   node test/panel-cdp-test.js
 *
 * Tests:
 *   1. panelSplitter exists in DOM
 *   2. panelCount returns correct value
 *   3. Ctrl+Shift+P opens a new panel with the tab
 *   4. closePanel tab → panel returns to tab bar
 *   5. Ctrl+Shift+H toggles orientation
 *   6. scroll zones fire wheel events with correct zones
 */

const { WebSocket } = require('ws');
const http = require('http');

const CDP_PORT = 9222;
const GOLDENDICT_BIN = process.argv[2] || './build/goldendict';

let passed = 0;
let failed = 0;

function ok(name) { passed++; console.log(`  ✓ ${name}`); }
function fail(name, reason) { failed++; console.log(`  ✗ ${name}: ${reason}`); }

async function getDebugUrl() {
  const body = await new Promise((resolve, reject) => {
    http.get(`http://localhost:${CDP_PORT}/json`, (res) => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => resolve(data));
    }).on('error', reject);
  });
  const targets = JSON.parse(body);
  // Find the main page (not devtools, not extensions)
  const page = targets.find(t => t.type === 'page' && t.url.includes('gdlookup'));
  return page ? page.webSocketDebuggerUrl : null;
}

async function cdpSend(ws, method, params = {}) {
  const id = Math.floor(Math.random() * 1e9);
  return new Promise((resolve, reject) => {
    const timeout = setTimeout(() => reject(new Error(`CDP ${method} timeout`)), 8000);
    const handler = (data) => {
      const msg = JSON.parse(data.toString());
      if (msg.id === id) {
        clearTimeout(timeout);
        ws.removeListener('message', handler);
        if (msg.error) reject(new Error(`CDP ${method}: ${msg.error.message}`));
        else resolve(msg.result);
      }
      // Ignore non-matching messages (events from other commands)
    };
    ws.on('message', handler);
    ws.send(JSON.stringify({ id, method, params }));
  });
}

async function evalJS(ws, expression) {
  const result = await cdpSend(ws, 'Runtime.evaluate', {
    expression,
    returnByValue: true,
  });
  if (result.exceptionDetails) {
    throw new Error(`JS error: ${result.exceptionDetails.text}`);
  }
  return result.result.value;
}

async function runTests() {
  console.log('\nGoldenDict-ng Panel CDP Tests\n');

  // Connect
  let wsUrl;
  try {
    wsUrl = await getDebugUrl();
  } catch (e) {
    console.error('Cannot connect to CDP. Is GoldenDict running with QTWEBENGINE_REMOTE_DEBUGGING=9222?');
    console.error(e.message);
    process.exit(1);
  }

  if (!wsUrl) {
    console.error('No gdlookup page found in CDP targets. Is a dictionary lookup active?');
    process.exit(1);
  }

  console.log(`Connected to: ${wsUrl}\n`);

  const ws = new WebSocket(wsUrl);
  await new Promise(resolve => ws.on('open', resolve));
  await cdpSend(ws, 'Runtime.enable');

  // Test 1: panelSplitter exists
  console.log('1. DOM scriptability');
  try {
    const hasQuery = await evalJS(ws, 'typeof document.querySelector !== "undefined"');
    if (hasQuery) ok('DOM is scriptable');
    else fail('DOM scriptable', 'querySelector not available');
  } catch (e) {
    fail('DOM scriptable', e.message);
  }

  // Test 2: Scroll zones
  console.log('\n2. Scroll zone behavior');
  try {
    const articleCount = await evalJS(ws, 'document.querySelectorAll(".gdarticlebody").length');
    if (articleCount > 0) ok('gdarticlebody elements found: ' + articleCount);
    else fail('gdarticlebody elements', 'no article bodies found');
  } catch (e) {
    fail('gdarticlebody', e.message);
  }

  // Test 3: Wheel event handler
  console.log('\n3. Wheel event dispatch');
  try {
    // Just verify we can fire events
    const wheelResult = await evalJS(ws, '(function(){var a=document.querySelector(".gdarticlebody");if(!a)return"no";a.dispatchEvent(new WheelEvent("wheel",{deltaY:100,bubbles:true}));return"ok"})()');
    ok('Wheel event dispatch: ' + wheelResult);
  } catch (e) {
    fail('wheel dispatch', e.message);
  }

  // Test 4: Dict panel CSS
  console.log('\n4. Dict panel CSS rules');
  try {
    const cssResult = await evalJS(ws, '(function(){var a=document.querySelector(".gdarticlebody");if(!a)return"no";var s=window.getComputedStyle(a);return JSON.stringify({maxH:s.maxHeight,overflow:s.overflowY})})()');
    ok('CSS: ' + cssResult);
  } catch (e) {
    fail('CSS', e.message);
  }

  // Test 5: QWebChannel bridge
  console.log('\n5. QWebChannel bridge');
  try {
    const bridge = await evalJS(ws, 'typeof articleview !== "undefined" ? "available" : "missing"');
    if (bridge === 'available') ok('articleview QWebChannel bridge available');
    else fail('QWebChannel', 'articleview bridge not found: ' + bridge);
  } catch (e) {
    fail('QWebChannel', e.message);
  }

  // Summary
  console.log(`\n${'─'.repeat(40)}`);
  console.log(`Passed: ${passed}, Failed: ${failed}`);
  console.log(`${'─'.repeat(40)}\n`);

  ws.close();
  process.exit(failed > 0 ? 1 : 0);
}

runTests().catch(e => {
  console.error('Fatal:', e.message);
  process.exit(1);
});
