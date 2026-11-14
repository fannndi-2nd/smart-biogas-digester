<!--
  ARTIKEL BLOG — SMART BIOGAS DIGESTER
  ------------------------------------------------------------------
  Cara pakai:
  1. Ganti [Tanggal publikasi] dan [Link GitHub] sebelum publish.
  2. Di editor blog Gamasti, tempel bagian "Judul" sampai "Penutup".
  3. Unggah 2 gambar yang disarankan di akhir artikel.
  4. Nama penulis mengikuti akun yang mempublikasikan; cantumkan
     nama tim di bagian akhir.
  ------------------------------------------------------------------
-->

# Smart Biogas Digester: Membaca Gas dari Limbah Organik lewat IoT

**Penulis:** Kelompok 6 — Smart Biogas Project
**Tanggal:** [Tanggal publikasi]
**Estimasi baca:** 2 menit

**Ringkasan:** Digester biogas sederhana yang membaca kadar metana, suhu, dan kelembapan, lalu mengirim datanya ke Blynk dan Google Sheets secara otomatis.

---

Sampah organik rumah tangga sering berakhir di tempat pembuangan. Padahal, sisa makanan dan sayuran bisa difermentasi menjadi biogas — sumber energi yang bisa dipakai untuk memasak. Masalahnya, proses di dalam digester tidak terlihat. Kita tidak tahu apakah gasnya sudah terbentuk, apakah suhunya terlalu rendah, atau apakah ada kebocoran.

Untuk tugas mata kuliah Internet of Things, kami membuat **Smart Biogas Digester**: jerigen transparan 5 liter sebagai digester, ditambah lapisan sensor dan konektivitas supaya prosesnya bisa dipantau dari jauh.

Otak sistem ini adalah **ESP32**. Ia membaca dua sensor. **MQ-4** memberi estimasi kadar gas metana (CH₄) di udara, sedangkan **DHT22** membaca suhu dan kelembapan di sekitar digester. Data ditampilkan di **LCD 16×2 I2C** dengan tiga halaman bergantian: CH₄ dan status, suhu dan kelembapan, lalu WiFi dan alamat IP.

Setiap dua detik, ESP32 mengirim data ke **Blynk IoT**. Setiap 60 detik, satu baris log dikirim ke **Google Sheets** lewat Apps Script. Dari sana, sebuah **dashboard web** membaca data live dari Blynk dan riwayat dari Sheets. Dashboard menampilkan kartu status, grafik tren, riwayat pengukuran, dan tombol unduh CSV.

Ada satu detail rangkaian yang sempat membuat kami mengulang: modul MQ-4 diberi 5 volt, sedangkan ADC ESP32 hanya tahan 3,3 volt. Output analognya harus melewati pembagi tegangan 1 kΩ dan 2 kΩ sebelum masuk ke GPIO34. Setelah itu, firmware menghitung estimasi ppm dari kurva datasheet, dengan konstanta `R0` yang dikalibrasi di udara bersih setelah sensor di-burn-in 24–48 jam.

Kami juga menetapkan dua ambang: **1.000 ppm** sebagai peringatan dini dan **5.000 ppm** sebagai bahaya, atau sekitar 10% LEL metana. Status di dashboard berubah dari AMAN, WASPADA, ke BAHAYA. Perlu ditekankan: MQ-4 bukan sensor keselamatan dan tidak selektif. Ia juga bereaksi terhadap asap, LPG, dan alkohol. Jadi angka ppm di dashboard adalah estimasi, bukan hasil laboratorium.

Dashboard-nya sendiri bisa dipakai tanpa koneksi lewat **mode demo**, cocok untuk pameran saat WiFi kampus sedang ramai. Kalau token Blynk diisi, dashboard langsung menampilkan data live. Token dan URL Sheets hanya disimpan di browser, tidak pernah masuk ke source code.

Yang paling kami pelajari dari project ini bukan sekadar cara merakit sensor. IoT adalah soal merancang aliran data yang jujur: memisahkan secret dari kode, menangani sensor yang gagal membaca, dan berani menuliskan batasan alat di dokumentasi. Kadar CH₄ 65% di poster kami adalah kandungan metana di dalam biogas, bukan bacaan MQ-4 — dan itu penting untuk dijelaskan, bukan disembunyikan.

Kode firmware, dashboard, panduan perakitan, dan slide presentasi tersedia di GitHub:

**https://github.com/fannndi-2nd/smart-biogas-digester**

---

### Gambar yang disarankan

1. `assets/poster.jpg` — poster pameran Smart Biogas Digester.
2. `assets/dashboard.png` — tangkapan layar dashboard monitoring.

### Tim

- Andriyan Susetyo — 240103125
- Alwi Sofyan Afandi — 240103124
- Nisa Aulia Ramadhani — 240103143
- Guritno Wulandoro Suryosaputro — 240103132

Mata kuliah Internet of Things · Semester 5 · 2026.
