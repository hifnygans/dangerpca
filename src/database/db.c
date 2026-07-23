#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

sqlite3 *g_db = NULL;

static void hash_password(const char *password, char *out_hash, int max_len) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    snprintf(out_hash, max_len, "%lx", hash);
}

bool db_execute(const char *sql) {
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

bool db_init(const char *db_path) {
    int rc = sqlite3_open(db_path, &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(g_db));
        return false;
    }

    // Enable foreign keys
    db_execute("PRAGMA foreign_keys = ON;");

    // Start Transaction for Migrations
    db_execute("BEGIN TRANSACTION;");

    // Users Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "role INTEGER NOT NULL"
        ");"
    );

    // Teachers Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS teachers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nip TEXT UNIQUE NOT NULL,"
        "name TEXT NOT NULL,"
        "gender TEXT NOT NULL,"
        "subject TEXT NOT NULL,"
        "phone TEXT NOT NULL,"
        "status TEXT NOT NULL"
        ");"
    );

    // Classes Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS classes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "academic_year TEXT NOT NULL,"
        "teacher_id INTEGER,"
        "FOREIGN KEY(teacher_id) REFERENCES teachers(id) ON DELETE SET NULL"
        ");"
    );

    // Students Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS students ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nisn TEXT UNIQUE NOT NULL,"
        "name TEXT NOT NULL,"
        "class_id INTEGER,"
        "gender TEXT NOT NULL,"
        "dob TEXT NOT NULL,"
        "address TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "FOREIGN KEY(class_id) REFERENCES classes(id) ON DELETE SET NULL"
        ");"
    );

    // Attendance Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS attendance ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER NOT NULL,"
        "date TEXT NOT NULL,"
        "status INTEGER NOT NULL,"
        "notes TEXT,"
        "UNIQUE(student_id, date),"
        "FOREIGN KEY(student_id) REFERENCES students(id) ON DELETE CASCADE"
        ");"
    );

    // Capaian Pembelajaran (CP) Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS cp ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "code TEXT UNIQUE NOT NULL,"
        "description TEXT NOT NULL,"
        "subject TEXT NOT NULL"
        ");"
    );

    // Tujuan Pembelajaran (TP) Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS tp ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "cp_id INTEGER NOT NULL,"
        "code TEXT UNIQUE NOT NULL,"
        "description TEXT NOT NULL,"
        "FOREIGN KEY(cp_id) REFERENCES cp(id) ON DELETE CASCADE"
        ");"
    );

    // Alur Tujuan Pembelajaran (ATP) Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS atp ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "tp_id INTEGER NOT NULL,"
        "order_num INTEGER NOT NULL,"
        "description TEXT NOT NULL,"
        "FOREIGN KEY(tp_id) REFERENCES tp(id) ON DELETE CASCADE"
        ");"
    );

    // Daily Journal Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS daily_journal ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "teacher_id INTEGER NOT NULL,"
        "class_id INTEGER NOT NULL,"
        "date TEXT NOT NULL,"
        "activity TEXT NOT NULL,"
        "notes TEXT,"
        "FOREIGN KEY(teacher_id) REFERENCES teachers(id) ON DELETE CASCADE,"
        "FOREIGN KEY(class_id) REFERENCES classes(id) ON DELETE CASCADE"
        ");"
    );

    // Grades Daily Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS grades_daily ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER NOT NULL,"
        "tp_id INTEGER NOT NULL,"
        "score REAL NOT NULL,"
        "date TEXT NOT NULL,"
        "notes TEXT,"
        "UNIQUE(student_id, tp_id, date),"
        "FOREIGN KEY(student_id) REFERENCES students(id) ON DELETE CASCADE,"
        "FOREIGN KEY(tp_id) REFERENCES tp(id) ON DELETE CASCADE"
        ");"
    );

    // Grades Exam Table
    db_execute(
        "CREATE TABLE IF NOT EXISTS grades_exam ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER NOT NULL,"
        "subject TEXT NOT NULL,"
        "exam_type INTEGER NOT NULL,"
        "score REAL NOT NULL,"
        "date TEXT NOT NULL,"
        "UNIQUE(student_id, subject, exam_type),"
        "FOREIGN KEY(student_id) REFERENCES students(id) ON DELETE CASCADE"
        ");"
    );

    // Settings Table for System & School Configuration
    db_execute(
        "CREATE TABLE IF NOT EXISTS settings ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL"
        ");"
    );

    db_execute("COMMIT;");

    // Insert Default Admin if not exists
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM users", -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            if (count == 0) {
                // Insert default admin
                char pass_hash[100];
                hash_password("admin", pass_hash, sizeof(pass_hash));
                sqlite3_stmt *instmt;
                const char *insql = "INSERT INTO users (username, password_hash, name, role) VALUES ('admin', ?, 'Administrator', 0)";
                if (sqlite3_prepare_v2(g_db, insql, -1, &instmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_text(instmt, 1, pass_hash, -1, SQLITE_TRANSIENT);
                    sqlite3_step(instmt);
                    sqlite3_finalize(instmt);
                }
            }
        }
        sqlite3_finalize(stmt);
    }

    return true;
}

