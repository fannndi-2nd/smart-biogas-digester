/* ============================================================
 *  SMART BIOGAS DIGESTER — FIRMWARE ESP32
 *  ------------------------------------------------------------
 *  Membaca kadar CH4 (MQ-4), suhu & kelembapan (DHT22),
 *  menampilkan di LCD I2C, mengirim ke Blynk IoT, dan
 *  mencatat log ke Google Sheets lewat Apps Script.
 *
 *  Board   : ESP32 DevKit V1 (30 pin)
 *  Sensor  : MQ-4 (AO -> GPIO34), DHT22 (DATA -> GPIO4)
 *  LCD     : I2C 16x2, alamat 0x27 (SDA -> GPIO21, SCL -> GPIO22)
 *
 *  SEBELUM UPLOAD:
 *  1. Salin config.example.h menjadi config.h
 *  2. Isi WiFi, token Blynk, dan URL Apps Script
 *  3. Jangan commit config.h (sudah ada di .gitignore)
 * ============================================================ */

#if __has_include("config.h")
  #include "config.h"
#else
  #error "config.h belum ada. Salin config.example.h menjadi config.h lalu isi nilainya."
#endif

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <math.h>

// ---------------- PIN & PERANGKAT ----------------
#define MQ4_PIN       34
#define DHT_PIN        4
#define DHT_TYPE       DHT22
#define LCD_ADDR       0x27
#define LCD_COLS       16
#define LCD_ROWS        2
#define I2C_SDA        21
#define I2C_SCL        22

// ---------------- RANGKAIAN MQ-4 ----------------
// MQ-4 diberi 5 V. Output AO melewati pembagi tegangan
// R1 = 1 kOhm (seri) dan R2 = 2 kOhm (ke GND), sehingga
// tegangan masuk ESP32 maksimum sekitar 3,3 V.
const float MQ4_VCC   = 5.0f;
const float ADC_VREF  = 3.3f;
const int   ADC_MAX   = 4095;
const float RL        = 1000.0f;   // beban MQ-4 (Ohm)
const float R0        = 1000.0f;   // hasil kalibrasi di udara bersih
const float R1        = 1000.0f;
const float R2        = 2000.0f;
const float MQ_A      = 1012.7f;   // konstanta kurva datasheet MQ-4
const float MQ_B      = -2.786f;

// ---------------- AMBANG & KALIBRASI ----------------
const float CH4_WARN_PPM   = 1000.0f;  // early warning (~2% LEL)
const float CH4_DANGER_PPM = 5000.0f;  // bahaya (~10% LEL)
const float CH4_HYST_PPM   = 100.0f;   // histeresis agar status tidak berkedip
const float HUM_OFFSET     = -7.0f;    // koreksi kelembapan DHT22
const float TEMP_OFFSET    = 0.0f;     // koreksi suhu DHT22

// ---------------- INTERVAL ----------------
const unsigned long WARMUP_MS          = 180000UL; // 3 menit stabilisasi
const unsigned long SENSOR_INTERVAL_MS = 2000UL;
const unsigned long LCD_INTERVAL_MS    = 3000UL;
const unsigned long SHEETS_INTERVAL_MS = 60000UL;
const unsigned long WIFI_CHECK_MS      = 30000UL;

// ---------------- OBJEK ----------------
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

float gPpm   = 0.0f;
float gHum   = 0.0f;
float gTemp  = 0.0f;
int   gStatus = 0;          // 0=WARMUP, 1=AMAN, 2=WASPADA, 3=BAHAYA
unsigned long gBootMs = 0;
uint8_t gLcdPage = 0;

// ---------------- STATUS ----------------
const char* statusText(int status) {
  switch (status) {
    case 0: return "WARMUP";
    case 2: return "WASPADA";
    case 3: return "BAHAYA";
    default: return "AMAN";
  }
}

int nextStatus(float ppm, int previous) {
  if (millis() - gBootMs < WARMUP_MS) return 0;

  if (ppm >= CH4_DANGER_PPM) return 3;
  if (previous == 3 && ppm >= (CH4_DANGER_PPM - CH4_HYST_PPM)) return 3;

  if (ppm >= CH4_WARN_PPM) return 2;
  if (previous == 2 && ppm >= (CH4_WARN_PPM - CH4_HYST_PPM)) return 2;

  return 1;
}

// ---------------- SENSOR MQ-4 ----------------
float readMq4Ppm() {
  long total = 0;
  for (int i = 0; i < 16; i++) {
    total += analogRead(MQ4_PIN);
    delay(2);
  }

  float adc = total / 16.0f;
  float vOut = (adc / (float)ADC_MAX) * ADC_VREF;
  float vSensor = vOut * ((R1 + R2) / R2);

  if (vSensor < 0.01f) vSensor = 0.01f;
  if (vSensor > (MQ4_VCC - 0.01f)) vSensor = MQ4_VCC - 0.01f;

  float rs = ((MQ4_VCC - vSensor) / vSensor) * RL;
  float ratio = rs / R0;
  float ppm = MQ_A * pow(ratio, MQ_B);

  if (!isfinite(ppm) || ppm < 0.0f) ppm = 0.0f;
  return ppm;
}

