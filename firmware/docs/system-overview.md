# Project IoT

# 📘 **Sistem Monitoring & Kontrol PLTS Offgrid 50 Wp Berbasis IoT (Blynk & ESP8266)**

**Disusun oleh : Mas Willy**

**Versi : 1.5.0 — Desember 2025**

---

## 🔆 1. Ringkasan Sistem / System Overview

Sistem ini berfungsi untuk **memantau dan mengontrol PLTS Offgrid 50 Wp** secara real-time menggunakan **NodeMCU ESP8266** dan **Blynk IoT Cloud**.

Data sensor seperti **tegangan, arus, daya, energi, suhu, dan kelembapan** dikirim ke **Blynk setiap 5 detik**, serta ditampilkan langsung di **LCD 20×4 I²C**.

Fitur tambahan:

- **Relay real-time control** (ON/OFF tanpa delay)
- **Watchdog otomatis reboot jika hang > 60 detik**
- **Indikator sinyal WiFi (RSSI)** di LCD dan Blynk

---

## 🧩 2. Komponen & Fungsi / Components and Functions

| Komponen | Fungsi | English Description |
| --- | --- | --- |
| **NodeMCU ESP8266** | Mikrokontroler utama | IoT microcontroller |
| **DHT11** | Sensor suhu & kelembapan ruangan | Ambient temperature & humidity |
| **DS18B20** | Sensor suhu baterai | Battery temperature |
| **PZEM-004T v3.0** | Sensor tegangan, arus, daya, energi inverter | AC voltage, current, power, energy |
| **Relay 4 Channel** | Pengendali beban (lampu, pompa, kipas) | Load control |
| **LCD 20x4 I²C** | Tampilan lokal | Real-time local display |
| **Blynk IoT Cloud** | Platform monitoring & kontrol | Cloud-based dashboard |

---

## ⚙️ 3. Koneksi Hardware / Hardware Wiring

| Komponen | Pin NodeMCU | Keterangan |
| --- | --- | --- |
| **DHT11** | D2 (GPIO4) | DATA → D2, VCC → 3.3V |
| **DS18B20** | D1 (GPIO5) | DATA → D1, VCC → 3.3V, resistor 4.7kΩ pull-up |
| **PZEM004T v3.0** | RX → D7, TX → D6 | VCC → 5V, GND → GND |
| **LCD 20×4 I²C** | SDA → D2, SCL → D1 | Address: 0x27 atau 0x3F |
| **Relay 4 Channel** | IN1–IN4 → D5, D6, D7, D8 | Aktif LOW (LOW = ON) |
| **Catu Daya** | 5V @ 2A | Untuk NodeMCU + relay board |

> Semua GND wajib disatukan (common ground).
> 

---

## 🧠 4. Diagram Sistem / System Diagram

```
[ Panel Surya 50Wp ]
        ↓
[ Baterai 12V 20Ah ]
        ↓
[ Inverter 220V AC ]
        ↓
 ┌─────────────────────────────┐
 │DHT11 |DS18B20 |PZEM004T │
 └─────────────────────────────┘
        ↓
[ NodeMCU ESP8266 ]
   ├──LCD20×4I²C
   ├──Relay4Channel
   ↓
[ Blynk IoT Cloud ↔ Smartphone ]

```

---

## 📡 5. Virtual Pin Blynk Mapping

| Virtual Pin | Data | Widget | Keterangan |
| --- | --- | --- | --- |
| **V0** | Waktu online (detik) | Label | Internal uptime |
| **V1** | Suhu Ruangan (°C) | Gauge | DHT11 |
| **V2** | Kelembapan (%) | Gauge | DHT11 |
| **V3** | Tegangan (V) | Gauge | PZEM004T |
| **V4** | Arus (A) | Gauge | PZEM004T |
| **V5** | Daya (W) | Gauge / Chart | PZEM004T |
| **V6** | Energi (kWh) | Label | PZEM004T |
| **V7** | Suhu Baterai (°C) | Gauge | DS18B20 |
| **V8** | Kekuatan Sinyal WiFi (RSSI) | Gauge / Label | WiFi Monitor |
| **V10–V13** | Relay 1–4 | Switch | Kontrol Beban |

---

## 📺 6. Tampilan LCD 20×4

LCD menampilkan data real-time:

```
⚡:220.1V I:0.52A -65
W:115W E:0.028kWh
🔋:28.6°C
🌡️:30.2°C 💧:65%

```

**Keterangan:**

- Angka `65` menunjukkan **kekuatan sinyal WiFi (RSSI)** dalam dBm.
- “OK” diganti dengan nilai RSSI jika tersambung.
- Jika WiFi terputus → tampil “NF” (*Not Found*).
- Data diperbarui setiap **5 detik**.

---