void db_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

// Auth & Users
bool db_authenticate_user(const char *username, const char *password, User *out_user) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, username, password_hash, name, role FROM users WHERE username = ?";
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char pass_hash[100];
        hash_password(password, pass_hash, sizeof(pass_hash));
        const char *db_hash = (const char *)sqlite3_column_text(stmt, 2);
        if (strcmp(pass_hash, db_hash) == 0) {
            out_user->id = sqlite3_column_int(stmt, 0);
            strncpy(out_user->username, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_user->username) - 1);
            strncpy(out_user->name, (const char *)sqlite3_column_text(stmt, 3), sizeof(out_user->name) - 1);
            out_user->role = (UserRole)sqlite3_column_int(stmt, 4);
            success = true;
        }
    }
    sqlite3_finalize(stmt);
    return success;
}

bool db_get_users(User *out_users, int max_users, int *out_count) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, username, name, role FROM users ORDER BY name ASC";
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_users) {
        out_users[count].id = sqlite3_column_int(stmt, 0);
        strncpy(out_users[count].username, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_users[count].username) - 1);
        out_users[count].password_hash[0] = '\0';
        strncpy(out_users[count].name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_users[count].name) - 1);
        out_users[count].role = (UserRole)sqlite3_column_int(stmt, 3);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_user(const User *user) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (username, password_hash, name, role) VALUES (?, ?, ?, ?)";
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    char pass_hash[100];
    hash_password(user->password_hash, pass_hash, sizeof(pass_hash));

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pass_hash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, (int)user->role);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_user(const User *user) {
    sqlite3_stmt *stmt;
    bool success;
    if (strlen(user->password_hash) > 0) {
        const char *sql = "UPDATE users SET username = ?, password_hash = ?, name = ?, role = ? WHERE id = ?";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
        char pass_hash[100];
        hash_password(user->password_hash, pass_hash, sizeof(pass_hash));
        sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pass_hash, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, user->name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, (int)user->role);
        sqlite3_bind_int(stmt, 5, user->id);
    } else {
        const char *sql = "UPDATE users SET username = ?, name = ?, role = ? WHERE id = ?";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
        sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user->name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, (int)user->role);
        sqlite3_bind_int(stmt, 4, user->id);
    }
    success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_user(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM users WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Students
bool db_get_students(Student *out_students, int max_students, const char *search, int class_id, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT s.id, s.nisn, s.name, s.class_id, IFNULL(c.name, 'Belum Ada'), s.gender, s.dob, s.address, s.status "
                    "FROM students s LEFT JOIN classes c ON s.class_id = c.id WHERE 1=1";
    
    if (class_id > 0) {
        strcat(sql, " AND s.class_id = ?");
    }
    if (search && strlen(search) > 0) {
        strcat(sql, " AND (s.name LIKE ? OR s.nisn LIKE ?)");
    }
    strcat(sql, " ORDER BY s.name ASC");

    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    int bind_idx = 1;
    if (class_id > 0) {
        sqlite3_bind_int(stmt, bind_idx++, class_id);
    }
    char like_pattern[128] = {0};
    if (search && strlen(search) > 0) {
        snprintf(like_pattern, sizeof(like_pattern), "%%%s%%", search);
        sqlite3_bind_text(stmt, bind_idx++, like_pattern, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, bind_idx++, like_pattern, -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_students) {
        out_students[count].id = sqlite3_column_int(stmt, 0);
        strncpy(out_students[count].nisn, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_students[count].nisn) - 1);
        strncpy(out_students[count].name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_students[count].name) - 1);
        out_students[count].class_id = sqlite3_column_int(stmt, 3);
        strncpy(out_students[count].class_name, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_students[count].class_name) - 1);
        strncpy(out_students[count].gender, (const char *)sqlite3_column_text(stmt, 5), sizeof(out_students[count].gender) - 1);
        strncpy(out_students[count].dob, (const char *)sqlite3_column_text(stmt, 6), sizeof(out_students[count].dob) - 1);
        strncpy(out_students[count].address, (const char *)sqlite3_column_text(stmt, 7), sizeof(out_students[count].address) - 1);
        strncpy(out_students[count].status, (const char *)sqlite3_column_text(stmt, 8), sizeof(out_students[count].status) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_student(const Student *student) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO students (nisn, name, class_id, gender, dob, address, status) VALUES (?, ?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, student->nisn, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, student->name, -1, SQLITE_TRANSIENT);
    if (student->class_id > 0) {
        sqlite3_bind_int(stmt, 3, student->class_id);
    } else {
        sqlite3_bind_null(stmt, 3);
    }
    sqlite3_bind_text(stmt, 4, student->gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, student->dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, student->address, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, student->status, -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_student(const Student *student) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE students SET nisn = ?, name = ?, class_id = ?, gender = ?, dob = ?, address = ?, status = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, student->nisn, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, student->name, -1, SQLITE_TRANSIENT);
    if (student->class_id > 0) {
        sqlite3_bind_int(stmt, 3, student->class_id);
    } else {
        sqlite3_bind_null(stmt, 3);
    }
    sqlite3_bind_text(stmt, 4, student->gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, student->dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, student->address, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, student->status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, student->id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_student(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM students WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Teachers
bool db_get_teachers(Teacher *out_teachers, int max_teachers, const char *search, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT id, nip, name, gender, subject, phone, status FROM teachers WHERE 1=1";
    if (search && strlen(search) > 0) {
        strcat(sql, " AND (name LIKE ? OR nip LIKE ? OR subject LIKE ?)");
    }
    strcat(sql, " ORDER BY name ASC");

    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    if (search && strlen(search) > 0) {
        char like_pattern[128];
        snprintf(like_pattern, sizeof(like_pattern), "%%%s%%", search);
        sqlite3_bind_text(stmt, 1, like_pattern, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, like_pattern, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, like_pattern, -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_teachers) {
        out_teachers[count].id = sqlite3_column_int(stmt, 0);
        strncpy(out_teachers[count].nip, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_teachers[count].nip) - 1);
        strncpy(out_teachers[count].name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_teachers[count].name) - 1);
        strncpy(out_teachers[count].gender, (const char *)sqlite3_column_text(stmt, 3), sizeof(out_teachers[count].gender) - 1);
        strncpy(out_teachers[count].subject, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_teachers[count].subject) - 1);
        strncpy(out_teachers[count].phone, (const char *)sqlite3_column_text(stmt, 5), sizeof(out_teachers[count].phone) - 1);
        strncpy(out_teachers[count].status, (const char *)sqlite3_column_text(stmt, 6), sizeof(out_teachers[count].status) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_teacher(const Teacher *teacher) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO teachers (nip, name, gender, subject, phone, status) VALUES (?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, teacher->nip, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, teacher->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, teacher->gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, teacher->subject, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, teacher->phone, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, teacher->status, -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_teacher(const Teacher *teacher) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE teachers SET nip = ?, name = ?, gender = ?, subject = ?, phone = ?, status = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, teacher->nip, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, teacher->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, teacher->gender, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, teacher->subject, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, teacher->phone, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, teacher->status, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, teacher->id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_teacher(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM teachers WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Classes
bool db_get_classes(ClassEntity *out_classes, int max_classes, int *out_count) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT c.id, c.name, c.academic_year, c.teacher_id, IFNULL(t.name, 'Belum Ditentukan') "
                      "FROM classes c LEFT JOIN teachers t ON c.teacher_id = t.id ORDER BY c.name ASC";
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_classes) {
        out_classes[count].id = sqlite3_column_int(stmt, 0);
        strncpy(out_classes[count].name, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_classes[count].name) - 1);
        strncpy(out_classes[count].academic_year, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_classes[count].academic_year) - 1);
        out_classes[count].teacher_id = sqlite3_column_int(stmt, 3);
        strncpy(out_classes[count].teacher_name, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_classes[count].teacher_name) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_class(const ClassEntity *cls) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO classes (name, academic_year, teacher_id) VALUES (?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, cls->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cls->academic_year, -1, SQLITE_TRANSIENT);
    if (cls->teacher_id > 0) {
        sqlite3_bind_int(stmt, 3, cls->teacher_id);
    } else {
        sqlite3_bind_null(stmt, 3);
    }

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_class(const ClassEntity *cls) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE classes SET name = ?, academic_year = ?, teacher_id = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, cls->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cls->academic_year, -1, SQLITE_TRANSIENT);
    if (cls->teacher_id > 0) {
        sqlite3_bind_int(stmt, 3, cls->teacher_id);
    } else {
        sqlite3_bind_null(stmt, 3);
    }
    sqlite3_bind_int(stmt, 4, cls->id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_class(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM classes WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Attendance
bool db_get_attendance(Attendance *out_att, int max_att, const char *date, int class_id, int *out_count) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT s.id, s.name, IFNULL(c.name, 'Belum Ada'), IFNULL(a.id, 0), IFNULL(a.status, 0), IFNULL(a.notes, '') "
                      "FROM students s JOIN classes c ON s.class_id = c.id "
                      "LEFT JOIN attendance a ON s.id = a.student_id AND a.date = ? "
                      "WHERE s.class_id = ? ORDER BY s.name ASC";
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, class_id);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_att) {
        out_att[count].student_id = sqlite3_column_int(stmt, 0);
        strncpy(out_att[count].student_name, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_att[count].student_name) - 1);
        strncpy(out_att[count].class_name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_att[count].class_name) - 1);
        out_att[count].id = sqlite3_column_int(stmt, 3);
        strncpy(out_att[count].date, date, sizeof(out_att[count].date) - 1);
        out_att[count].status = (AttendanceStatus)sqlite3_column_int(stmt, 4);
        strncpy(out_att[count].notes, (const char *)sqlite3_column_text(stmt, 5), sizeof(out_att[count].notes) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_save_attendance(int student_id, const char *date, AttendanceStatus status, const char *notes) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO attendance (student_id, date, status, notes) VALUES (?, ?, ?, ?) "
                      "ON CONFLICT(student_id, date) DO UPDATE SET status=excluded.status, notes=excluded.notes";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_text(stmt, 2, date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, (int)status);
    sqlite3_bind_text(stmt, 4, notes, -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Capaian Pembelajaran (CP)
bool db_get_cp(CapaianPembelajaran *out_cp, int max_cp, const char *search, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT id, code, description, subject FROM cp WHERE 1=1";
    if (search && strlen(search) > 0) {
        strcat(sql, " AND (code LIKE ? OR description LIKE ? OR subject LIKE ?)");
    }
    strcat(sql, " ORDER BY code ASC");

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    if (search && strlen(search) > 0) {
        char like_pattern[128];
        snprintf(like_pattern, sizeof(like_pattern), "%%%s%%", search);
        sqlite3_bind_text(stmt, 1, like_pattern, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, like_pattern, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, like_pattern, -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_cp) {
        out_cp[count].id = sqlite3_column_int(stmt, 0);
        strncpy(out_cp[count].code, (const char *)sqlite3_column_text(stmt, 1), sizeof(out_cp[count].code) - 1);
        strncpy(out_cp[count].description, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_cp[count].description) - 1);
        strncpy(out_cp[count].subject, (const char *)sqlite3_column_text(stmt, 3), sizeof(out_cp[count].subject) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_cp(const CapaianPembelajaran *cp) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO cp (code, description, subject) VALUES (?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, cp->code, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cp->description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cp->subject, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_cp(const CapaianPembelajaran *cp) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE cp SET code = ?, description = ?, subject = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, cp->code, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cp->description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cp->subject, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, cp->id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_cp(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM cp WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Tujuan Pembelajaran (TP)
bool db_get_tp(TujuanPembelajaran *out_tp, int max_tp, int cp_id, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT t.id, t.cp_id, c.code, t.code, t.description "
                    "FROM tp t JOIN cp c ON t.cp_id = c.id WHERE 1=1";
    if (cp_id > 0) {
        strcat(sql, " AND t.cp_id = ?");
    }
    strcat(sql, " ORDER BY t.code ASC");

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    if (cp_id > 0) {
        sqlite3_bind_int(stmt, 1, cp_id);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_tp) {
        out_tp[count].id = sqlite3_column_int(stmt, 0);
        out_tp[count].cp_id = sqlite3_column_int(stmt, 1);
        strncpy(out_tp[count].cp_code, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_tp[count].cp_code) - 1);
        strncpy(out_tp[count].code, (const char *)sqlite3_column_text(stmt, 3), sizeof(out_tp[count].code) - 1);
        strncpy(out_tp[count].description, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_tp[count].description) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_tp(const TujuanPembelajaran *tp) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO tp (cp_id, code, description) VALUES (?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, tp->cp_id);
    sqlite3_bind_text(stmt, 2, tp->code, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, tp->description, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_tp(const TujuanPembelajaran *tp) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE tp SET cp_id = ?, code = ?, description = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, tp->cp_id);
    sqlite3_bind_text(stmt, 2, tp->code, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, tp->description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, tp->id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_tp(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM tp WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Alur Tujuan Pembelajaran (ATP)
bool db_get_atp(AlurTujuanPembelajaran *out_atp, int max_atp, int tp_id, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT a.id, a.tp_id, t.code, a.order_num, a.description "
                    "FROM atp a JOIN tp t ON a.tp_id = t.id WHERE 1=1";
    if (tp_id > 0) {
        strcat(sql, " AND a.tp_id = ?");
    }
    strcat(sql, " ORDER BY a.order_num ASC");

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    if (tp_id > 0) {
        sqlite3_bind_int(stmt, 1, tp_id);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_atp) {
        out_atp[count].id = sqlite3_column_int(stmt, 0);
        out_atp[count].tp_id = sqlite3_column_int(stmt, 1);
        strncpy(out_atp[count].tp_code, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_atp[count].tp_code) - 1);
        out_atp[count].order_num = sqlite3_column_int(stmt, 3);
        strncpy(out_atp[count].description, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_atp[count].description) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_atp(const AlurTujuanPembelajaran *atp) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO atp (tp_id, order_num, description) VALUES (?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, atp->tp_id);
    sqlite3_bind_int(stmt, 2, atp->order_num);
    sqlite3_bind_text(stmt, 3, atp->description, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_atp(const AlurTujuanPembelajaran *atp) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE atp SET tp_id = ?, order_num = ?, description = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, atp->tp_id);
    sqlite3_bind_int(stmt, 2, atp->order_num);
    sqlite3_bind_text(stmt, 3, atp->description, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, atp->id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_atp(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM atp WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Daily Journal
bool db_get_journal(DailyJournal *out_journal, int max_journal, const char *date, int class_id, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT j.id, j.teacher_id, t.name, j.class_id, c.name, j.date, j.activity, j.notes "
                    "FROM daily_journal j JOIN teachers t ON j.teacher_id = t.id JOIN classes c ON j.class_id = c.id WHERE 1=1";
    if (date && strlen(date) > 0) {
        strcat(sql, " AND j.date = ?");
    }
    if (class_id > 0) {
        strcat(sql, " AND j.class_id = ?");
    }
    strcat(sql, " ORDER BY j.date DESC, j.id DESC");

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    int bind_idx = 1;
    if (date && strlen(date) > 0) {
        sqlite3_bind_text(stmt, bind_idx++, date, -1, SQLITE_TRANSIENT);
    }
    if (class_id > 0) {
        sqlite3_bind_int(stmt, bind_idx++, class_id);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_journal) {
        out_journal[count].id = sqlite3_column_int(stmt, 0);
        out_journal[count].teacher_id = sqlite3_column_int(stmt, 1);
        strncpy(out_journal[count].teacher_name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_journal[count].teacher_name) - 1);
        out_journal[count].class_id = sqlite3_column_int(stmt, 3);
        strncpy(out_journal[count].class_name, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_journal[count].class_name) - 1);
        strncpy(out_journal[count].date, (const char *)sqlite3_column_text(stmt, 5), sizeof(out_journal[count].date) - 1);
        strncpy(out_journal[count].activity, (const char *)sqlite3_column_text(stmt, 6), sizeof(out_journal[count].activity) - 1);
        strncpy(out_journal[count].notes, (const char *)sqlite3_column_text(stmt, 7), sizeof(out_journal[count].notes) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_create_journal(const DailyJournal *journal) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO daily_journal (teacher_id, class_id, date, activity, notes) VALUES (?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, journal->teacher_id);
    sqlite3_bind_int(stmt, 2, journal->class_id);
    sqlite3_bind_text(stmt, 3, journal->date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, journal->activity, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, journal->notes, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_update_journal(const DailyJournal *journal) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE daily_journal SET teacher_id = ?, class_id = ?, date = ?, activity = ?, notes = ? WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, journal->teacher_id);
    sqlite3_bind_int(stmt, 2, journal->class_id);
    sqlite3_bind_text(stmt, 3, journal->date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, journal->activity, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, journal->notes, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, journal->id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_journal(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM daily_journal WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Grades Daily
bool db_get_daily_grades(DailyGrade *out_grades, int max_grades, int student_id, int tp_id, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT g.id, g.student_id, s.name, g.tp_id, t.code, g.score, g.date, g.notes "
                    "FROM grades_daily g JOIN students s ON g.student_id = s.id JOIN tp t ON g.tp_id = t.id WHERE 1=1";
    if (student_id > 0) {
        strcat(sql, " AND g.student_id = ?");
    }
    if (tp_id > 0) {
        strcat(sql, " AND g.tp_id = ?");
    }
    strcat(sql, " ORDER BY g.date DESC");

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    int bind_idx = 1;
    if (student_id > 0) {
        sqlite3_bind_int(stmt, bind_idx++, student_id);
    }
    if (tp_id > 0) {
        sqlite3_bind_int(stmt, bind_idx++, tp_id);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_grades) {
        out_grades[count].id = sqlite3_column_int(stmt, 0);
        out_grades[count].student_id = sqlite3_column_int(stmt, 1);
        strncpy(out_grades[count].student_name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_grades[count].student_name) - 1);
        out_grades[count].tp_id = sqlite3_column_int(stmt, 3);
        strncpy(out_grades[count].tp_code, (const char *)sqlite3_column_text(stmt, 4), sizeof(out_grades[count].tp_code) - 1);
        out_grades[count].score = sqlite3_column_double(stmt, 5);
        strncpy(out_grades[count].date, (const char *)sqlite3_column_text(stmt, 6), sizeof(out_grades[count].date) - 1);
        strncpy(out_grades[count].notes, (const char *)sqlite3_column_text(stmt, 7), sizeof(out_grades[count].notes) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_save_daily_grade(int student_id, int tp_id, double score, const char *date, const char *notes) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO grades_daily (student_id, tp_id, score, date, notes) VALUES (?, ?, ?, ?, ?) "
                      "ON CONFLICT(student_id, tp_id, date) DO UPDATE SET score=excluded.score, notes=excluded.notes";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_int(stmt, 2, tp_id);
    sqlite3_bind_double(stmt, 3, score);
    sqlite3_bind_text(stmt, 4, date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, notes, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_daily_grade(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM grades_daily WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Grades Exam
bool db_get_exam_grades(ExamGrade *out_grades, int max_grades, int student_id, const char *subject, int *out_count) {
    sqlite3_stmt *stmt;
    char sql[512] = "SELECT g.id, g.student_id, s.name, g.subject, g.exam_type, g.score, g.date "
                    "FROM grades_exam g JOIN students s ON g.student_id = s.id WHERE 1=1";
    if (student_id > 0) {
        strcat(sql, " AND g.student_id = ?");
    }
    if (subject && strlen(subject) > 0) {
        strcat(sql, " AND g.subject = ?");
    }
    strcat(sql, " ORDER BY g.date DESC");

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    int bind_idx = 1;
    if (student_id > 0) {
        sqlite3_bind_int(stmt, bind_idx++, student_id);
    }
    if (subject && strlen(subject) > 0) {
        sqlite3_bind_text(stmt, bind_idx++, subject, -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max_grades) {
        out_grades[count].id = sqlite3_column_int(stmt, 0);
        out_grades[count].student_id = sqlite3_column_int(stmt, 1);
        strncpy(out_grades[count].student_name, (const char *)sqlite3_column_text(stmt, 2), sizeof(out_grades[count].student_name) - 1);
        strncpy(out_grades[count].subject, (const char *)sqlite3_column_text(stmt, 3), sizeof(out_grades[count].subject) - 1);
        out_grades[count].exam_type = (ExamType)sqlite3_column_int(stmt, 4);
        out_grades[count].score = sqlite3_column_double(stmt, 5);
        strncpy(out_grades[count].date, (const char *)sqlite3_column_text(stmt, 6), sizeof(out_grades[count].date) - 1);
        count++;
    }
    *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool db_save_exam_grade(int student_id, const char *subject, ExamType exam_type, double score, const char *date) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO grades_exam (student_id, subject, exam_type, score, date) VALUES (?, ?, ?, ?, ?) "
                      "ON CONFLICT(student_id, subject, exam_type) DO UPDATE SET score=excluded.score, date=excluded.date";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_text(stmt, 2, subject, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, (int)exam_type);
    sqlite3_bind_double(stmt, 4, score);
    sqlite3_bind_text(stmt, 5, date, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_delete_exam_grade(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM grades_exam WHERE id = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Dashboard Statistics
bool db_get_dashboard_stats(DashboardStats *out_stats) {
    sqlite3_stmt *stmt;
    memset(out_stats, 0, sizeof(DashboardStats));

    // Students Count
    if (sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM students WHERE status='Aktif'", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) out_stats->total_students = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    // Teachers Count
    if (sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM teachers WHERE status='Aktif'", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) out_stats->total_teachers = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    // Classes Count
    if (sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM classes", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) out_stats->total_classes = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    // Users Count
    if (sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM users", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) out_stats->total_users = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Attendance rate today
    // Let's get today's date formatted as YYYY-MM-DD in C or just query
    // To make it simple we check count of active students vs count of present today
    char today[20];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(today, sizeof(today), "%Y-%m-%d", tm_info);

    const char *att_sql = "SELECT "
                          "(SELECT COUNT(*) FROM attendance WHERE date = ? AND status = 0) * 100.0 / "
                          "NULLIF((SELECT COUNT(*) FROM students WHERE status='Aktif'), 0)";
    if (sqlite3_prepare_v2(g_db, att_sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, today, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                out_stats->attendance_rate_today = sqlite3_column_double(stmt, 0);
            } else {
                out_stats->attendance_rate_today = 0.0;
            }
        }
        sqlite3_finalize(stmt);
    }

    return true;
}

// Settings Persistence Functions
bool db_get_setting(const char *key, const char *default_val, char *out_val, int max_len) {
    if (!g_db || !key || !out_val || max_len <= 0) return false;
    
    // Set default value first
    if (default_val) {
        strncpy(out_val, default_val, max_len - 1);
        out_val[max_len - 1] = '\0';
    } else {
        out_val[0] = '\0';
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT value FROM settings WHERE key = ?";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *val = (const char *)sqlite3_column_text(stmt, 0);
        if (val) {
            strncpy(out_val, val, max_len - 1);
            out_val[max_len - 1] = '\0';
        }
    }
    sqlite3_finalize(stmt);
    return true;
}

bool db_set_setting(const char *key, const char *val) {
    if (!g_db || !key || !val) return false;
    
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO settings (key, value) VALUES (?, ?) "
                      "ON CONFLICT(key) DO UPDATE SET value = excluded.value";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, val, -1, SQLITE_TRANSIENT);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool db_get_school_settings(SchoolSettings *out_settings) {
    if (!out_settings) return false;
    db_get_setting("school_name", "SMA Negeri 1 Enterprise", out_settings->school_name, sizeof(out_settings->school_name));
    db_get_setting("school_npsn", "20199482", out_settings->school_npsn, sizeof(out_settings->school_npsn));
    db_get_setting("school_address", "Jl. Edukasi Utama No. 45, Jakarta Pusat", out_settings->school_address, sizeof(out_settings->school_address));
    db_get_setting("principal_name", "Dr. H. Ahmad Fauzi, M.Pd.", out_settings->principal_name, sizeof(out_settings->principal_name));
    db_get_setting("principal_nip", "19780512 200312 1 002", out_settings->principal_nip, sizeof(out_settings->principal_nip));
    db_get_setting("academic_year", "2025/2026", out_settings->academic_year, sizeof(out_settings->academic_year));
    db_get_setting("semester", "Ganjil", out_settings->semester, sizeof(out_settings->semester));
    return true;
}

bool db_save_school_settings(const SchoolSettings *settings) {
    if (!settings) return false;
    db_set_setting("school_name", settings->school_name);
    db_set_setting("school_npsn", settings->school_npsn);
    db_set_setting("school_address", settings->school_address);
    db_set_setting("principal_name", settings->principal_name);
    db_set_setting("principal_nip", settings->principal_nip);
    db_set_setting("academic_year", settings->academic_year);
    db_set_setting("semester", settings->semester);
    return true;
}