// ---------------- SENSOR DHT22 ----------------
void readDht() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("[DHT] gagal membaca, memakai nilai terakhir");
    return;
  }

  hum += HUM_OFFSET;
  temp += TEMP_OFFSET;

  if (hum < 0.0f) hum = 0.0f;
  if (hum > 100.0f) hum = 100.0f;

  gHum = hum;
  gTemp = temp;
}

// ---------------- LCD ----------------
void renderLcd() {
  char line[17];
  lcd.clear();

  switch (gLcdPage % 3) {
    case 0:
      snprintf(line, sizeof(line), "CH4:%7.1f ppm", gPpm);
      lcd.setCursor(0, 0); lcd.print(line);
      snprintf(line, sizeof(line), "Status:%-9s", statusText(gStatus));
      lcd.setCursor(0, 1); lcd.print(line);
      break;

    case 1:
      snprintf(line, sizeof(line), "Hum :%6.1f %%", gHum);
      lcd.setCursor(0, 0); lcd.print(line);
      snprintf(line, sizeof(line), "Temp:%6.1f C", gTemp);
      lcd.setCursor(0, 1); lcd.print(line);
      break;

    default: {
      snprintf(line, sizeof(line), "WiFi:%4d dBm", WiFi.RSSI());
      lcd.setCursor(0, 0); lcd.print(line);

      String ip = WiFi.localIP().toString();
      if (ip.length() > 12) ip = ip.substring(0, 12);
      snprintf(line, sizeof(line), "IP:%s", ip.c_str());
      lcd.setCursor(0, 1); lcd.print(line);
      break;
    }
  }
}

// ---------------- BLYNK ----------------
void sendToBlynk() {
  if (!Blynk.connected()) return;

  Blynk.virtualWrite(V0, statusText(gStatus));
  Blynk.virtualWrite(V1, gHum);
  Blynk.virtualWrite(V2, gPpm);
  Blynk.virtualWrite(V3, gTemp);
  Blynk.virtualWrite(V4, WiFi.RSSI());
}

// ---------------- GOOGLE SHEETS ----------------
void sendToSheets() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (strlen(SHEETS_URL) < 12) return;

  HTTPClient http;
  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  http.begin(SHEETS_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = String("{\"device\":\"") + DEVICE_NAME + "\""
                 + ",\"ppm\":"   + String(gPpm, 2)
                 + ",\"hum\":"   + String(gHum, 2)
                 + ",\"temp\":"  + String(gTemp, 2)
                 + ",\"status\":\"" + statusText(gStatus) + "\""
                 + ",\"rssi\":"  + String(WiFi.RSSI())
                 + ",\"uptime\":" + String(millis() / 1000UL)
                 + "}";

  int code = http.POST(payload);
  if (code > 0) {
    Serial.printf("[Sheets] HTTP %d\n", code);
  } else {
    Serial.printf("[Sheets] gagal: %s\n", http.errorToString(code).c_str());
  }
  http.end();
}

// ---------------- TIMER CALLBACKS ----------------
void sensorTick() {
  gPpm = readMq4Ppm();
  readDht();
  gStatus = nextStatus(gPpm, gStatus);

  Serial.printf("CH4 %.1f ppm | Hum %.1f %% | Temp %.1f C | %s | RSSI %d dBm\n",
                gPpm, gHum, gTemp, statusText(gStatus), WiFi.RSSI());

  sendToBlynk();
}

void lcdTick() {
  gLcdPage = (gLcdPage + 1) % 3;
  renderLcd();
}

void sheetsTick() {
  sendToSheets();
}

void connectionTick() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] terputus, mencoba reconnect...");
    WiFi.reconnect();
    return;
  }
  if (!Blynk.connected()) {
    Serial.println("[Blynk] terputus, mencoba connect...");
    Blynk.connect(5000);
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Smart Biogas");
  lcd.setCursor(0, 1); lcd.print("Booting...");

  dht.begin();
  analogReadResolution(12);
  analogSetPinAttenuation(MQ4_PIN, ADC_11db);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  lcd.setCursor(0, 1); lcd.print("WiFi...         ");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000UL) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(5000);

  gBootMs = millis();
  timer.setInterval(SENSOR_INTERVAL_MS, sensorTick);
  timer.setInterval(LCD_INTERVAL_MS, lcdTick);
  timer.setInterval(SHEETS_INTERVAL_MS, sheetsTick);
  timer.setInterval(WIFI_CHECK_MS, connectionTick);
  timer.setTimeout(10000L, sheetsTick);   // log pertama setelah 10 detik

  renderLcd();

  Serial.println();
  Serial.println("=== SMART BIOGAS DIGESTER ===");
  Serial.printf("WiFi   : %s\n", WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "belum terhubung");
  Serial.printf("Blynk  : %s\n", Blynk.connected() ? "online" : "offline");
  Serial.println("Warm-up MQ-4: 3 menit (status WARMUP)");
  Serial.println("Catatan: MQ-4 butuh burn-in 24-48 jam untuk akurasi terbaik.");
}

// ---------------- LOOP ----------------
void loop() {
  Blynk.run();
  timer.run();
}
