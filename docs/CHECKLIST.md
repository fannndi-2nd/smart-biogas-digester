# Checklist Smart Biogas Digester

Gunakan checklist ini sebelum presentasi, push ke GitHub, atau publish blog.

## 1. Identitas

- [x] Nama dan NIM anggota sudah diisi di README, dashboard, dan blog.
- [ ] Nama kampus / program studi: `[isi]`
- [ ] Nama dosen pengampu: `[isi]`
- [ ] Username GitHub dan URL repo: `[isi]`
- [ ] Tanggal presentasi / publish blog: `[isi]`

## 2. Foto dan tangkapan layar

- [ ] Foto prototipe tampak depan (resolusi minimal 1200 px).
- [ ] Foto prototipe tampak samping / detail sensor.
- [ ] Foto LCD menyala.
- [ ] Screenshot aplikasi Blynk asli (bukan mockup poster).
- [ ] Screenshot dashboard dengan data live (token disembunyikan).
- [ ] Ganti `assets/poster.jpg` bila poster direvisi.
- [ ] Ganti `assets/dashboard.png` bila dashboard berubah.

## 3. Pengujian

- [ ] Catat waktu warm-up MQ-4.
- [ ] Catat CH₄ di udara bersih.
- [ ] Catat CH₄ dekat sumber gas.
- [ ] Bandingkan kelembapan dengan higrometer.
- [ ] Bandingkan suhu dengan termometer.
- [ ] Ukur latensi Blynk.
- [ ] Pastikan log Sheets masuk setiap 60 detik.
- [ ] Uji LCD 3 halaman.
- [ ] Uji uptime minimal 1 jam.
- [ ] Isi tabel **Hasil Pengujian** di README.
- [ ] Ganti placeholder `—` dengan angka asli.

## 4. Blynk

- [ ] Template **Smart Biogas Digester** dibuat.
- [ ] Datastream `V0` String, `V1`–`V4` Double.
- [ ] Auth token baru sudah dirotasi (token lama pernah tertulis di `.ino`).
- [ ] Token hanya ada di `config.h` dan pengaturan dashboard.
- [ ] Aplikasi Blynk menampilkan V0–V4.
- [ ] Perangkat muncul **Online** di Blynk.

## 5. Google Sheets

- [ ] Spreadsheet baru dibuat.
- [ ] `apps-script/Code.gs` ditempel dan `setup()` dijalankan.
- [ ] Web App di-deploy ulang (`/exec`, akses **Anyone**).
- [ ] URL `/exec` hanya ada di `config.h` dan pengaturan dashboard.
- [ ] `testPost()` dan `testGet()` berhasil.
- [ ] Dashboard bisa membaca riwayat.

## 6. Firmware

- [ ] `config.example.h` disalin menjadi `config.h`.
- [ ] WiFi 2,4 GHz, token Blynk, dan URL Sheets diisi.
- [ ] Board **ESP32 Dev Module** dipilih.
- [ ] Library Blynk, DHT, LiquidCrystal_I2C terpasang.
- [ ] Upload berhasil.
- [ ] Serial Monitor menampilkan pembacaan sensor.
- [ ] LCD menampilkan 3 halaman.
- [ ] `config.h` **tidak** ikut ter-commit.

## 7. Dashboard

- [ ] Dibuka lewat `python -m http.server` atau GitHub Pages.
- [ ] Mode demo berjalan tanpa token.
- [ ] Mode live menampilkan data Blynk.
- [ ] Grafik, tabel, dan CSV berfungsi.
- [ ] Responsif di HP (390 px) dan laptop (1440 px).
- [ ] Token tidak muncul di screenshot.
- [ ] Semua tautan dokumen bisa dibuka.

## 8. Keamanan sebelum push

Jalankan:

```bash
git status
git grep -n -i -E "auth_token|password|script.google.com|TMPL"
```

- [ ] Tidak ada token/password/URL asli di file yang di-commit.
- [ ] `config.h` ada di `.gitignore`.
- [ ] Password WiFi diganti dari password lama.
- [ ] Token Blynk lama sudah dirotasi.
- [ ] Repository tidak berisi file `.env` atau backup `.ino` lama.
- [ ] Screenshot tidak memperlihatkan token.

## 9. Publikasi

- [ ] `git init`, `git add .`, `git commit -m "Initial commit: Smart Biogas Digester IoT"`.
- [ ] Repo baru dibuat di GitHub dan di-push.
- [ ] GitHub Pages aktif (root).
- [ ] Deskripsi repo diisi.
- [ ] Topik repo: `iot`, `esp32`, `biogas`, `blynk`, `mq-4`, `dht22`.
- [ ] Blog Gamasti: ganti `[Tanggal]` dan `[Link GitHub]`.
- [ ] Blog Gamasti: unggah poster dan screenshot dashboard.
- [ ] Tautan GitHub di blog sudah benar.

## 10. Presentasi

- [ ] PDF panduan dibuka dan tidak ada halaman terpotong.
- [ ] PPTX dibuka dan semua slide terbaca.
- [ ] Speaker notes dicek.
- [ ] Demo dashboard disiapkan (mode demo sebagai cadangan).
- [ ] Perangkat dan kabel cadangan dibawa.
- [ ] Power bank terisi penuh.
- [ ] Video cadangan 30–60 detik bila WiFi bermasalah.
