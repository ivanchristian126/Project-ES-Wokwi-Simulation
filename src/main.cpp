// =====================================================
// FridgeGuard - versi MQTT (kirim data ke dashboard)
// Sensor  : DHT22 (suhu & kelembapan) + magnetic switch (pintu)
// Aktuator: LED kuning + LED biru bawaan ESP32 (D2) + buzzer aktif (D23)
// Library : "DHT sensor library" + "Adafruit Unified Sensor" (Adafruit)
//           + PubSubClient (PlatformIO: otomatis dari lib_deps di platformio.ini)
// Board   : ESP32 Dev Module | Serial Monitor: 115200
// =====================================================
//
// SATU KODE UNTUK ALAT ASLI & WOKWI -> cukup ganti MODE_WOKWI di bawah:
//   1 = simulasi Wokwi (WiFi virtual "Wokwi-GUEST")
//   0 = alat asli (WiFi rumah)
//
// RANGKAIAN (sama di alat asli & Wokwi):
// DHT22 (modul "+ out -"): + -> 3V3 | - -> GND | out -> D4 (GPIO4)
// Magnetic switch        : kabel 1 -> D18 | kabel 2 -> GND
//                          (Wokwi: slide switch, pin tengah -> GPIO18, pin kiri -> GND
//                           geser kiri = TERTUTUP, geser kanan = TERBUKA)
// LED kuning             : D19 -> resistor 220 ohm -> kaki panjang | kaki pendek -> GND
// LED biru               : bawaan board (D2)
//                          (board-esp32-devkit-c-v4 di Wokwi tidak punya LED ini,
//                           jadi di simulasi tidak kelihatan; di alat asli tetap nyala)
// Buzzer (modul 3 pin)   : VCC -> 3V3 | GND -> GND | I/O -> D23 (GPIO23)
//                          (modul "Low Level Trigger" = aktif LOW; pakai 3V3, bukan 5V,
//                           karena pin ESP32 cuma 3.3V jadi tidak bisa mematikan buzzer di 5V)
//                          (Wokwi: wokwi-buzzer, pin 1 -> GND, pin 2 -> GPIO23;
//                           dibunyikan pakai tone() karena buzzer Wokwi butuh nada)
//
// MQTT (broker publik HiveMQ):
// <TOPIK>/data   -> JSON tiap 1 detik (retained: broker menyimpan data terakhir,
//                   ada "ts" = waktu kirim dari jam internet/NTP)
// <TOPIK>/pintu  -> "TERBUKA" / "TERTUTUP" saat berubah (retained)
// <TOPIK>/alarm  -> "ON" / "OFF" saat berubah (retained)
// <TOPIK>/status -> "online" / "offline" (retained, offline otomatis lewat LWT)
// =====================================================

#define MODE_WOKWI 1   // 1 = simulasi Wokwi | 0 = alat asli

#include <Arduino.h>   // wajib di PlatformIO (.cpp), di Arduino IDE otomatis
#include <WiFi.h>
#include <PubSubClient.h>   // dari lib_deps di platformio.ini
#include <DHT.h>
#include <time.h>

// ---- WiFi (ESP32 hanya bisa WiFi 2.4 GHz) ----
#if MODE_WOKWI
const char* WIFI_SSID = "Wokwi-GUEST";   // WiFi virtual bawaan Wokwi, tanpa password
const char* WIFI_PASS = "";
#else
const char* WIFI_SSID = "";
const char* WIFI_PASS = "";
#endif

// ---- MQTT ----
const char* MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;
#define TOPIK "fridgeguard/kel-7"   // harus sama persis dengan di dashboard

// ---- Pin ----
#define DHT_PIN   4
#define DHT_TYPE  DHT22
#define REED_PIN  18
#define LED_PIN   19
#define LED_BIRU  2    // LED biru bawaan ESP32
#define BUZZER_PIN 23  // I/O modul buzzer

// Lama pintu boleh terbuka sebelum peringatan (ms)
const unsigned long BATAS_PINTU_TERBUKA = 5000;

// Kecepatan kedip LED saat peringatan (ms)
const unsigned long INTERVAL_KEDIP = 250;

// Interval kirim data ke dashboard (ms)
const unsigned long INTERVAL_KIRIM = 1000;

// Interval baca DHT22 (ms) - DHT22 tidak bisa lebih cepat dari 2 detik,
// jadi di antara dua bacaan yang dikirim adalah nilai terakhir
const unsigned long INTERVAL_DHT = 2000;

// Magnet didekatkan = pintu TERTUTUP -> false
// (ganti true hanya kalau terbaca kebalik)
const bool REED_TERBALIK = false;

// Modul buzzer "Low Level Trigger" = aktif LOW (I/O = LOW -> bunyi).
// Kalau pakai modul aktif HIGH (buzzer bunyi terus saat pintu tertutup /
// diam saat alarm), ganti jadi false. Hanya berlaku di alat asli.
const bool BUZZER_AKTIF_LOW = true;

