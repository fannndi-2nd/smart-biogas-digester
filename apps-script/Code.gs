/**
 * ============================================================
 *  SMART BIOGAS DIGESTER — GOOGLE APPS SCRIPT
 * ============================================================
 *  Script ini menempel pada Google Spreadsheet (bound script).
 *
 *  CARA PASANG:
 *  1. Buat Google Spreadsheet baru, beri nama "Log Smart Biogas".
 *  2. Menu Extensions > Apps Script.
 *  3. Hapus isi Code.gs, tempel seluruh file ini, lalu Save.
 *  4. Jalankan fungsi setup() sekali (pilih setup > Run > izinkan akses).
 *  5. Deploy > New deployment > Web app.
 *       - Execute as       : Me
 *       - Who has access   : Anyone
 *  6. Salin URL yang berakhiran /exec.
 *  7. Tempel URL itu di config.h (SHEETS_URL) dan di dashboard.
 *
 *  Catatan keamanan:
 *  - URL /exec sebaiknya tidak dibagikan sembarangan.
 *  - Kalau ingin kunci sederhana, isi Script Property "API_KEY",
 *    lalu tambahkan ?key=KUNCI pada URL di config.h.
 * ============================================================
 */

const SHEET_NAME = 'Log';
const TIMEZONE   = 'Asia/Jakarta';
const MAX_ROWS   = 20000;

/**
 * Jalankan sekali untuk membuat sheet dan header.
 */
function setup() {
  const sheet = getSheet();
  const headers = [
    'Timestamp',
    'Device',
    'CH4 (ppm)',
    'Kelembapan (%)',
    'Suhu (C)',
    'Status',
    'RSSI (dBm)',
    'Uptime (s)'
  ];

  sheet.getRange(1, 1, 1, headers.length)
    .setValues([headers])
    .setFontWeight('bold')
    .setBackground('#0b3d2e')
    .setFontColor('#ffffff')
    .setVerticalAlignment('middle');

  sheet.setFrozenRows(1);
  sheet.setColumnWidth(1, 165);
  sheet.setColumnWidth(2, 110);
  sheet.setColumnWidth(3, 105);
  sheet.setColumnWidth(4, 125);
  sheet.setColumnWidth(5, 95);
  sheet.setColumnWidth(6, 100);
  sheet.setColumnWidth(7, 105);
  sheet.setColumnWidth(8, 95);
  sheet.getRange('A:A').setNumberFormat('yyyy-mm-dd hh:mm:ss');

  return 'Setup selesai. Sheet "Log" siap dipakai.';
}

/**
 * Dipanggil ESP32 setiap 60 detik (HTTP POST JSON).
 */
function doPost(e) {
  try {
    if (!isAuthorized(e)) {
      return jsonOutput({ ok: false, error: 'unauthorized' });
    }

    if (!e || !e.postData || !e.postData.contents) {
      return jsonOutput({ ok: false, error: 'payload kosong' });
    }

    const data = JSON.parse(e.postData.contents);
    const sheet = getSheet();

    const row = [
      new Date(),
      String(data.device || 'biogas-01'),
      toNumber(data.ppm),
      toNumber(data.hum),
      toNumber(data.temp),
      String(data.status || '-'),
      toNumber(data.rssi),
      toNumber(data.uptime)
    ];

    sheet.appendRow(row);
    trimRows(sheet);

    return jsonOutput({ ok: true, row: sheet.getLastRow() });
  } catch (err) {
    return jsonOutput({ ok: false, error: String(err) });
  }
}

/**
 * Dipanggil dashboard untuk membaca riwayat.
 * Contoh: /exec?limit=300
 * Fallback JSONP: /exec?limit=300&callback=cb
 */
function doGet(e) {
  try {
    if (!isAuthorized(e)) {
      return webOutput({ ok: false, error: 'unauthorized' }, e);
    }

    const limit = Math.min(Math.max(toNumber(e && e.parameter && e.parameter.limit) || 300, 1), 2000);
    const sheet = getSheet();
    const lastRow = sheet.getLastRow();

    if (lastRow < 2) {
      return webOutput({ ok: true, rows: [] }, e);
    }

    const startRow = Math.max(2, lastRow - limit + 1);
    const values = sheet.getRange(startRow, 1, lastRow - startRow + 1, 8).getValues();

    const rows = values.map(function (r) {
      return {
        ts: formatDate(r[0]),
        device: r[1],
        ppm: toNumber(r[2]),
        hum: toNumber(r[3]),
        temp: toNumber(r[4]),
        status: r[5],
        rssi: toNumber(r[6]),
        uptime: toNumber(r[7])
      };
    });

    return webOutput({ ok: true, count: rows.length, rows: rows }, e);
  } catch (err) {
    return webOutput({ ok: false, error: String(err) }, e);
  }
}

// ---------------- HELPER ----------------

function getSheet() {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let sheet = ss.getSheetByName(SHEET_NAME);
  if (!sheet) {
    sheet = ss.insertSheet(SHEET_NAME);
  }
  return sheet;
}

function toNumber(value) {
  const n = Number(value);
  return isFinite(n) ? n : 0;
}

function formatDate(value) {
  if (!value) return '';
  const date = value instanceof Date ? value : new Date(value);
  return Utilities.formatDate(date, TIMEZONE, "yyyy-MM-dd'T'HH:mm:ssXXX");
}

function trimRows(sheet) {
  const lastRow = sheet.getLastRow();
  if (lastRow > MAX_ROWS) {
    const excess = lastRow - MAX_ROWS;
    sheet.deleteRows(2, excess);
  }
}

function isAuthorized(e) {
  const key = PropertiesService.getScriptProperties().getProperty('API_KEY');
  if (!key) return true;
  const supplied = e && e.parameter ? e.parameter.key : '';
  return supplied === key;
}

function jsonOutput(obj) {
  return ContentService
    .createTextOutput(JSON.stringify(obj))
    .setMimeType(ContentService.MimeType.JSON);
}

function webOutput(obj, e) {
  const body = JSON.stringify(obj);
  const callback = e && e.parameter ? e.parameter.callback : '';

  if (callback) {
    return ContentService
      .createTextOutput(callback + '(' + body + ');')
      .setMimeType(ContentService.MimeType.JAVASCRIPT);
  }

  return ContentService
    .createTextOutput(body)
    .setMimeType(ContentService.MimeType.JSON);
}

// ---------------- UJI MANUAL ----------------

function testPost() {
  const fake = {
    postData: {
      contents: JSON.stringify({
        device: 'biogas-test',
        ppm: 812.4,
        hum: 88.2,
        temp: 35.6,
        status: 'AMAN',
        rssi: -54,
        uptime: 120
      })
    }
  };
  Logger.log(doPost(fake).getContent());
}

function testGet() {
  Logger.log(doGet({ parameter: { limit: '5' } }).getContent());
}
