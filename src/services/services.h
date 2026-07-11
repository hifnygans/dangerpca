#ifndef SERVICES_H
#define SERVICES_H

#include <stdbool.h>

// Backup & Restore
bool service_backup_db(const char *dest_path);
bool service_restore_db(const char *src_path);

// Exports
bool service_export_students_csv(const char *dest_path);
bool service_export_teachers_csv(const char *dest_path);
bool service_export_attendance_csv(const char *dest_path, const char *date, int class_id, const char *class_name);
bool service_export_grades_csv(const char *dest_path, int student_id, const char *student_name);

bool service_export_students_pdf(const char *dest_path);
bool service_export_teachers_pdf(const char *dest_path);
bool service_export_attendance_pdf(const char *dest_path, const char *date, int class_id, const char *class_name);
bool service_export_grades_pdf(const char *dest_path, int student_id, const char *student_name);

#endif // SERVICES_H
