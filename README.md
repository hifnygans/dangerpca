# DangerPCA ERP Sekolah

Sistem ERP Sekolah desktop native, ringan, dan cepat yang ditulis menggunakan bahasa C murni. Aplikasi ini menggunakan **SDL2** untuk rendering window, **Nuklear** untuk antarmuka pengguna (GUI) immediate-mode dengan estetika ala Material Design (Light/Dark Mode), serta **SQLite3** untuk manajemen database lokal.

Aplikasi ini bersifat **100% portable** karena file aset font (Roboto & FontAwesome) dikompilasi langsung ke dalam binary executable (embedded), sehingga aplikasi tidak membutuhkan folder aset eksternal dan dapat langsung dijalankan di PC mana pun secara mandiri.

---

## 🌟 Fitur Utama

- **Dashboard Real-time**: Monitoring cepat total siswa aktif, jumlah guru, kelas, dan persentase kehadiran hari ini.
- **Manajemen Data Siswa**: CRUD data siswa lengkap dengan pencarian berdasarkan NISN/Nama, filter kelas, jenis kelamin, dan status aktif.
- **Manajemen Data Guru**: CRUD data guru lengkap dengan NIP, mata pelajaran, jenis kelamin, dan status mengajar.
- **Manajemen Kelas (Rombel)**: Pengelolaan rombongan belajar beserta tahun ajaran dan penunjukan Wali Kelas.
- **Absensi / Kehadiran**: Pencatatan kehadiran siswa harian (Hadir, Sakit, Izin, Alpa) per kelas.
- **Akademik & Nilai**: Input nilai mata pelajaran, pencatatan jurnal mengajar guru, dan pengelolaan raport/kelas.
- **Manajemen Pengguna**: Pengelolaan hak akses akun (Administrator, Guru, Siswa).
- **Sistem Backup Database**: Fitur backup database SQLite sekali klik ke folder `backups/` dengan penamaan tanggal otomatis.
- **Material Theme Toggle**: Dukungan penuh untuk pergantian tema instan antara **Mode Terang (Light Mode)** dan **Mode Gelap (Dark Mode)**.

---

## 🔑 Kredensial Login Bawaan

Saat aplikasi pertama kali dijalankan, database `school_erp.db` akan dibuat secara otomatis dengan akun administrator default berikut:

- **Username**: `admin`
- **Password**: `admin`
- **Role**: `Administrator`

---

## 🛠️ Persyaratan Sistem & Dependensi

### Linux (Debian/Ubuntu)
Instal dependensi pembangunan sistem melalui terminal:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libsdl2-dev libsqlite3-dev
```

### Windows
- Visual Studio (dengan Workload Desktop Development with C++).
- CMake.
- Dependensi diinstal via vcpkg (static linking):
  ```cmd
  vcpkg install sdl2:x64-windows-static sqlite3:x64-windows-static
  ```

---

## 🚀 Cara Kompilasi & Build

### 1. Di Linux / macOS
Jalankan perintah berikut di root direktori proyek untuk melakukan kompilasi:
```bash
# Buat folder build
mkdir -p build
cd build

# Konfigurasi CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Bangun executable
make -j$(nproc)
```
Hasil executable bernama `dangerpca` akan berada di folder `build/`.

### 2. Di Windows
Jalankan Developer Command Prompt untuk VS dan ketik:
```cmd
:: Buat folder build
mkdir build
cd build

:: Konfigurasi CMake dengan vcpkg static
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static

:: Bangun binary executable
cmake --build . --config Release
```
Hasil executable bernama `dangerpca.exe` akan berada di folder `build/Release/`.

---

## 运行 Cara Menjalankan

Cukup jalankan file executable hasil build di atas:
- Di Linux: `./build/dangerpca`
- Di Windows: Double-click pada `build/Release/dangerpca.exe`

Aplikasi akan mendeteksi file database `school_erp.db` secara lokal. Jika belum ada, program akan menginisiasi database dan migrasi tabel secara otomatis.
