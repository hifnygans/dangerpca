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

// Model Structs with fixed-size buffers for simpler C memory management
typedef struct {
    int id;
    char username[50];
    char password_hash[100];
    char name[100];
    UserRole role;
} User;

typedef struct {
    int id;
    char nisn[20];
    char name[100];
    int class_id;
    char class_name[50]; // helper for displaying
    char gender[10]; // "L" or "P"
    char dob[20]; // YYYY-MM-DD
    char address[200];
    char status[20]; // "Aktif", "Lulus", "Keluar"
} Student;

typedef struct {
    int id;
    char nip[30];
    char name[100];
    char gender[10]; // "L" or "P"
    char subject[100];
    char phone[20];
    char status[20]; // "Aktif", "Cuti", "Pensiun"
} Teacher;

typedef struct {
    int id;
    char name[50];
    char academic_year[20]; // e.g. 2025/2026
    int teacher_id;
    char teacher_name[100]; // helper
} ClassEntity;

typedef struct {
    int id;
    int student_id;
    char student_name[100]; // helper
    char class_name[50]; // helper
    char date[20]; // YYYY-MM-DD
    AttendanceStatus status;
    char notes[100];
} Attendance;

typedef struct {
    int id;
    char code[20]; // e.g. CP-001
    char description[2048];
    char subject[100];
} CapaianPembelajaran;

typedef struct {
    int id;
    int cp_id;
    char cp_code[20]; // helper
    char code[20]; // e.g. TP-001
    char description[2048];
} TujuanPembelajaran;

typedef struct {
    int id;
    int tp_id;
    char tp_code[20]; // helper
    int order_num;
    char description[2048];
} AlurTujuanPembelajaran;

typedef struct {
    int id;
    int teacher_id;
    char teacher_name[100]; // helper
    int class_id;
    char class_name[50]; // helper
    char date[20]; // YYYY-MM-DD
    char activity[2048];
    char notes[2048];
} DailyJournal;

typedef struct {
    int id;
    int student_id;
    char student_name[100]; // helper
    int tp_id;
    char tp_code[20]; // helper
    double score;
    char date[20]; // YYYY-MM-DD
    char notes[100];
} DailyGrade;

typedef struct {
    int id;
    int student_id;
    char student_name[100]; // helper
    char subject[100];
    ExamType exam_type;
    double score;
    char date[20]; // YYYY-MM-DD
} ExamGrade;

// Dashboard Summary Struct
typedef struct {
    int total_students;
    int total_teachers;
    int total_classes;
    double attendance_rate_today;
    int total_users;
} DashboardStats;

#endif // MODELS_H
