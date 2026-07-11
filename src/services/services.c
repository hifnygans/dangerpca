#include "services.h"
#include "../database/db.h"
#include "../thirdparty/pdfgen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper to get current time string (YYYY-MM-DD HH:MM:SS)
static void get_current_time_str(char *buf, int max_len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, max_len, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Helper to check if a file exists and is writable
static bool is_file_writable(const char *path) {
    FILE *f = fopen(path, "ab");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

// Backup DB
bool service_backup_db(const char *dest_path) {
    if (!g_db) return false;
    sqlite3 *pDest;
    int rc = sqlite3_open(dest_path, &pDest);
    if (rc != SQLITE_OK) {
        sqlite3_close(pDest);
        return false;
    }
    sqlite3_backup *pBackup = sqlite3_backup_init(pDest, "main", g_db, "main");
    if (pBackup) {
        sqlite3_backup_step(pBackup, -1);
        sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pDest);
    sqlite3_close(pDest);
    return (rc == SQLITE_OK);
}

// Restore DB
bool service_restore_db(const char *src_path) {
    if (!g_db) return false;
    sqlite3 *pSrc;
    int rc = sqlite3_open(src_path, &pSrc);
    if (rc != SQLITE_OK) {
        sqlite3_close(pSrc);
        return false;
    }
    sqlite3_backup *pBackup = sqlite3_backup_init(g_db, "main", pSrc, "main");
    if (pBackup) {
        sqlite3_backup_step(pBackup, -1);
        sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(g_db);
    sqlite3_close(pSrc);
    return (rc == SQLITE_OK);
}

// CSV EXPORTS
bool service_export_students_csv(const char *dest_path) {
    FILE *f = fopen(dest_path, "w");
    if (!f) return false;

    // Excel compatibility BOM
    fprintf(f, "\xEF\xBB\xBF");
    fprintf(f, "NISN,Nama Siswa,Kelas,Jenis Kelamin,Tanggal Lahir,Alamat,Status\n");

    Student students[1000];
    int count = 0;
    if (db_get_students(students, 1000, NULL, 0, &count)) {
        for (int i = 0; i < count; i++) {
            fprintf(f, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
                    students[i].nisn,
                    students[i].name,
                    students[i].class_name,
                    strcmp(students[i].gender, "L") == 0 ? "Laki-laki" : "Perempuan",
                    students[i].dob,
                    students[i].address,
                    students[i].status);
        }
    }

    fclose(f);
    return true;
}

bool service_export_teachers_csv(const char *dest_path) {
    FILE *f = fopen(dest_path, "w");
    if (!f) return false;

    fprintf(f, "\xEF\xBB\xBF");
    fprintf(f, "NIP,Nama Guru,Jenis Kelamin,Mata Pelajaran,No Telepon,Status\n");

    Teacher teachers[1000];
    int count = 0;
    if (db_get_teachers(teachers, 1000, NULL, &count)) {
        for (int i = 0; i < count; i++) {
            fprintf(f, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
                    teachers[i].nip,
                    teachers[i].name,
                    strcmp(teachers[i].gender, "L") == 0 ? "Laki-laki" : "Perempuan",
                    teachers[i].subject,
                    teachers[i].phone,
                    teachers[i].status);
        }
    }

    fclose(f);
    return true;
}

bool service_export_attendance_csv(const char *dest_path, const char *date, int class_id, const char *class_name) {
    FILE *f = fopen(dest_path, "w");
    if (!f) return false;

    fprintf(f, "\xEF\xBB\xBF");
    fprintf(f, "Laporan Absensi Kelas: %s, Tanggal: %s\n\n", class_name, date);
    fprintf(f, "Nama Siswa,Status Kehadiran,Catatan/Keterangan\n");

    Attendance att[1000];
    int count = 0;
    if (db_get_attendance(att, 1000, date, class_id, &count)) {
        for (int i = 0; i < count; i++) {
            const char *status_str = "Hadir";
            if (att[i].status == ATT_SAKIT) status_str = "Sakit";
            else if (att[i].status == ATT_IZIN) status_str = "Izin";
            else if (att[i].status == ATT_ALPA) status_str = "Alpa";

            fprintf(f, "\"%s\",\"%s\",\"%s\"\n",
                    att[i].student_name,
                    status_str,
                    att[i].notes);
        }
    }

    fclose(f);
    return true;
}

bool service_export_grades_csv(const char *dest_path, int student_id, const char *student_name) {
    FILE *f = fopen(dest_path, "w");
    if (!f) return false;

    fprintf(f, "\xEF\xBB\xBF");
    fprintf(f, "Laporan Nilai Siswa: %s\n\n", student_name);

    // Section 1: Daily Grades
    fprintf(f, "NILAI HARIAN (Capaian TP)\n");
    fprintf(f, "Kode TP,Nilai,Tanggal,Catatan\n");
    DailyGrade dgrades[1000];
    int dcount = 0;
    if (db_get_daily_grades(dgrades, 1000, student_id, 0, &dcount)) {
        for (int i = 0; i < dcount; i++) {
            fprintf(f, "\"%s\",%.1f,\"%s\",\"%s\"\n",
                    dgrades[i].tp_code,
                    dgrades[i].score,
                    dgrades[i].date,
                    dgrades[i].notes);
        }
    }

    fprintf(f, "\n");

    // Section 2: Exam Grades
    fprintf(f, "NILAI UJIAN\n");
    fprintf(f, "Mata Pelajaran,Jenis Ujian,Nilai,Tanggal\n");
    ExamGrade egrades[1000];
    int ecount = 0;
    if (db_get_exam_grades(egrades, 1000, student_id, NULL, &ecount)) {
        for (int i = 0; i < ecount; i++) {
            fprintf(f, "\"%s\",\"%s\",%.1f,\"%s\"\n",
                    egrades[i].subject,
                    egrades[i].exam_type == EXAM_UTS ? "UTS" : "UAS",
                    egrades[i].score,
                    egrades[i].date);
        }
    }

    fclose(f);
    return true;
}

// PDF EXPORTS (using pdfgen)
static void draw_pdf_header(struct pdf_doc *pdf, struct pdf_object *page, const char *title, const char *meta) {
    // Top Banner Background
    pdf_add_filled_rectangle(pdf, page, 0, 780, 595, 62, 0, PDF_RGB(20, 24, 33), PDF_RGB(20, 24, 33));
    
    // Title
    pdf_add_text(pdf, page, title, 16, 30, 810, PDF_WHITE);
    
    // Sub-header / Metadata
    pdf_add_text(pdf, page, meta, 10, 30, 790, PDF_RGB(160, 170, 185));
    
    // Footer line & text
    pdf_add_line(pdf, page, 30, 40, 565, 40, 1, PDF_RGB(200, 200, 200));
    pdf_add_text(pdf, page, "DangerPCA School ERP - Dokumen Resmi - Rahasia", 8, 30, 28, PDF_RGB(120, 120, 120));
}

bool service_export_students_pdf(const char *dest_path) {
    struct pdf_info info = {
        .creator = "DangerPCA ERP",
        .producer = "DangerPCA ERP",
        .title = "Data Siswa",
        .author = "Administrator",
        .subject = "Laporan Siswa",
        .date = ""
    };
    get_current_time_str(info.date, sizeof(info.date));

    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    if (!pdf) return false;

    // Use default Times-Roman or Helvetica if TTF fails, or load Roboto
    pdf_set_font_ttf(pdf, "assets/Roboto-Regular.ttf");

    Student students[1000];
    int count = 0;
    if (!db_get_students(students, 1000, NULL, 0, &count)) {
        pdf_destroy(pdf);
        return false;
    }

    struct pdf_object *page = NULL;
    int y = 50; // starts low so it triggers page creation immediately

    for (int i = 0; i < count; i++) {
        if (y < 80) {
            page = pdf_append_page(pdf);
            draw_pdf_header(pdf, page, "LAPORAN DATA SISWA", "Dicetak pada: ");
            
            // Draw Table Header
            y = 740;
            pdf_add_filled_rectangle(pdf, page, 30, y - 5, 535, 20, 0, PDF_RGB(40, 50, 70), PDF_RGB(40, 50, 70));
            pdf_add_text(pdf, page, "NISN", 10, 40, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Nama Siswa", 10, 130, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Kelas", 10, 310, y, PDF_WHITE);
            pdf_add_text(pdf, page, "JK", 10, 410, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Status", 10, 480, y, PDF_WHITE);
            y -= 25;
        }

        // Draw Row Border
        pdf_add_line(pdf, page, 30, y - 5, 565, y - 5, 0.5f, PDF_RGB(220, 225, 230));

        // Draw Row Text
        pdf_add_text(pdf, page, students[i].nisn, 9, 40, y, PDF_BLACK);
        pdf_add_text(pdf, page, students[i].name, 9, 130, y, PDF_BLACK);
        pdf_add_text(pdf, page, students[i].class_name, 9, 310, y, PDF_BLACK);
        pdf_add_text(pdf, page, students[i].gender, 9, 410, y, PDF_BLACK);
        pdf_add_text(pdf, page, students[i].status, 9, 480, y, PDF_BLACK);

        y -= 22;
    }

    if (count == 0) {
        page = pdf_append_page(pdf);
        draw_pdf_header(pdf, page, "LAPORAN DATA SISWA", "Data Kosong");
        pdf_add_text(pdf, page, "Tidak ada data siswa ditemukan.", 12, 50, 600, PDF_BLACK);
    }

    int rc = pdf_save(pdf, dest_path);
    pdf_destroy(pdf);
    return (rc >= 0);
}

bool service_export_teachers_pdf(const char *dest_path) {
    struct pdf_info info = {
        .creator = "DangerPCA ERP",
        .producer = "DangerPCA ERP",
        .title = "Data Guru",
        .author = "Administrator",
        .subject = "Laporan Guru",
        .date = ""
    };
    get_current_time_str(info.date, sizeof(info.date));

    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    if (!pdf) return false;

    pdf_set_font_ttf(pdf, "assets/Roboto-Regular.ttf");

    Teacher teachers[1000];
    int count = 0;
    if (!db_get_teachers(teachers, 1000, NULL, &count)) {
        pdf_destroy(pdf);
        return false;
    }

    struct pdf_object *page = NULL;
    int y = 50;

    for (int i = 0; i < count; i++) {
        if (y < 80) {
            page = pdf_append_page(pdf);
            draw_pdf_header(pdf, page, "LAPORAN DATA GURU", "Dicetak pada: ");
            
            // Draw Table Header
            y = 740;
            pdf_add_filled_rectangle(pdf, page, 30, y - 5, 535, 20, 0, PDF_RGB(40, 50, 70), PDF_RGB(40, 50, 70));
            pdf_add_text(pdf, page, "NIP", 10, 40, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Nama Guru", 10, 150, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Mata Pelajaran", 10, 310, y, PDF_WHITE);
            pdf_add_text(pdf, page, "No Telepon", 10, 430, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Status", 10, 510, y, PDF_WHITE);
            y -= 25;
        }

        pdf_add_line(pdf, page, 30, y - 5, 565, y - 5, 0.5f, PDF_RGB(220, 225, 230));

        pdf_add_text(pdf, page, teachers[i].nip, 9, 40, y, PDF_BLACK);
        pdf_add_text(pdf, page, teachers[i].name, 9, 150, y, PDF_BLACK);
        pdf_add_text(pdf, page, teachers[i].subject, 9, 310, y, PDF_BLACK);
        pdf_add_text(pdf, page, teachers[i].phone, 9, 430, y, PDF_BLACK);
        pdf_add_text(pdf, page, teachers[i].status, 9, 510, y, PDF_BLACK);

        y -= 22;
    }

    if (count == 0) {
        page = pdf_append_page(pdf);
        draw_pdf_header(pdf, page, "LAPORAN DATA GURU", "Data Kosong");
        pdf_add_text(pdf, page, "Tidak ada data guru ditemukan.", 12, 50, 600, PDF_BLACK);
    }

    int rc = pdf_save(pdf, dest_path);
    pdf_destroy(pdf);
    return (rc >= 0);
}

bool service_export_attendance_pdf(const char *dest_path, const char *date, int class_id, const char *class_name) {
    struct pdf_info info = {
        .creator = "DangerPCA ERP",
        .producer = "DangerPCA ERP",
        .title = "Absensi Siswa",
        .author = "Administrator",
        .subject = "Laporan Absensi",
        .date = ""
    };
    get_current_time_str(info.date, sizeof(info.date));

    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    if (!pdf) return false;

    pdf_set_font_ttf(pdf, "assets/Roboto-Regular.ttf");

    Attendance att[1000];
    int count = 0;
    if (!db_get_attendance(att, 1000, date, class_id, &count)) {
        pdf_destroy(pdf);
        return false;
    }

    struct pdf_object *page = NULL;
    int y = 50;
    char meta[200];
    snprintf(meta, sizeof(meta), "Kelas: %s  |  Tanggal: %s", class_name, date);

    for (int i = 0; i < count; i++) {
        if (y < 80) {
            page = pdf_append_page(pdf);
            draw_pdf_header(pdf, page, "LAPORAN ABSENSI SISWA", meta);
            
            // Draw Table Header
            y = 740;
            pdf_add_filled_rectangle(pdf, page, 30, y - 5, 535, 20, 0, PDF_RGB(40, 50, 70), PDF_RGB(40, 50, 70));
            pdf_add_text(pdf, page, "Nama Siswa", 10, 40, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Status Kehadiran", 10, 260, y, PDF_WHITE);
            pdf_add_text(pdf, page, "Keterangan / Catatan", 10, 390, y, PDF_WHITE);
            y -= 25;
        }

        pdf_add_line(pdf, page, 30, y - 5, 565, y - 5, 0.5f, PDF_RGB(220, 225, 230));

        const char *status_str = "Hadir";
        uint32_t status_color = PDF_RGB(30, 130, 70); // Green
        if (att[i].status == ATT_SAKIT) {
            status_str = "Sakit";
            status_color = PDF_RGB(220, 130, 20); // Orange
        } else if (att[i].status == ATT_IZIN) {
            status_str = "Izin";
            status_color = PDF_RGB(30, 100, 200); // Blue
        } else if (att[i].status == ATT_ALPA) {
            status_str = "Alpa";
            status_color = PDF_RGB(200, 30, 30); // Red
        }

        pdf_add_text(pdf, page, att[i].student_name, 9, 40, y, PDF_BLACK);
        pdf_add_text(pdf, page, status_str, 9, 260, y, status_color);
        pdf_add_text(pdf, page, att[i].notes, 9, 390, y, PDF_BLACK);

        y -= 22;
    }

    if (count == 0) {
        page = pdf_append_page(pdf);
        draw_pdf_header(pdf, page, "LAPORAN ABSENSI SISWA", meta);
        pdf_add_text(pdf, page, "Tidak ada data absensi untuk tanggal ini.", 12, 50, 600, PDF_BLACK);
    }

    int rc = pdf_save(pdf, dest_path);
    pdf_destroy(pdf);
    return (rc >= 0);
}

bool service_export_grades_pdf(const char *dest_path, int student_id, const char *student_name) {
    struct pdf_info info = {
        .creator = "DangerPCA ERP",
        .producer = "DangerPCA ERP",
        .title = "Transkrip Nilai Siswa",
        .author = "Administrator",
        .subject = "Laporan Nilai",
        .date = ""
    };
    get_current_time_str(info.date, sizeof(info.date));

    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    if (!pdf) return false;

    pdf_set_font_ttf(pdf, "assets/Roboto-Regular.ttf");

    DailyGrade dgrades[1000];
    int dcount = 0;
    db_get_daily_grades(dgrades, 1000, student_id, 0, &dcount);

    ExamGrade egrades[1000];
    int ecount = 0;
    db_get_exam_grades(egrades, 1000, student_id, NULL, &ecount);

    struct pdf_object *page = pdf_append_page(pdf);
    char meta[200];
    snprintf(meta, sizeof(meta), "Nama Siswa: %s  |  Dicetak pada: %s", student_name, info.date);
    draw_pdf_header(pdf, page, "TRANSKRIP CAPAIAN & HASIL BELAJAR SISWA", meta);

    int y = 740;

    // SECTION 1: NILAI HARIAN
    pdf_add_text(pdf, page, "I. Capaian Nilai Harian (Berdasarkan Tujuan Pembelajaran)", 11, 30, y, PDF_RGB(40, 50, 70));
    y -= 18;
    pdf_add_filled_rectangle(pdf, page, 30, y - 5, 535, 18, 0, PDF_RGB(230, 235, 245), PDF_RGB(200, 210, 225));
    pdf_add_text(pdf, page, "Kode TP", 9, 40, y, PDF_RGB(40, 50, 70));
    pdf_add_text(pdf, page, "Nilai", 9, 120, y, PDF_RGB(40, 50, 70));
    pdf_add_text(pdf, page, "Tanggal", 9, 200, y, PDF_RGB(40, 50, 70));
    pdf_add_text(pdf, page, "Catatan / Keterangan", 9, 300, y, PDF_RGB(40, 50, 70));
    y -= 20;

    for (int i = 0; i < dcount && y > 350; i++) {
        pdf_add_line(pdf, page, 30, y - 3, 565, y - 3, 0.5f, PDF_RGB(220, 225, 230));
        char score_str[20];
        snprintf(score_str, sizeof(score_str), "%.1f", dgrades[i].score);
        
        pdf_add_text(pdf, page, dgrades[i].tp_code, 9, 40, y, PDF_BLACK);
        pdf_add_text(pdf, page, score_str, 9, 120, y, dgrades[i].score >= 70 ? PDF_RGB(30, 120, 50) : PDF_RGB(200, 30, 30));
        pdf_add_text(pdf, page, dgrades[i].date, 9, 200, y, PDF_BLACK);
        pdf_add_text(pdf, page, dgrades[i].notes, 9, 300, y, PDF_BLACK);
        y -= 18;
    }
    if (dcount == 0) {
        pdf_add_text(pdf, page, "Belum ada nilai harian tercatat.", 9, 40, y, PDF_RGB(120, 120, 120));
        y -= 18;
    }

    // SECTION 2: NILAI UJIAN (FORCE ON LOWER HALF OR NEW PAGE IF NEEDED)
    if (y > 350) {
        y = 330;
    } else {
        page = pdf_append_page(pdf);
        draw_pdf_header(pdf, page, "TRANSKRIP CAPAIAN & HASIL BELAJAR SISWA", meta);
        y = 740;
    }

    pdf_add_text(pdf, page, "II. Hasil Ujian (Evaluasi Sumatif)", 11, 30, y, PDF_RGB(40, 50, 70));
    y -= 18;
    pdf_add_filled_rectangle(pdf, page, 30, y - 5, 535, 18, 0, PDF_RGB(230, 235, 245), PDF_RGB(200, 210, 225));
    pdf_add_text(pdf, page, "Mata Pelajaran", 9, 40, y, PDF_RGB(40, 50, 70));
    pdf_add_text(pdf, page, "Jenis Ujian", 9, 200, y, PDF_RGB(40, 50, 70));
    pdf_add_text(pdf, page, "Nilai", 9, 320, y, PDF_RGB(40, 50, 70));
    pdf_add_text(pdf, page, "Tanggal Evaluasi", 9, 420, y, PDF_RGB(40, 50, 70));
    y -= 20;

    for (int i = 0; i < ecount && y > 60; i++) {
        pdf_add_line(pdf, page, 30, y - 3, 565, y - 3, 0.5f, PDF_RGB(220, 225, 230));
        char score_str[20];
        snprintf(score_str, sizeof(score_str), "%.1f", egrades[i].score);

        pdf_add_text(pdf, page, egrades[i].subject, 9, 40, y, PDF_BLACK);
        pdf_add_text(pdf, page, egrades[i].exam_type == EXAM_UTS ? "UTS (Tengah Semester)" : "UAS (Akhir Semester)", 9, 200, y, PDF_BLACK);
        pdf_add_text(pdf, page, score_str, 9, 320, y, egrades[i].score >= 70 ? PDF_RGB(30, 120, 50) : PDF_RGB(200, 30, 30));
        pdf_add_text(pdf, page, egrades[i].date, 9, 420, y, PDF_BLACK);
        y -= 18;
    }
    if (ecount == 0) {
        pdf_add_text(pdf, page, "Belum ada nilai ujian tercatat.", 9, 40, y, PDF_RGB(120, 120, 120));
    }

    int rc = pdf_save(pdf, dest_path);
    pdf_destroy(pdf);
    return (rc >= 0);
}
