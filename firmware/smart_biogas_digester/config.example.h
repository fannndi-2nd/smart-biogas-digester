#pragma once

// ============================================================
//  KONFIGURASI RAHASIA — SMART BIOGAS DIGESTER
// ============================================================
//  1. Salin file ini menjadi "config.h" di folder yang sama.
//  2. Isi nilai di bawah dengan milikmu.
//  3. JANGAN commit config.h. File itu sudah masuk .gitignore.
//
//  Di Arduino IDE: sketch ini akan menolak compile kalau config.h
//  belum ada, supaya tidak ada token yang tertulis di kode publik.
// ============================================================

// ---------- Blynk IoT ----------
// Ambil dari Blynk Console > Template > Info.
#define BLYNK_TEMPLATE_ID   "TMPL_XXXXXXXXX"
#define BLYNK_TEMPLATE_NAME "Smart Biogas Digester"
#define BLYNK_AUTH_TOKEN    "TOKEN_BLYNK_KAMU"

// ---------- WiFi 2,4 GHz ----------
// ESP32 tidak bisa terhubung ke WiFi 5 GHz.
#define WIFI_SSID "NAMA_WIFI_KAMU"
#define WIFI_PASS "PASSWORD_WIFI_KAMU"

// ---------- Google Apps Script ----------
// URL Web App yang berakhiran /exec, bukan /dev.
#define SHEETS_URL "https://script.google.com/macros/s/XXXXXXXXXXXX/exec"

// ---------- Identitas perangkat ----------
// Muncul di kolom "Device" pada Google Sheets.
#define DEVICE_NAME "biogas-01"