// Nada buzzer di simulasi Wokwi (Hz)
const unsigned int FREKUENSI_BUZZER_WOKWI = 2000;

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

char clientId[32];
unsigned long waktuBacaDHT = 0;
unsigned long waktuKirim = 0;
float suhuTerakhir = NAN;
float lembapTerakhir = NAN;
unsigned long waktuPintuDibuka = 0;
unsigned long waktuCobaMqtt = 0;
bool pintuTerbukaSebelumnya = false;
bool alarmSebelumnya = false;
bool wifiSebelumnya = false;

void setBuzzer(bool nyala) {
#if MODE_WOKWI
  // Buzzer Wokwi itu piezo pasif: harus diberi nada (tone) supaya bunyi.
  // tone() hanya dipanggil saat status berubah, bukan tiap loop.
  static bool buzzerNyala = false;
  if (nyala == buzzerNyala) return;
  buzzerNyala = nyala;
  if (nyala) tone(BUZZER_PIN, FREKUENSI_BUZZER_WOKWI);
  else noTone(BUZZER_PIN);
#else
  // Buzzer aktif di alat asli: cukup dinyalakan / dimatikan
  digitalWrite(BUZZER_PIN, (nyala != BUZZER_AKTIF_LOW) ? HIGH : LOW);
#endif
}

bool bacaPintuTerbuka() {
  // INPUT_PULLUP: magnet dekat -> switch nyambung -> LOW -> pintu tertutup
  bool terbuka = (digitalRead(REED_PIN) == HIGH);
  return REED_TERBALIK ? !terbuka : terbuka;
}

// Kirim ke MQTT kalau sedang terhubung (kalau tidak, dilewati saja)
void kirim(const char* topik, const char* isi, bool retained) {
  if (mqtt.connected()) mqtt.publish(topik, isi, retained);
}

// Coba konek ke broker tiap 5 detik, tanpa menghentikan logika LED
void cekKoneksi() {
  bool wifiOk = (WiFi.status() == WL_CONNECTED);
  if (wifiOk != wifiSebelumnya) {
    wifiSebelumnya = wifiOk;
    if (wifiOk) {
      Serial.print("[WIFI] Terhubung, IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("[WIFI] Terputus, mencoba ulang...");
    }
  }
  if (!wifiOk || mqtt.connected()) return;
  if (millis() - waktuCobaMqtt < 5000) return;
  waktuCobaMqtt = millis();

  Serial.print("[MQTT] Konek ke broker... ");
  // LWT: kalau ESP32 putus tiba-tiba, broker kirim "offline" ke TOPIK/status
  if (mqtt.connect(clientId, TOPIK "/status", 1, true, "offline")) {
    Serial.println("OK");
    kirim(TOPIK "/status", "online", true);
    kirim(TOPIK "/pintu", pintuTerbukaSebelumnya ? "TERBUKA" : "TERTUTUP", true);
    kirim(TOPIK "/alarm", alarmSebelumnya ? "ON" : "OFF", true);
  } else {
    Serial.printf("gagal (kode %d), coba lagi 5 detik\n", mqtt.state());
  }
}

void setup() {
  // Buzzer dimatikan paling awal supaya tidak bunyi saat boot
  setBuzzer(false);
  pinMode(BUZZER_PIN, OUTPUT);
  setBuzzer(false);

  Serial.begin(115200);
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BIRU, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BIRU, LOW);
  dht.begin();

  delay(500);
  Serial.println();
  Serial.println("=== FridgeGuard: DHT22 + Magnetic Switch + 2 LED + Buzzer + MQTT ===");

  // Tes LED & buzzer: kuning & biru kedip 3x + bip 3x saat baru nyala
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); digitalWrite(LED_BIRU, HIGH); setBuzzer(true);  delay(200);
    digitalWrite(LED_PIN, LOW);  digitalWrite(LED_BIRU, LOW);  setBuzzer(false); delay(200);
  }
  Serial.println("Kalau LED kuning & biru kedip 3x + buzzer bip 3x -> LED & buzzer OK.");

  pintuTerbukaSebelumnya = bacaPintuTerbuka();
  if (pintuTerbukaSebelumnya) waktuPintuDibuka = millis();
  Serial.println(pintuTerbukaSebelumnya ? "[PINTU] awal: TERBUKA" : "[PINTU] awal: TERTUTUP");

  // ID unik dari MAC address, supaya tidak bentrok dengan alat lain di broker publik
  uint64_t mac = ESP.getEfuseMac();
  snprintf(clientId, sizeof(clientId), "fridgeguard-%08lX", (unsigned long)(mac & 0xFFFFFFFF));

  WiFi.mode(WIFI_STA);
