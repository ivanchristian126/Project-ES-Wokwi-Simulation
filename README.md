# FridgeGuard: Simulasi ESP32 di VS Code (Wokwi + PlatformIO)

FridgeGuard adalah alat pemantau kulkas: mengukur **suhu & kelembapan** (sensor DHT22) dan mendeteksi **pintu terbuka/tertutup** (magnetic switch). Kalau pintu terbuka terlalu lama, LED berkedip sebagai peringatan. Datanya dikirim lewat internet dan bisa dilihat di dashboard web:

**Dashboard:** https://ivanchristian126.github.io/fridge-guard/

Project ini berisi kode ESP32-nya. Kamu **tidak perlu punya alatnya**: semuanya bisa dijalankan sebagai simulasi di laptop pakai VS Code.

![Rangkaian FridgeGuard di simulasi Wokwi](rangkaian.png)

---

## Daftar isi

1. [Yang dibutuhkan](#1-yang-dibutuhkan)
2. [Download project ini](#2-download-project-ini)
3. [Install VS Code](#3-install-vs-code)
4. [Install extension PlatformIO](#4-install-extension-platformio)
5. [Install extension Wokwi + aktifkan lisensi](#5-install-extension-wokwi--aktifkan-lisensi)
6. [Buka project di VS Code](#6-buka-project-di-vs-code)
7. [Build (compile) kode](#7-build-compile-kode)
8. [Jalankan simulasi](#8-jalankan-simulasi)
9. [Coba-coba di simulasi](#9-coba-coba-di-simulasi)
10. [Kalau mau mengubah kode](#10-kalau-mau-mengubah-kode)
11. [Upload ke alat asli (opsional)](#11-upload-ke-alat-asli-opsional)
12. [Kalau ada masalah](#12-kalau-ada-masalah)
13. [Penjelasan teknis](#13-penjelasan-teknis)

Total waktu pertama kali: sekitar **20–30 menit** (kebanyakan menunggu download). Setelah itu, menjalankan simulasi cuma butuh 1 menit.

---

## 1. Yang dibutuhkan

- Laptop/PC Windows, macOS, atau Linux.
- Koneksi internet (untuk download dan untuk simulasi, karena simulasinya kirim data ke internet).
- Akun Wokwi (gratis, bisa daftar saat langkah 5; bisa login pakai akun Google atau GitHub).

Tidak perlu install Arduino IDE, dan tidak perlu install library apa pun secara manual.

---

## 2. Download project ini

Pilih salah satu:

**Cara A: Download ZIP (paling gampang)**

1. Buka https://github.com/ivanchristian126/Project-ES-Wokwi-Simulation
2. Klik tombol hijau **Code**, lalu **Download ZIP**.
3. Buka file ZIP-nya, lalu **Extract All** ke folder yang gampang dicari, misalnya `Documents`.
   Jangan membuka project langsung dari dalam ZIP, harus di-extract dulu.

**Cara B: Pakai Git** (kalau sudah terbiasa)

```
git clone https://github.com/ivanchristian126/Project-ES-Wokwi-Simulation.git
```

Setelah itu, pastikan kamu tahu letak folder yang berisi file `platformio.ini`. Folder itulah yang nanti dibuka di VS Code.

> Kalau pakai Download ZIP, kadang hasilnya jadi folder di dalam folder (misalnya `Project-ES-Wokwi-Simulation-main\Project-ES-Wokwi-Simulation-main`). Pakai folder yang **langsung** berisi `platformio.ini`.

---

## 3. Install VS Code

1. Buka https://code.visualstudio.com, klik **Download**.
2. Jalankan installer-nya. Pilihan bawaan sudah cukup, klik **Next** terus sampai selesai.
3. Buka VS Code.

---

## 4. Install extension PlatformIO

PlatformIO adalah alat untuk compile kode ESP32 di dalam VS Code.

1. Di VS Code, klik ikon **Extensions** di sidebar kiri (ikon 4 kotak), atau tekan `Ctrl+Shift+X` (Mac: `Cmd+Shift+X`).
2. Di kotak pencarian ketik **PlatformIO IDE**.
3. Klik **Install** pada hasil yang bernama **PlatformIO IDE**.
4. **Tunggu sampai selesai.** Di pojok kanan bawah akan ada proses instalasi PlatformIO Core. Ini bisa 5–10 menit, jangan tutup VS Code.
5. Kalau muncul tombol **Reload Now** atau diminta restart, klik / tutup lalu buka lagi VS Code.

Tandanya sudah berhasil: di sidebar kiri muncul ikon PlatformIO (ikon kepala alien), dan di status bar paling bawah muncul tombol-tombol kecil seperti ✓ dan →.

---

## 5. Install extension Wokwi + aktifkan lisensi

Wokwi adalah simulator: ESP32, sensor, dan LED-nya jadi virtual.

**Install extension:**

1. Buka lagi **Extensions** (`Ctrl+Shift+X`).
2. Cari **Wokwi Simulator**, klik **Install**.

**Aktifkan lisensi (wajib, gratis, setiap orang punya sendiri):**

1. Tekan `F1` (di beberapa laptop: `Fn+F1`). Akan muncul kotak perintah di atas.
2. Ketik **Wokwi: Request a New License**, lalu tekan Enter.
3. VS Code bertanya mau membuka website Wokwi. Klik **Open**.
4. Di browser, klik **GET YOUR LICENSE**. Login atau daftar akun Wokwi (gratis).
5. Browser minta izin mengirim lisensi ke VS Code. Klik izinkan/**Open**. Mungkin perlu konfirmasi sekali lagi di VS Code.
6. Selesai kalau di VS Code muncul pesan **"License activated for (nama kamu)"**.

Kalau suatu saat Wokwi minta lisensi lagi, ulangi langkah ini.

---

## 6. Buka project di VS Code

1. Di VS Code klik menu **File > Open Folder...**
2. Pilih folder project yang berisi `platformio.ini` (lihat langkah 2), lalu klik **Select Folder**.
3. Kalau muncul pertanyaan *"Do you trust the authors of the files in this folder?"*, klik **Yes, I trust the authors**.
4. **Tunggu.** Pertama kali dibuka, PlatformIO otomatis download platform ESP32 dan library yang dibutuhkan. Prosesnya kelihatan di pojok kanan bawah dan bisa beberapa menit.

Isi folder yang akan kamu lihat di sidebar kiri:

| File | Fungsi |
|---|---|
| `src/main.cpp` | Kode utama ESP32 |
| `platformio.ini` | Pengaturan board dan daftar library (didownload otomatis) |
| `diagram.json` | Rangkaian untuk simulasi Wokwi |
| `wokwi.toml` | Memberi tahu Wokwi letak hasil compile |
| `rangkaian.png` | Gambar rangkaian (yang ada di README ini) |

---

## 7. Build (compile) kode

Wokwi tidak bisa compile kode sendiri, jadi kode harus di-build dulu.

1. Lihat status bar paling bawah VS Code. Klik tombol **✓** (tulisannya *PlatformIO: Build* kalau mouse diarahkan ke sana).
2. Terminal akan terbuka di bawah dan menampilkan proses compile. Build pertama lebih lama (1–3 menit).
3. Berhasil kalau di akhir terminal muncul tulisan **`SUCCESS`** berwarna hijau.

Tombol PlatformIO di status bar bawah:

| Tombol | Fungsi |
|---|---|
| ✓ | Build (compile kode) |
| → | Upload ke ESP32 asli (sekaligus build) |
| ikon colokan | Serial Monitor (melihat pesan dari ESP32 asli) |

---

## 8. Jalankan simulasi

1. Tekan `F1`.
2. Ketik **Wokwi: Start Simulator**, tekan Enter.
3. Akan terbuka tab baru berisi rangkaian (seperti gambar di atas) dan simulasinya langsung berjalan.
4. Pesan dari ESP32 muncul di terminal bagian bawah. Dalam beberapa detik harus muncul kurang lebih seperti ini:

```
=== FridgeGuard: DHT22 + Magnetic Switch + 2 LED + MQTT ===
Kalau LED kuning & biru kedip 3x -> LED OK.
[PINTU] awal: TERTUTUP
[WIFI] Menghubungkan ke Wokwi-GUEST ...
[DHT22] Suhu: 24.0 C | Kelembapan: 40.0 %
[WIFI] Terhubung, IP: 10.10.0.2
[MQTT] Konek ke broker... OK
[MQTT] Kirim: {"suhu":24.0,"lembap":40.0,"pintu":"TERTUTUP",...}
```

Kalau sudah muncul `[MQTT] Kirim: ...`, data simulasi sudah terkirim. Buka dashboard https://ivanchristian126.github.io/fridge-guard/ dan datanya akan ikut berubah.

Untuk menghentikan simulasi, tutup tab Wokwi-nya.

---

## 9. Coba-coba di simulasi

| Yang dilakukan | Yang terjadi |
|---|---|
| Geser **slide switch** ke **kanan** | Pintu dianggap **terbuka**: LED kuning menyala, dashboard menampilkan "Terbuka" |
| Biarkan terbuka lebih dari **5 detik** | Peringatan: LED kuning berkedip, dashboard menampilkan banner peringatan |
| Geser slide switch ke **kiri** | Pintu **tertutup**: LED mati |
| Klik sensor **DHT22** saat simulasi berjalan | Muncul slider untuk mengubah suhu dan kelembapan |

Slide switch di simulasi menggantikan magnetic switch pada alat asli.

Catatan: LED biru bawaan board tidak kelihatan di simulasi (board ESP32 di Wokwi tidak punya LED itu). Di alat asli LED biru tetap menyala.

---

## 10. Kalau mau mengubah kode

1. Edit file `src/main.cpp`, lalu simpan (`Ctrl+S`).
2. Klik **✓ (Build)** lagi.
3. Kalau simulasi masih terbuka, Wokwi otomatis memakai kode yang baru.

Contoh yang gampang diubah: `BATAS_PINTU_TERBUKA = 5000` (lama pintu boleh terbuka, dalam milidetik).

---

## 11. Upload ke alat asli (opsional)

Lewati bagian ini kalau hanya mau simulasi.

1. Di `src/main.cpp`, ubah `#define MODE_WOKWI 1` menjadi `#define MODE_WOKWI 0`.
2. Di bawah baris `#else`, isi nama WiFi dan password:
   ```cpp
   const char* WIFI_SSID = "nama_wifi_kamu";
   const char* WIFI_PASS = "password_wifi_kamu";
   ```
   ESP32 hanya bisa WiFi **2.4 GHz**. Paling aman pakai hotspot HP.
3. Colok ESP32 ke laptop pakai kabel USB (pastikan kabel data, bukan kabel charge saja).
4. Klik **→ (Upload)** di status bar bawah. Tunggu sampai `SUCCESS`.
5. Klik **ikon colokan (Serial Monitor)**. Tunggu tulisan `[MQTT] Konek ke broker... OK`.

Rangkaian alat asli:

| Komponen | Sambungan ke ESP32 |
|---|---|
| DHT22 (modul `+ out -`) | `+` ke 3V3, `-` ke GND, `out` ke D4 |
| Magnetic switch | kabel 1 ke D18, kabel 2 ke GND |
| LED kuning | D19 ke resistor 220 ohm ke kaki panjang LED; kaki pendek ke GND |
| LED biru | bawaan board (D2), tidak perlu dirangkai |

> **Sebelum push ke GitHub:** kosongkan lagi `WIFI_SSID` dan `WIFI_PASS`, dan kembalikan `MODE_WOKWI` ke `1`. Isi repo GitHub bisa dibaca orang lain, jadi password WiFi jangan sampai ikut ter-upload.

---

## 12. Kalau ada masalah

| Masalah | Solusi |
|---|---|
| Instalasi PlatformIO gagal / ada pesan error soal **Python** | Install Python dari https://www.python.org/downloads/ dan **centang "Add python.exe to PATH"** saat instalasi. Lalu tutup dan buka lagi VS Code. |
| Tombol ✓ dan → tidak muncul di status bar | Pastikan folder yang dibuka adalah folder yang **langsung** berisi `platformio.ini` (langkah 6). Tunggu juga sampai proses download PlatformIO selesai. |
| Build gagal karena download library/platform | Cek koneksi internet, lalu klik ✓ lagi. |
| Wokwi bilang firmware tidak ditemukan | Klik ✓ (Build) dulu sampai `SUCCESS`, baru **Wokwi: Start Simulator**. |
| Wokwi minta lisensi | Ulangi langkah 5 (aktifkan lisensi). |
| Simulasi jalan tapi berhenti di `[WIFI] Menghubungkan...` atau MQTT gagal | Cek internet. Pastikan `MODE_WOKWI` bernilai `1`. |
| Upload ke alat asli gagal / port tidak terdeteksi | Tutup Arduino IDE atau Serial Monitor lain yang masih terhubung ke ESP32. Pastikan driver USB **CH340** sudah terpasang dan kabel USB-nya kabel data. |
| Alat asli tidak konek WiFi | Pastikan `MODE_WOKWI` bernilai `0`, SSID/password benar, dan WiFi-nya 2.4 GHz. |
| Data di dashboard campur aduk | Jangan menyalakan simulasi dan alat asli bersamaan, karena keduanya mengirim ke tempat yang sama. |

---

## 13. Penjelasan teknis

Untuk yang ingin tahu cara kerjanya.

**Perilaku LED**

| Kondisi pintu | LED |
|---|---|
| Tertutup | Semua mati |
| Terbuka sampai 5 detik | LED kuning menyala |
| Terbuka lebih dari 5 detik | LED kuning + biru berkedip (peringatan) |

**Pengiriman data (MQTT)**

ESP32 mengirim data ke broker MQTT publik HiveMQ (`broker.hivemq.com`), lalu dashboard web membacanya dari broker yang sama. Tidak ada database atau server sendiri.

| Topik | Isi | Disimpan broker (retained) |
|---|---|---|
| `fridgeguard/kel-7/data` | JSON tiap 1 detik: `suhu`, `lembap`, `pintu`, `alarm`, `buka_detik`, `uptime`, `ts` (jam kirim dari internet/NTP) | Ya |
| `fridgeguard/kel-7/pintu` | `TERBUKA` / `TERTUTUP` saat berubah | Ya |
| `fridgeguard/kel-7/alarm` | `ON` / `OFF` saat berubah | Ya |
| `fridgeguard/kel-7/status` | `online` / `offline` (offline otomatis kalau ESP32 terputus) | Ya |

Karena semua topik disimpan broker (retained), dashboard yang baru dibuka langsung menampilkan data terakhir beserta jamnya, walaupun ESP32 sedang mati.

**Library** (diatur di `platformio.ini`, didownload otomatis): DHT sensor library, Adafruit Unified Sensor, PubSubClient.

**Satu kode untuk dua mode:** `#define MODE_WOKWI 1` memakai WiFi virtual Wokwi (`Wokwi-GUEST`); `0` memakai WiFi asli. Sisa kodenya sama persis.
