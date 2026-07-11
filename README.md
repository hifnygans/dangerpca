# DangerPCA ERP Sekolah - Khusus Buat Ica! 💖

Halo Ica! Ini aplikasi ERP Sekolah desktop native yang super ringan, cepet, dan gak ribet. Dibikin pakai bahasa C murni, aplikasi ini pakai **SDL2** buat nampilin window, **Nuklear** buat tampilan antarmuka (GUI) yang udah disetting pakai tema Material Design (bisa gonta-ganti Light/Dark Mode), dan **SQLite3** buat nyimpen data-datanya secara lokal.

Aplikasi ini udah **100% portable**, Ca! File font (Roboto) sama semua ikon kerennya udah ditanam langsung di dalam aplikasi (embedded). Jadi, Ica tinggal jalankan satu file executable-nya aja tanpa perlu ribet mindahin folder aset atau takut ikonnya berubah jadi tanda tanya.

---

## 📥 Cara Download Instan (Gak Perlu Build Sendiri, Ca!)

Kalau Ica cuma pengen langsung pakai aplikasinya tanpa perlu ribet urusan coding atau compile, Ica bisa langsung download file installernya yang siap pakai:

👉 **[Download Aplikasi (.exe / .deb) di Halaman Releases GitHub](https://github.com/hifnygans/dangerpca/releases/latest)**

- **Pengguna Windows**: Cukup download `dangerpca-windows.exe`, tinggal double-click langsung bisa dipakai.
- **Pengguna Linux (Ubuntu/Debian)**: Download file `.deb` (bisa double-click untuk langsung install ke sistem) atau download binary standalone `dangerpca-linux`.

---

## 🔑 Kredensial Login (Jangan Lupa ya, Ca!)

Pas pertama kali Ica buka aplikasi ini, database `school_erp.db` bakal otomatis dibuat secara ajaib. Nah, buat masuk ke dashboard-nya, Ica bisa login pakai akun admin default ini:

- **Username**: `admin`
- **Password**: `admin`
- **Role**: `Administrator`

---

## 🌟 Fitur-Fitur Keren yang Bisa Ica Pake

- **Dashboard Kece**: Langsung kelihatan total siswa aktif, jumlah guru, kelas, dan grafik persentase absensi hari ini.
- **Manajemen Siswa**: Ica bisa tambah, edit, cari (pakai NISN/Nama), dan hapus data siswa dengan mudah.
- **Manajemen Guru**: Kelola data guru lengkap dengan NIP, jenis kelamin, dan mata pelajaran yang diajar.
- **Manajemen Kelas (Rombel)**: Kelola rombongan belajar beserta Wali Kelas dan tahun ajarannya.
- **Absensi Siswa**: Ica bisa catat absensi harian siswa (Hadir, Sakit, Izin, Alpa) per kelas.
- **Akademik & Nilai**: Input nilai pelajaran, catat jurnal mengajar guru, dan pantau raport kelas.
- **Manajemen Pengguna**: Atur akun-akun yang bisa akses aplikasi ini (Admin, Guru, Siswa).
- **Backup Satu Klik**: Ica takut datanya hilang? Tenang, tinggal klik tombol backup, database langsung tersimpan aman di folder `backups/` lengkap dengan tanggal otomatis.
- **Tema Terang & Gelap**: Layar Ica silau? Tinggal ganti ke Mode Gelap lewat tombol di pojok kanan atas biar mata Ica gak capek.

---

## 🛠️ Persiapan Sebelum Build (Dependensi)

Sebelum mulai build, pastiin komputer yang Ica pake udah terpasang dependensi ini ya:

### Kalau Ica pakai Linux (Debian/Ubuntu):
Tinggal copas perintah ini ke terminal:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libsdl2-dev libsqlite3-dev
```

### Kalau Ica pakai Windows:
- Pasang **Visual Studio** (pilih workload "Desktop Development with C++").
- Pasang **CMake**.
- Pasang library dependensi lewat vcpkg biar statis:
  ```cmd
  vcpkg install sdl2:x64-windows-static sqlite3:x64-windows-static
  ```

---

## 🚀 Cara Build Aplikasi (Gampang Kok, Ca!)

### 1. Cara Build di Linux:
Buka terminal di folder proyek ini, terus ketik perintah ini satu-satu:
```bash
# Buat folder build biar rapi
mkdir -p build
cd build

# Konfigurasi CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Mulai compile aplikasinya
make -j$(nproc)
```
Nanti file executable namanya `dangerpca` bakal muncul di folder `build/`. Ica tinggal jalankan pakai `./dangerpca`!

### 2. Cara Build di Windows:
Buka Developer Command Prompt bawaan VS, masuk ke folder proyek ini, lalu ketik:
```cmd
:: Buat folder build
mkdir build
cd build

:: Konfigurasi CMake pakai vcpkg static
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static

:: Mulai compile
cmake --build . --config Release
```
Hasil executable bernama `dangerpca.exe` bakal muncul di folder `build/Release/`. Tinggal Ica double-click aja buat mainin!

---

Semoga aplikasi ini membantu pekerjaan Ica ya! Semangat terus, Ca! 🌸
