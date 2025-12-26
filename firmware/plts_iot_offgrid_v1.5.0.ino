/*************************************************************
   SISTEM MONITORING & KONTROL PLTS 50WP OFF GRID
   NodeMCU ESP8266 + LCD 20x4 I2C
   Sensor: DHT11, DS18B20, PZEM004T v3.0
   Platform: Blynk IoT
   Fitur: Watchdog, WiFi Monitor, RSSI, Relay Realtime
   Versi: 1.5.0 | Desember 2025 | Mas Willy
*************************************************************/

#define BLYNK_TEMPLATE_ID   "TMPL6x7si7P-L"
#define BLYNK_TEMPLATE_NAME "Sistem Monitoring PLTS 50 WP OFF Grid"
#define BLYNK_FIRMWARE_VERSION "1.5.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include "BlynkEdgent.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <PZEM004Tv30.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

/* ---------------- PIN DEFINITIONS ---------------- */
#define DHTPIN       D2
#define DHTTYPE      DHT11
#define ONE_WIRE_BUS D1
#define RELAY1       D5
#define RELAY2       D6
#define RELAY3       D7
#define RELAY4       D8
#define PZEM_RX      D7
#define PZEM_TX      D6

/* ---------------- OBJECTS ---------------- */
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
SoftwareSerial pzemSW(PZEM_RX, PZEM_TX);
PZEM004Tv30 pzem(pzemSW);
LiquidCrystal_I2C lcd(0x27, 20, 4);
Ticker watchdogTimer;

/* ---------------- SENSOR DATA ---------------- */
float suhuRuang = 0, kelembapan = 0, suhuBatt = 0;
float voltage = 0, current = 0, power = 0, energy = 0;
int wifiRSSI = 0;

/* ---------------- TIMING ---------------- */
unsigned long lastRead = 0;
const long interval = 5000;          // kirim data setiap 5 detik
unsigned long lastBlynkResponse = 0;
const unsigned long WATCHDOG_TIMEOUT = 60000; // reboot jika hang >60s

/* ---------------- CUSTOM ICONS ---------------- */
byte icon_batt[8] = {B01110,B11111,B11111,B11111,B11111,B11111,B11111,B11111};
byte icon_temp[8] = {B00100,B01010,B01010,B01010,B01010,B01110,B01110,B00100};
byte icon_drop[8] = {B00100,B00100,B01010,B01010,B10001,B10001,B01110,B00000};
byte icon_volt[8] = {B00100,B01110,B11111,B10101,B11111,B01110,B00100,B00000};
byte icon_watt[8] = {B00000,B01010,B10101,B11111,B10101,B01010,B00000,B00000};

/* ==========================================================
   SETUP
   ========================================================== */
void setup() {
  Serial.begin(115200);
  dht.begin();
  ds18b20.begin();

  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, icon_batt);
  lcd.createChar(1, icon_temp);
  lcd.createChar(2, icon_drop);
  lcd.createChar(3, icon_volt);
  lcd.createChar(4, icon_watt);
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("PLTS IoT System");
  lcd.setCursor(3, 2);
  lcd.print("By Mas Willy");
  delay(1500);
  lcd.clear();

  // Relay setup (aktif LOW)
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, HIGH);

  BlynkEdgent.begin();

  // Watchdog setiap 15 detik
  watchdogTimer.attach_ms(15000, checkSystemHealth);
}

/* ==========================================================
   LOOP
   ========================================================== */
void loop() {
  BlynkEdgent.run();  // tetap realtime untuk relay
  unsigned long now = millis();

  // Update sensor & LCD tiap 5 detik
  if (now - lastRead >= interval) {
    lastRead = now;
    readSensors();
    updateLCD();
    sendOnlineTime();
    sendWiFiSignal(); // kirim sinyal WiFi ke Blynk
  }
}

/* ==========================================================
   SENSOR READ FUNCTIONS
   ========================================================== */
void readSensors() {
  // DHT11
  kelembapan = dht.readHumidity();
  suhuRuang  = dht.readTemperature();

  // DS18B20
  ds18b20.requestTemperatures();
  suhuBatt = ds18b20.getTempCByIndex(0);

  // PZEM004T
  voltage = pzem.voltage();
  current = pzem.current();
  power   = pzem.power();
  energy  = pzem.energy();

  // Kirim ke Blynk (5 detik sekali)
  Blynk.virtualWrite(V1, suhuRuang);
  Blynk.virtualWrite(V2, kelembapan);
  Blynk.virtualWrite(V3, voltage);
  Blynk.virtualWrite(V4, current);
  Blynk.virtualWrite(V5, power);
  Blynk.virtualWrite(V6, energy);
  Blynk.virtualWrite(V7, suhuBatt);

  lastBlynkResponse = millis();
}

/* ==========================================================
   LCD DISPLAY UPDATE
   ========================================================== */
void updateLCD() {
  lcd.clear();

  // Baris 1: Volt, Arus, Status WiFi, RSSI
  lcd.setCursor(0, 0);
  lcd.write(3); lcd.print(":");
  lcd.print(voltage, 1); lcd.print("V ");
  lcd.print("I:"); lcd.print(current, 2); lcd.print("A");

  lcd.setCursor(15, 0);
  if (WiFi.status() == WL_CONNECTED) {
    wifiRSSI = WiFi.RSSI();
    lcd.print(wifiRSSI); // tampilkan sinyal (contoh -65)
  } else {
    lcd.print("NF"); // Not Found
  }

  // Baris 2: Daya & Energi
  lcd.setCursor(0, 1);
  lcd.write(4); lcd.print(":");
  lcd.print(power, 0); lcd.print("W ");
  lcd.print("E:");
  lcd.print(energy, 2); lcd.print("kWh");

  // Baris 3: Suhu Baterai
  lcd.setCursor(0, 2);
  lcd.write(0); lcd.print(":");
  lcd.print(suhuBatt, 1); lcd.print((char)223); lcd.print("C");

  // Baris 4: Suhu & Kelembapan Ruang
  lcd.setCursor(0, 3);
  lcd.write(1); lcd.print(":");
  lcd.print(suhuRuang, 1); lcd.print((char)223); lcd.print("C ");
  lcd.write(2); lcd.print(":");
  lcd.print(kelembapan, 0); lcd.print("%");
}

/* ==========================================================
   MONITORING FUNGSI TAMBAHAN
   ========================================================== */
void sendOnlineTime() {
  Blynk.virtualWrite(V0, millis() / 1000);   // waktu aktif ke Blynk
}

void sendWiFiSignal() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiRSSI = WiFi.RSSI();
    Blynk.virtualWrite(V8, wifiRSSI);  // kirim RSSI ke Blynk (V8)
  } else {
    Blynk.virtualWrite(V8, -100); // bila tidak terhubung
  }
}

void checkSystemHealth() {
  unsigned long now = millis();
  if ((now - lastBlynkResponse) > WATCHDOG_TIMEOUT) {
    Serial.println("⚠️ Hang/Koneksi putus >60s → Reboot");
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("System Rebooting...");
    delay(1000);
    ESP.restart();
  }
}

/* ==========================================================
   RELAY CONTROL (REALTIME)
   ========================================================== */
BLYNK_WRITE(V10) { digitalWrite(RELAY1, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V11) { digitalWrite(RELAY2, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V12) { digitalWrite(RELAY3, param.asInt() ? LOW : HIGH); }
BLYNK_WRITE(V13) { digitalWrite(RELAY4, param.asInt() ? LOW : HIGH); }
