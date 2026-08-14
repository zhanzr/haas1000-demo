'use strict';

/* --------------------------------------------------------------------------
 * e_server front-end: single page, three tabs, no external libraries.
 * Talks to the reference (and embedded) C backend over relative /api paths.
 * ------------------------------------------------------------------------ */

async function getJSON(url) {
  const r = await fetch(url, { cache: 'no-store' });
  if (!r.ok) throw new Error('HTTP ' + r.status);
  return r.json();
}

async function postJSON(url, obj) {
  const r = await fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(obj),
    cache: 'no-store'
  });
  if (!r.ok) throw new Error('HTTP ' + r.status);
  return r.json();
}

function setStatus(id, text, cls) {
  const el = document.getElementById(id);
  el.textContent = text;
  el.className = 'status' + (cls ? ' ' + cls : '');
}

/* ---------------------------------- tabs --------------------------------- */

const tabButtons = Array.from(document.querySelectorAll('.tab-btn'));

function showTab(name) {
  tabButtons.forEach(b => b.classList.toggle('active', b.dataset.tab === name));
  document.querySelectorAll('.panel').forEach(p => {
    p.classList.toggle('active', p.id === 'tab-' + name);
  });
  if (name === 'led') loadLeds();
  if (name === 'adc') { startAdc(); redrawAll(); }
  if (name === 'board') loadBoardInfo();
}
tabButtons.forEach(b => b.addEventListener('click', () => showTab(b.dataset.tab)));

/* ----------------------------- Tab 1: LEDs ------------------------------- */

const LED_NAMES = [
  'LED1 (GPIO40)', 'LED2 (GPIO41)', 'LED3 (GPIO36)', 'LED4 (GPIO35)', 'LED5 (GPIO34)'
];
let ledStates = [0, 0, 0, 0, 0];

function renderLeds() {
  const list = document.getElementById('led-list');
  list.innerHTML = '';
  ledStates.forEach((s, i) => {
    const label = document.createElement('label');
    label.className = 'led';
    const cb = document.createElement('input');
    cb.type = 'checkbox';
    cb.checked = !!s;
    cb.addEventListener('change', () => setLed(i, cb.checked));
    label.appendChild(cb);
    label.appendChild(document.createTextNode(LED_NAMES[i]));
    list.appendChild(label);
  });
}

async function loadLeds() {
  try {
    const j = await getJSON('/api/leds');
    ledStates = (j.leds && j.leds.length === 5) ? j.leds.slice() : [0, 0, 0, 0, 0];
    renderLeds();
  } catch (e) {
    setStatus('led-status', 'failed to load LED state', 'err');
  }
}

let ledBusy = false;
let ledDirty = false;

async function setLed(i, on) {
  ledStates[i] = on ? 1 : 0;
  if (ledBusy) { ledDirty = true; return; }   /* coalesce clicks mid-flight */
  ledBusy = true;
  ledDirty = false;
  setStatus('led-status', 'sending…', '');
  try {
    do {
      const sent = ledStates.slice();
      const j = await postJSON('/api/leds', { leds: sent });
      ledStates = (j.leds && j.leds.length === 5) ? j.leds.slice() : ledStates;
      ledDirty = JSON.stringify(sent) !== JSON.stringify(ledStates);
    } while (ledDirty);
    renderLeds();
    setStatus('led-status', 'ok', 'ok');
  } catch (e) {
    setStatus('led-status', 'failed — ' + e.message, 'err');
    loadLeds();                   /* re-sync checkboxes with the server */
  } finally {
    ledBusy = false;
  }
}

/* ------------------------------ Tab 2: ADC ------------------------------- */

const MAX_SAMPLES = 60;

const plots = {
  ch0: { canvasId: 'plot-ch0', label: 'mV', min: 0,    max: 4000, dec: 0, buf: [] },
  bat: { canvasId: 'plot-bat', label: 'mV', min: 0,    max: 4000, dec: 0, buf: [] },
  ch2: { canvasId: 'plot-ch2', label: 'mV', min: 0,    max: 1000, dec: 0, buf: [] }
};

let adcTimer = null;
let lastAdc = null;

const intervalSel = document.getElementById('adc-interval');

function adcIntervalMs() { return parseInt(intervalSel.value, 10); }

function startAdc() {
  stopAdc();
  adcTimer = setInterval(sampleAdc, adcIntervalMs());
  sampleAdc();
}

function stopAdc() {
  if (adcTimer) { clearInterval(adcTimer); adcTimer = null; }
}

intervalSel.addEventListener('change', () => {
  try { localStorage.setItem('adc-interval', intervalSel.value); } catch (e) {}
  if (adcTimer) { startAdc(); }
});