#if MODE_WOKWI
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);   // Wokwi-GUEST ada di channel 6 -> konek lebih cepat
#else
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif
  Serial.printf("[WIFI] Menghubungkan ke %s ...\n", WIFI_SSID);

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(512);

  // Jam internet (NTP), supaya data yang disimpan broker punya cap waktu
  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");
}

void loop() {
  // ---------- Koneksi WiFi & MQTT ----------
  cekKoneksi();
  mqtt.loop();

  // ---------- Magnetic switch (dengan debounce) ----------
  bool pintuTerbuka = bacaPintuTerbuka();
  if (pintuTerbuka != pintuTerbukaSebelumnya) {
    delay(50);  // debounce
    pintuTerbuka = bacaPintuTerbuka();
    if (pintuTerbuka != pintuTerbukaSebelumnya) {
      pintuTerbukaSebelumnya = pintuTerbuka;
      if (pintuTerbuka) waktuPintuDibuka = millis();
      Serial.println(pintuTerbuka ? "[PINTU] TERBUKA" : "[PINTU] TERTUTUP");
      kirim(TOPIK "/pintu", pintuTerbuka ? "TERBUKA" : "TERTUTUP", true);
    }
  }

  // ---------- Peringatan ----------
  bool alarm = pintuTerbuka && (millis() - waktuPintuDibuka > BATAS_PINTU_TERBUKA);
  if (alarm != alarmSebelumnya) {
    alarmSebelumnya = alarm;
    if (alarm) {
      Serial.printf("[ALARM] Pintu terbuka > %lu detik -> LED kuning + biru kedip + buzzer bunyi\n", BATAS_PINTU_TERBUKA / 1000);
    } else {
      Serial.println("[ALARM] Selesai");
    }
    kirim(TOPIK "/alarm", alarm ? "ON" : "OFF", true);
  }

  // ---------- LED & buzzer ----------
  bool fasaKedip = (millis() / INTERVAL_KEDIP) % 2 == 0;
  if (!pintuTerbuka) {
    // Tertutup: semua mati
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_BIRU, LOW);
    setBuzzer(false);
  } else if (!alarm) {
    // Baru dibuka: kuning nyala terus, biru & buzzer mati
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_BIRU, LOW);
    setBuzzer(false);
  } else {
    // Peringatan: kuning, biru & buzzer kedip/bip bareng
    digitalWrite(LED_PIN, fasaKedip ? HIGH : LOW);
    digitalWrite(LED_BIRU, fasaKedip ? HIGH : LOW);
    setBuzzer(fasaKedip);
  }

  // ---------- DHT22 tiap 2 detik ----------
  if (millis() - waktuBacaDHT >= INTERVAL_DHT) {
    waktuBacaDHT = millis();
    suhuTerakhir = dht.readTemperature();
    lembapTerakhir = dht.readHumidity();
    if (isnan(suhuTerakhir) || isnan(lembapTerakhir)) {
      Serial.println("[DHT22] Gagal baca! Cek kabel DATA/VCC/GND.");
    } else {
      Serial.printf("[DHT22] Suhu: %.1f C | Kelembapan: %.1f %%\n", suhuTerakhir, lembapTerakhir);
    }
  }

  // ---------- Kirim data ke dashboard tiap 1 detik ----------
  if (millis() - waktuKirim >= INTERVAL_KIRIM) {
    waktuKirim = millis();

    char suhuStr[12], lembapStr[12];
    if (isnan(suhuTerakhir) || isnan(lembapTerakhir)) {
      strcpy(suhuStr, "null");
      strcpy(lembapStr, "null");
    } else {
      snprintf(suhuStr, sizeof(suhuStr), "%.1f", suhuTerakhir);
      snprintf(lembapStr, sizeof(lembapStr), "%.1f", lembapTerakhir);
    }

    unsigned long bukaDetik = pintuTerbuka ? (millis() - waktuPintuDibuka) / 1000 : 0;
    time_t ts = time(nullptr);
    if (ts < 1700000000) ts = 0;   // jam belum sinkron dengan internet -> 0
    char data[200];
    snprintf(data, sizeof(data),
             "{\"suhu\":%s,\"lembap\":%s,\"pintu\":\"%s\",\"alarm\":%s,\"buka_detik\":%lu,\"uptime\":%lu,\"ts\":%lu}",
             suhuStr, lembapStr,
             pintuTerbuka ? "TERBUKA" : "TERTUTUP",
             alarm ? "true" : "false",
             bukaDetik, millis() / 1000, (unsigned long)ts);
    kirim(TOPIK "/data", data, true);   // retained: dashboard langsung dapat data terakhir
    if (mqtt.connected()) {
      Serial.print("[MQTT] Kirim: ");
      Serial.println(data);
    }
  }

#if MODE_WOKWI
  delay(10);   // bikin simulasi Wokwi lebih ringan & cepat (alat asli tidak perlu)
#endif
}
