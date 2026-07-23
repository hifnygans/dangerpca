#ifndef DB_H
#define DB_H

#include "../models/models.h"
#include <sqlite3.h>

// Global database connection pointer
extern sqlite3 *g_db;

// Core functions
bool db_init(const char *db_path);
void db_close(void);
bool db_execute(const char *sql);

// Auth & Users
bool db_authenticate_user(const char *username, const char *password, User *out_user);
bool db_get_users(User *out_users, int max_users, int *out_count);
bool db_create_user(const User *user);
bool db_update_user(const User *user);
bool db_delete_user(int id);

// Students
bool db_get_students(Student *out_students, int max_students, const char *search, int class_id, int *out_count);
bool db_create_student(const Student *student);
bool db_update_student(const Student *student);
bool db_delete_student(int id);

// Teachers
bool db_get_teachers(Teacher *out_teachers, int max_teachers, const char *search, int *out_count);
bool db_create_teacher(const Teacher *teacher);
bool db_update_teacher(const Teacher *teacher);
bool db_delete_teacher(int id);

// Classes
bool db_get_classes(ClassEntity *out_classes, int max_classes, int *out_count);
bool db_create_class(const ClassEntity *cls);
bool db_update_class(const ClassEntity *cls);
bool db_delete_class(int id);

// Attendance
bool db_get_attendance(Attendance *out_att, int max_att, const char *date, int class_id, int *out_count);
bool db_save_attendance(int student_id, const char *date, AttendanceStatus status, const char *notes);

// Capaian Pembelajaran (CP)
bool db_get_cp(CapaianPembelajaran *out_cp, int max_cp, const char *search, int *out_count);
bool db_create_cp(const CapaianPembelajaran *cp);
bool db_update_cp(const CapaianPembelajaran *cp);
bool db_delete_cp(int id);

// Tujuan Pembelajaran (TP)
bool db_get_tp(TujuanPembelajaran *out_tp, int max_tp, int cp_id, int *out_count);
bool db_create_tp(const TujuanPembelajaran *tp);
bool db_update_tp(const TujuanPembelajaran *tp);
bool db_delete_tp(int id);

// Alur Tujuan Pembelajaran (ATP)
bool db_get_atp(AlurTujuanPembelajaran *out_atp, int max_atp, int tp_id, int *out_count);
bool db_create_atp(const AlurTujuanPembelajaran *atp);
bool db_update_atp(const AlurTujuanPembelajaran *atp);
bool db_delete_atp(int id);

// Daily Journal
bool db_get_journal(DailyJournal *out_journal, int max_journal, const char *date, int class_id, int *out_count);
bool db_create_journal(const DailyJournal *journal);
bool db_update_journal(const DailyJournal *journal);
bool db_delete_journal(int id);

// Grades Daily
bool db_get_daily_grades(DailyGrade *out_grades, int max_grades, int student_id, int tp_id, int *out_count);
bool db_save_daily_grade(int student_id, int tp_id, double score, const char *date, const char *notes);
bool db_delete_daily_grade(int id);

// Grades Exam
bool db_get_exam_grades(ExamGrade *out_grades, int max_grades, int student_id, const char *subject, int *out_count);
bool db_save_exam_grade(int student_id, const char *subject, ExamType exam_type, double score, const char *date);
bool db_delete_exam_grade(int id);

// Dashboard Statistics
bool db_get_dashboard_stats(DashboardStats *out_stats);

// Settings Persistence
bool db_get_setting(const char *key, const char *default_val, char *out_val, int max_len);
bool db_set_setting(const char *key, const char *val);
bool db_get_school_settings(SchoolSettings *out_settings);
bool db_save_school_settings(const SchoolSettings *settings);

#endif // DB_H