## 📶 7. Fitur RSSI / WiFi Signal

Fungsi `WiFi.RSSI()` digunakan untuk membaca kekuatan sinyal:

- Nilai dikirim ke **Blynk (V8)** setiap 5 detik
- Ditampilkan juga di LCD baris pertama

| Nilai RSSI | Keterangan | Status |
| --- | --- | --- |
| `>-50 dBm` | Sinyal sangat kuat | ✅ Excellent |
| `-60…-70 dBm` | Normal / stabil | ⚙️ OK |
| `<-80 dBm` | Lemah / sering putus | ⚠️ Weak |
| `-100 dBm` | Tidak ada sinyal | ❌ Disconnected |

---

## 🔒 8. Watchdog & Auto Reboot

### Fungsi:

- Jika **tidak ada update data > 60 detik**, sistem akan otomatis reboot.
- Menghindari sistem hang karena WiFi atau crash library.

**Variabel utama:**

| Variabel | Fungsi |
| --- | --- |
| `lastBlynkResponse` | Waktu terakhir kirim data ke Blynk |
| `WATCHDOG_TIMEOUT` | Waktu batas reboot (60 detik) |
| `ESP.restart()` | Reset software |

Pesan di LCD saat reboot:

```
System Rebooting...

```

---

## ⚙️ 9. Interval & Realtime System

| Komponen | Interval | Catatan |
| --- | --- | --- |
| **Sensor (DHT, DS18B20, PZEM)** | 5 detik | Non-blocking (pakai millis) |
| **Update LCD** | 5 detik | Sinkron sensor |
| **Kirim Data ke Blynk** | 5 detik | Termasuk RSSI |
| **Relay Control** | Realtime | Eksekusi langsung dari server |
| **Watchdog Check** | 15 detik | Reboot bila freeze 60s |

---

## 🧩 10. Fitur Utama Versi 1.5.0

| No | Fitur | Deskripsi |
| --- | --- | --- |
| 1 | Monitoring multi-sensor | PZEM004T, DHT11, DS18B20 |
| 2 | LCD 20x4 I²C dengan ikon kustom | ⚡ 🔋 🌡️ 💧 Watt |
| 3 | Kontrol 4 relay real-time via Blynk | Tanpa delay |
| 4 | Watchdog otomatis reboot | Stabil 24/7 |
| 5 | Indikator RSSI (WiFi Signal) | LCD + Blynk (V8) |
| 6 | Interval data efisien | 5 detik |
| 7 | Status WiFi di LCD | OK / NF |
| 8 | Blynk IoT Cloud Dashboard | Monitoring global |

---

## 🧪 11. Pengujian & Validasi

| Parameter | Nilai Uji | Status |
| --- | --- | --- |
| Tegangan Inverter | 220.1 V | ✅ Normal |
| Arus | 0.52 A | ✅ OK |
| Daya | 115 W | ✅ Sesuai beban |
| Energi | 0.028 kWh | ✅ Akumulasi |
| Suhu Ruang | 30.2 °C | ✅ OK |
| Suhu Baterai | 28.6 °C | ✅ Aman |
| WiFi RSSI | -65 dBm | ✅ Koneksi stabil |
| Watchdog | Auto restart setelah 60 detik hang | ✅ Berhasil |

---

## 📲 12. Rekomendasi Dashboard Blynk

| Widget | Label | Virtual Pin | Warna |
| --- | --- | --- | --- |
| Gauge | Tegangan (V) | V3 | Hijau |
| Gauge | Arus (A) | V4 | Kuning |
| Gauge | Daya (W) | V5 | Oranye |
| Label | Energi (kWh) | V6 | Putih |
| Gauge | Suhu Ruang | V1 | Merah muda |
| Gauge | Kelembapan | V2 | Biru |
| Gauge | Suhu Baterai | V7 | Kuning |
| Label / Gauge | WiFi Signal | V8 | Biru tua |
| Switch | Relay 1–4 | V10–V13 | Sesuai fungsi |
| Label | Online Time | V0 | Abu-abu |

---

## 🧾 13. Kesimpulan / Conclusion

Versi 1.5.0 berhasil mengintegrasikan **sistem PLTS offgrid** dengan:

✅ Monitoring lengkap (tegangan, arus, daya, energi, suhu, kelembapan)

✅ Kontrol beban real-time

✅ Indikator kekuatan sinyal WiFi (RSSI)

✅ Watchdog otomatis untuk kestabilan

✅ Tampilan informatif di LCD 20×4

> ⚡ Hasil: Sistem stabil 24/7, hemat bandwidth, dan siap diimplementasikan di lingkungan offgrid nyata.
> 

---

**Mas Willy – Desember 2025**

*"Reliable IoT Monitoring for Sustainable Solar Power"*
