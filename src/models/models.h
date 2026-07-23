#ifndef MODELS_H
#define MODELS_H

#include <stdbool.h>

// User roles
typedef enum {
    ROLE_ADMIN,
    ROLE_GURU,
    ROLE_STAF
} UserRole;

// Attendance statuses
typedef enum {
    ATT_HADIR,
    ATT_SAKIT,
    ATT_IZIN,
    ATT_ALPA
} AttendanceStatus;

// Exam types
typedef enum {
    EXAM_UTS,
    EXAM_UAS
} ExamType;

// Model Structs with super-expanded buffers for professional long-form journaling
typedef struct {
    int id;
    char username[50];
    char password_hash[100];
    char name[100];
    UserRole role;
} User;

typedef struct {
    int id;
    char nisn[50];
    char name[150];
    int class_id;
    char class_name[50]; // helper for displaying
    char gender[10]; // "L" or "P"
    char dob[20]; // YYYY-MM-DD
    char address[4096];
    char status[50]; // "Aktif", "Lulus", "Keluar"
} Student;

typedef struct {
    int id;
    char nip[50];
    char name[150];
    char gender[10]; // "L" or "P"
    char subject[100];
    char phone[50];
    char status[50]; // "Aktif", "Cuti", "Pensiun"
} Teacher;

typedef struct {
    int id;
    char name[50];
    char academic_year[50]; // e.g. 2025/2026
    int teacher_id;
    char teacher_name[150]; // helper
} ClassEntity;

typedef struct {
    int id;
    int student_id;
    char student_name[150]; // helper
    char class_name[50]; // helper
    char date[20]; // YYYY-MM-DD
    AttendanceStatus status;
    char notes[4096];
} Attendance;

typedef struct {
    int id;
    char code[50]; // e.g. CP-001
    char description[32768]; // 32 KB capacity
    char subject[100];
} CapaianPembelajaran;

typedef struct {
    int id;
    int cp_id;
    char cp_code[50]; // helper
    char code[50]; // e.g. TP-001
    char description[32768]; // 32 KB capacity
} TujuanPembelajaran;

typedef struct {
    int id;
    int tp_id;
    char tp_code[50]; // helper
    int order_num;
    char description[32768]; // 32 KB capacity
} AlurTujuanPembelajaran;

typedef struct {
    int id;
    int teacher_id;
    char teacher_name[150]; // helper
    int class_id;
    char class_name[50]; // helper
    char date[20]; // YYYY-MM-DD
    char activity[65536]; // 64 KB capacity for rich long journals
    char notes[65536]; // 64 KB capacity for notes & evaluations
} DailyJournal;

typedef struct {
    int id;
    int student_id;
    char student_name[150]; // helper
    int tp_id;
    char tp_code[50]; // helper
    double score;
    char date[20]; // YYYY-MM-DD
    char notes[4096];
} DailyGrade;

typedef struct {
    int id;
    int student_id;
    char student_name[150]; // helper
    char subject[100];
    ExamType exam_type;
    double score;
    char date[20]; // YYYY-MM-DD
} ExamGrade;

// Key-Value Settings Struct
typedef struct {
    char school_name[150];
    char school_npsn[50];
    char school_address[256];
    char principal_name[150];
    char principal_nip[50];
    char academic_year[50];
    char semester[20];
} SchoolSettings;

// Dashboard Summary Struct
typedef struct {
    int total_students;
    int total_teachers;
    int total_classes;
    double attendance_rate_today;
    int total_users;
} DashboardStats;

#endif // MODELS_H