function pushSample(key, val) {
  const p = plots[key];
  if (typeof val !== 'number' || !isFinite(val)) {
    val = p.buf.length ? p.buf[p.buf.length - 1] : p.min;   /* previous value */
  }
  p.buf.push(val);
  if (p.buf.length > MAX_SAMPLES) p.buf.shift();
}

async function sampleAdc() {
  let v;
  try {
    v = await getJSON('/api/adc');
  } catch (e) {
    v = lastAdc;                  /* timeout / no data: use previous values */
  }
  if (!v) return;
  lastAdc = v;
  /* All three channels are returned in millivolts. */
  pushSample('ch0', v.ch0_mv);
  pushSample('bat', v.bat_mv);
  pushSample('ch2', v.ch2_mv);
  redrawAll();
}

function drawPlot(p) {
  const cv = document.getElementById(p.canvasId);
  const w = cv.clientWidth;
  const h = cv.clientHeight;
  if (w === 0 || h === 0) return;          /* panel hidden */
  cv.width = Math.round(w * devicePixelRatio);
  cv.height = Math.round(h * devicePixelRatio);
  const ctx = cv.getContext('2d');
  ctx.scale(devicePixelRatio, devicePixelRatio);
  ctx.clearRect(0, 0, w, h);

  const padL = 6, padR = 8, padT = 6, padB = 16;
  const pw = w - padL - padR, ph = h - padT - padB;
  const span = p.max - p.min;

  /* grid */
  ctx.strokeStyle = '#232a37';
  ctx.lineWidth = 1;
  for (let g = 0; g < 4; g++) {
    const y = padT + (g / 3) * ph;
    ctx.beginPath();
    ctx.moveTo(padL, y);
    ctx.lineTo(w - padR, y);
    ctx.stroke();
  }

  /* axis labels */
  ctx.fillStyle = '#8b93a7';
  ctx.font = '10px monospace';
  ctx.textAlign = 'left';
  ctx.fillText(p.max.toFixed(p.dec), padL, padT - 3);
  ctx.fillText(p.min.toFixed(p.dec), padL, h - padB + 10);

  /* series */
  const buf = p.buf, n = buf.length;
  if (n >= 2) {
    ctx.strokeStyle = '#4ea1ff';
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    for (let i = 0; i < n; i++) {
      const x = padL + (i / (MAX_SAMPLES - 1)) * pw;
      const y = padT + ph - ((buf[i] - p.min) / span) * ph;
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    }
    ctx.stroke();
  }

  /* latest value - centered horizontally so it never overlaps the gray
   * min/max axis labels drawn on the left */
  if (n > 0) {
    ctx.fillStyle = '#cfe3ff';
    ctx.textAlign = 'center';
    ctx.fillText(buf[n - 1].toFixed(p.dec) + ' ' + p.label, w / 2, h - 3);
  }
}

function redrawAll() {
  Object.keys(plots).forEach(k => drawPlot(plots[k]));
}

window.addEventListener('resize', redrawAll);

/* ---------------------------- Tab 3: Board info -------------------------- */

async function loadBoardInfo() {
  const set = (id, v) => {
    document.getElementById(id).textContent =
      (v === null || v === undefined || v === '') ? 'N/A' : v;
  };
  set('info-arch', '…');
  set('info-lan', '…');
  set('info-pub', '…');
  set('info-geo', '…');
  set('info-weather', '…');

  let j;
  try {
    j = await getJSON('/api/info');
  } catch (e) {
    set('info-arch', 'N/A');
    set('info-lan', 'N/A');
    set('info-pub', 'N/A');
    set('info-geo', 'N/A');
    set('info-weather', 'N/A');
    return;
  }

  set('info-arch', j.arch);
  set('info-lan', j.lan_ip);
  set('info-pub', j.public_ip);
  if (j.geo && (j.geo.city || j.geo.country)) {
    set('info-geo', [j.geo.city, j.geo.country].filter(Boolean).join(', '));
  } else {
    set('info-geo', 'N/A');
  }
  if (j.weather && j.weather.temp_c !== null && j.weather.temp_c !== undefined) {
    const parts = [j.weather.temp_c + ' °C'];
    if (j.weather.desc) parts.push(j.weather.desc);
    set('info-weather', parts.join(' · '));
  } else {
    set('info-weather', 'N/A');
  }
}

document.getElementById('info-refresh').addEventListener('click', loadBoardInfo);

/* --------------------------------- init ---------------------------------- */

(function init() {
  /* restore the persisted sample interval, default 1 s */
  try {
    const saved = localStorage.getItem('adc-interval');
    if (saved && ['1000', '2000', '4000'].includes(saved)) intervalSel.value = saved;
  } catch (e) {}
  showTab('led');
})();
