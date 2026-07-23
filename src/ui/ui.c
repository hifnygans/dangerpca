#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include "ui.h"
#include "../database/db.h"
#include "../services/services.h"
#include "../models/models.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// FontAwesome solid icons macros in UTF-8
#define ICON_DASHBOARD   "\xef\x8f\xbd"
#define ICON_STUDENTS    "\xef\x83\x80"
#define ICON_TEACHERS    "\xef\x94\x9b"
#define ICON_CLASSES     "\xef\x95\x89"
#define ICON_ATTENDANCE  "\xef\x89\xb2"
#define ICON_ACADEMIC    "\xef\x95\x92"
#define ICON_JOURNAL     "\xef\x81\x84"
#define ICON_GRADES      "\xef\x97\x80"
#define ICON_USERS       "\xef\x93\xbe"
#define ICON_BACKUP      "\xef\x87\x80"
#define ICON_SETTINGS    "\xef\x80\x93"
#define ICON_ABOUT       "\xef\x81\x99"
#define ICON_LOGOUT      "\xef\x8b\xb5"
#define ICON_THEME       "\xef\x81\x82"
#define ICON_EDIT        "\xef\x81\x84"
#define ICON_TRASH       "\xef\x8b\xad"
#define ICON_SEARCH      "\xef\x80\x82"
#define ICON_USER_LOGIN  "\xef\x80\x87"
#define ICON_LOCK_LOGIN  "\xef\x80\xa3"
#define ICON_RTE_BOLD        "\xef\x80\xb2"
#define ICON_RTE_ITALIC      "\xef\x80\xb3"
#define ICON_RTE_UNDERLINE   "\xef\x83\x8d"
#define ICON_RTE_STRIKE      "\xef\x83\x8c"
#define ICON_RTE_HEADING     "\xef\x81\xa7"
#define ICON_RTE_LIST_UL     "\xef\x80\xba"
#define ICON_RTE_LIST_OL     "\xef\x80\xbb"
#define ICON_RTE_CHECK       "\xef\x85\x8a"
#define ICON_RTE_QUOTE       "\xef\x84\x8d"
#define ICON_RTE_CODE        "\xef\x84\xa1"
#define ICON_RTE_ALIGN_LEFT  "\xef\x80\xb6"
#define ICON_RTE_ALIGN_CENTER "\xef\x80\xb7"
#define ICON_RTE_ALIGN_RIGHT "\xef\x80\xb5"
#define ICON_RTE_ERASER      "\xef\x84\xad"
#define ICON_RTE_TEMPLATE    "\xef\x85\x9c"
#define ICON_RTE_PALETTE     "\xef\x94\xbf"
#define ICON_RTE_EYE         "\xef\x81\xae"


// Global State
static SidebarMenu g_current_menu = MENU_DASHBOARD;
static bool g_logged_in = false;
static User g_current_user = {0};
static SchoolSettings g_school_settings = {0};

// Login Inputs
static char g_login_user[50] = "";
static char g_login_pass[50] = "";

// Caches & Counts
static DashboardStats g_stats = {0};
static Student g_students[500];
static int g_students_count = 0;
static Teacher g_teachers[200];
static int g_teachers_count = 0;
static ClassEntity g_classes[100];
static int g_classes_count = 0;
static Attendance g_attendance[500];
static int g_attendance_count = 0;
static CapaianPembelajaran g_cp[200];
static int g_cp_count = 0;
static TujuanPembelajaran g_tp[300];
static int g_tp_count = 0;
static AlurTujuanPembelajaran g_atp[300];
static int g_atp_count = 0;
static DailyJournal g_journals[200];
static int g_journals_count = 0;
static DailyGrade g_daily_grades[500];
static int g_daily_grades_count = 0;
static ExamGrade g_exam_grades[500];
static int g_exam_grades_count = 0;
static User g_users[100];
static int g_users_count = 0;

// Search & Filter state
static char g_search_query[100] = "";
static int g_class_filter_id = 0;
static char g_attendance_date[20] = "";
static int g_selected_class_idx = 0;
static int g_selected_student_idx = 0;
static int g_selected_tp_idx = 0;
static char g_subject_filter[100] = "Matematika";

// Theme colors (Global for UI & Rich Text Editor)
static bool g_light_mode = false;
static struct nk_color g_theme_text_header;
static struct nk_color g_theme_text_muted;
static struct nk_color g_theme_border;
static struct nk_color g_theme_primary;
static struct nk_color g_theme_accent;

// Form States (Editing / Adding)
static bool g_show_form = false;
static bool g_is_editing = false;
static int g_editing_id = 0;

// Temporary Form Buffers (Super Expanded to 64 KB for Professional Long-Form Journaling)
static char g_form_txt1[65536] = "";
static char g_form_txt2[65536] = "";
static char g_form_txt3[65536] = "";
static char g_form_txt4[65536] = "";
static char g_form_txt5[65536] = "";
static char g_form_txt6[65536] = "";
static int g_form_int1 = 0;
static double g_form_dbl1 = 0.0;
static int g_form_gender_idx = 0; // 0=L, 1=P
static int g_form_status_idx = 0;
static int g_form_role_idx = 0;

// Rich Text Editor State Trackers
static int g_rte_mode_journal_act = 0;  // 0=Edit, 1=Preview, 2=Split
static int g_rte_mode_journal_note = 0;
static int g_rte_mode_cp = 0;
static int g_rte_mode_tp = 0;
static int g_rte_mode_atp = 0;
static int g_rte_mode_student = 0;

// Journal Detail Viewer State
static bool g_show_journal_detail = false;
static DailyJournal g_detail_journal;

// Backup / Restore Buffers
static char g_backup_path[256] = "dangerpca_backup.db";
static char g_restore_path[256] = "dangerpca_backup.db";

// Notifications
static char g_notification_msg[128] = "";
static time_t g_notification_expiry = 0;
static bool g_notification_success = true;

static void show_notification(const char *msg, bool success) {
    strncpy(g_notification_msg, msg, sizeof(g_notification_msg) - 1);
    g_notification_expiry = time(NULL) + 3;
    g_notification_success = success;
}

static void get_today_date(char *buf, int max_len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, max_len, "%Y-%m-%d", tm_info);
}

// PROFESSIONAL RICH TEXT EDITOR ENGINE & PARSER
typedef enum {
    RTE_CMD_BOLD,
    RTE_CMD_ITALIC,
    RTE_CMD_UNDERLINE,
    RTE_CMD_STRIKE,
    RTE_CMD_H1,
    RTE_CMD_H2,
    RTE_CMD_H3,
    RTE_CMD_LIST_UL,
    RTE_CMD_LIST_OL,
    RTE_CMD_CHECKLIST,
    RTE_CMD_QUOTE,
    RTE_CMD_CODE,
    RTE_CMD_HR,
    RTE_CMD_COLOR_RED,
    RTE_CMD_COLOR_BLUE,
    RTE_CMD_COLOR_GREEN,
    RTE_CMD_COLOR_PURPLE,
    RTE_CMD_ALIGN_LEFT,
    RTE_CMD_ALIGN_CENTER,
    RTE_CMD_ALIGN_RIGHT,
    RTE_CMD_CLEAR,
    RTE_CMD_TMPL_JOURNAL,
    RTE_CMD_TMPL_CP
} RteCommand;

static void rte_apply_command(char *buffer, size_t max_size, RteCommand cmd) {
    if (!buffer || max_size == 0) return;

    if (cmd == RTE_CMD_TMPL_JOURNAL) {
        const char *tmpl = 
            "# JURNAL PEMBELAJARAN & AKTIVITAS KELAS\n"
            "## 1. Topik & Materi Pokok Utama:\n"
            "- <color:#3b82f6><b>Materi: Persamaan Kuadrat & Diskriminan D = b^2 - 4ac</b></color>\n"
            "- Pemecahan studi kasus & latihan soal mandiri\n"
            "\n"
            "## 2. Catatan Evaluasi & Kehadiran:\n"
            "[x] Seluruh siswa mengikuti pembelajaran dengan baik\n"
            "[x] Modul latihan halaman 45 diselesaikan\n"
            "[ ] Sesi remidial kelompok 3 dilakukan minggu depan\n"
            "\n"
            "> <b>Catatan Khusus Guru:</b> Siswa menunjukkan pemahaman yang sangat tinggi pada rumus diskriminan.\n"
            "\n"
            "---\n"
            "```\n"
            "// Formula Diskriminan\n"
            "double D = (b * b) - (4 * a * c);\n"
            "```";
        snprintf(buffer, max_size, "%s", tmpl);
        return;
    } else if (cmd == RTE_CMD_TMPL_CP) {
        const char *tmpl = 
            "# CAPAIAN PEMBELAJARAN (CP)\n"
            "## 1. Elemen Kompetensi Utama:\n"
            "- <color:#22c55e><b>Peserta didik mampu menganalisis & menyelesaikan permasalahan matematis</b></color>\n"
            "- Mengaplikasikan prinsip logika & pemodelan data\n"
            "\n"
            "## 2. Target Hasil Belajar:\n"
            "[x] Memahami konsep dasar kurikulum merdeka\n"
            "[x] Menyelesaikan minimal 5 indikator ketercapaian\n"
            "\n"
            "> <b>Kriteria Ketuntasan:</b> Nilai rata-rata kognitif minimal 75.";
        snprintf(buffer, max_size, "%s", tmpl);
        return;
    } else if (cmd == RTE_CMD_CLEAR) {
        buffer[0] = '\0';
        return;
    }

    const char *prefix = "";
    const char *sample = "Teks";
    const char *suffix = "";

    switch (cmd) {
        case RTE_CMD_BOLD: prefix = "<b>"; sample = "Teks Tebal"; suffix = "</b>"; break;
        case RTE_CMD_ITALIC: prefix = "<i>"; sample = "Teks Miring"; suffix = "</i>"; break;
        case RTE_CMD_UNDERLINE: prefix = "<u>"; sample = "Teks Garis Bawah"; suffix = "</u>"; break;
        case RTE_CMD_STRIKE: prefix = "<s>"; sample = "Teks Coret"; suffix = "</s>"; break;
        case RTE_CMD_H1: prefix = "\n# "; sample = "Judul Topik Utama"; suffix = "\n"; break;
        case RTE_CMD_H2: prefix = "\n## "; sample = "Sub-Judul Pembahasan"; suffix = "\n"; break;
        case RTE_CMD_H3: prefix = "\n### "; sample = "Sub Poin Materi"; suffix = "\n"; break;
        case RTE_CMD_LIST_UL: prefix = "\n- "; sample = "Poin Catatan Utama\n- Poin Catatan Tambahan"; suffix = "\n"; break;
        case RTE_CMD_LIST_OL: prefix = "\n1. "; sample = "Langkah Pertama\n2. Langkah Kedua"; suffix = "\n"; break;
        case RTE_CMD_CHECKLIST: prefix = "\n[x] "; sample = "Tugas Selesai\n[ ] Tugas Belum Selesai"; suffix = "\n"; break;
        case RTE_CMD_QUOTE: prefix = "\n> "; sample = "Catatan Khusus: Penting untuk diperhatikan"; suffix = "\n"; break;
        case RTE_CMD_CODE: prefix = "\n```\n"; sample = "// Masukkan Kode atau Formula di sini\n"; suffix = "```\n"; break;
        case RTE_CMD_HR: prefix = "\n---\n"; sample = ""; suffix = ""; break;
        case RTE_CMD_COLOR_RED: prefix = "<color:#ef4444>"; sample = "Teks Warna Merah"; suffix = "</color>"; break;
        case RTE_CMD_COLOR_BLUE: prefix = "<color:#3b82f6>"; sample = "Teks Warna Biru"; suffix = "</color>"; break;
        case RTE_CMD_COLOR_GREEN: prefix = "<color:#22c55e>"; sample = "Teks Warna Hijau"; suffix = "</color>"; break;
        case RTE_CMD_COLOR_PURPLE: prefix = "<color:#a855f7>"; sample = "Teks Warna Ungu"; suffix = "</color>"; break;
        case RTE_CMD_ALIGN_LEFT: prefix = "\n<align:left>"; sample = "Teks Rata Kiri"; suffix = "</align>\n"; break;
        case RTE_CMD_ALIGN_CENTER: prefix = "\n<align:center>"; sample = "Teks Rata Tengah"; suffix = "</align>\n"; break;
        case RTE_CMD_ALIGN_RIGHT: prefix = "\n<align:right>"; sample = "Teks Rata Kanan"; suffix = "</align>\n"; break;
        default: break;
    }

    size_t len = strlen(buffer);
    size_t p_len = strlen(prefix);
    size_t s_len = strlen(sample);
    size_t suf_len = strlen(suffix);

    if (len + p_len + s_len + suf_len + 1 < max_size) {
        strcat(buffer, prefix);
        strcat(buffer, sample);
        strcat(buffer, suffix);
    }
}

static void parse_and_render_line_tags(struct nk_context *ctx, const char *line_str, struct nk_color default_color) {
    if (!line_str || strlen(line_str) == 0) return;

    // Custom inline color tag parser <color:#HEX>
    const char *color_tag = strstr(line_str, "<color:#");
    if (color_tag) {
        char hex_code[8] = {0};
        if (strlen(color_tag + 8) >= 6) {
            strncpy(hex_code, color_tag + 8, 6);
            hex_code[6] = '\0';
            
            unsigned int r = 255, g = 255, b = 255;
            if (sscanf(hex_code, "%02x%02x%02x", &r, &g, &b) == 3) {
                const char *tag_end = strstr(color_tag, "</color>");
                const char *text_start = strchr(color_tag, '>') ? strchr(color_tag, '>') + 1 : color_tag + 15;
                
                if (tag_end && text_start < tag_end) {
                    char inner_text[256];
                    size_t t_len = tag_end - text_start;
                    if (t_len >= sizeof(inner_text)) t_len = sizeof(inner_text) - 1;
                    memcpy(inner_text, text_start, t_len);
                    inner_text[t_len] = '\0';

                    char clean_inner[256];
                    int c_idx = 0;
                    for (size_t i = 0; i < strlen(inner_text); i++) {
                        if (inner_text[i] == '<') {
                            while (i < strlen(inner_text) && inner_text[i] != '>') i++;
                        } else {
                            if (c_idx < (int)sizeof(clean_inner) - 1) clean_inner[c_idx++] = inner_text[i];
                        }
                    }
                    clean_inner[c_idx] = '\0';

                    nk_label_colored(ctx, clean_inner, NK_TEXT_LEFT, nk_rgb(r, g, b));
                    return;
                }
            }
        }
    }

    // Strip inline html tags for clean visual rendering
    char clean_line[512];
    int c_idx = 0;
    for (size_t i = 0; i < strlen(line_str); i++) {
        if (line_str[i] == '<') {
            while (i < strlen(line_str) && line_str[i] != '>') i++;
        } else if (line_str[i] == '*' && i + 1 < strlen(line_str) && line_str[i+1] == '*') {
            i++;
        } else {
            if (c_idx < (int)sizeof(clean_line) - 1) clean_line[c_idx++] = line_str[i];
        }
    }
    clean_line[c_idx] = '\0';

    if (strlen(clean_line) > 0) {
        nk_label_colored(ctx, clean_line, NK_TEXT_LEFT, default_color);
    }
}

static void render_rich_text_content(struct nk_context *ctx, const char *text, const char *group_id_prefix) {
    if (!text || strlen(text) == 0) return;

    char line_buf[512];
    const char *p = text;
    int line_idx = 0;
    bool in_code_block = false;

    while (*p) {
        const char *line_start = p;
        while (*p && *p != '\n') p++;
        
        size_t line_len = p - line_start;
        if (line_len >= sizeof(line_buf)) line_len = sizeof(line_buf) - 1;
        memcpy(line_buf, line_start, line_len);
        line_buf[line_len] = '\0';
        
        if (*p == '\n') p++;

        if (line_len > 0 && line_buf[line_len - 1] == '\r') {
            line_buf[line_len - 1] = '\0';
            line_len--;
        }

        line_idx++;

        if (line_len == 0) {
            nk_layout_row_dynamic(ctx, 6, 1);
            nk_spacing(ctx, 1);
            continue;
        }

        if (strncmp(line_buf, "```", 3) == 0) {
            in_code_block = !in_code_block;
            nk_layout_row_dynamic(ctx, 2, 1);
            nk_rule_horizontal(ctx, nk_rgb(234, 179, 8), 1);
            continue;
        }

        if (in_code_block) {
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label_colored(ctx, line_buf, NK_TEXT_LEFT, nk_rgb(234, 179, 8));
            continue;
        }

        if (strcmp(line_buf, "---") == 0 || strcmp(line_buf, "***") == 0) {
            nk_layout_row_dynamic(ctx, 6, 1);
            nk_rule_horizontal(ctx, g_theme_border, 1);
            continue;
        }

        // Header 1: "# "
        if (strncmp(line_buf, "# ", 2) == 0) {
            nk_layout_row_dynamic(ctx, 28, 1);
            parse_and_render_line_tags(ctx, line_buf + 2, g_theme_text_header);
            nk_layout_row_dynamic(ctx, 2, 1);
            nk_rule_horizontal(ctx, g_theme_primary, 1);
        }
        // Header 2: "## "
        else if (strncmp(line_buf, "## ", 3) == 0) {
            nk_layout_row_dynamic(ctx, 24, 1);
            parse_and_render_line_tags(ctx, line_buf + 3, g_theme_primary);
        }
        // Header 3: "### "
        else if (strncmp(line_buf, "### ", 4) == 0) {
            nk_layout_row_dynamic(ctx, 22, 1);
            parse_and_render_line_tags(ctx, line_buf + 4, g_theme_accent);
        }
        // Bullet item: "- " or "* "
        else if (strncmp(line_buf, "- ", 2) == 0 || strncmp(line_buf, "* ", 2) == 0) {
            nk_layout_row_template_begin(ctx, 22);
            nk_layout_row_template_push_static(ctx, 20);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_end(ctx);
            nk_label_colored(ctx, "•", NK_TEXT_LEFT, g_theme_primary);
            parse_and_render_line_tags(ctx, line_buf + 2, g_theme_text_header);
        }
        // Numbered list item: e.g. "1. "
        else if (line_len >= 3 && line_buf[0] >= '0' && line_buf[0] <= '9' && line_buf[1] == '.' && line_buf[2] == ' ') {
            nk_layout_row_template_begin(ctx, 22);
            nk_layout_row_template_push_static(ctx, 24);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_end(ctx);
            char num_str[8];
            snprintf(num_str, sizeof(num_str), "%c.", line_buf[0]);
            nk_label_colored(ctx, num_str, NK_TEXT_LEFT, g_theme_primary);
            parse_and_render_line_tags(ctx, line_buf + 3, g_theme_text_header);
        }
        // Checkboxes: "[x] " or "[X] "
        else if (strncmp(line_buf, "[x] ", 4) == 0 || strncmp(line_buf, "[X] ", 4) == 0) {
            nk_layout_row_template_begin(ctx, 22);
            nk_layout_row_template_push_static(ctx, 30);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_end(ctx);
            nk_label_colored(ctx, "[✓]", NK_TEXT_LEFT, nk_rgb(34, 197, 94));
            parse_and_render_line_tags(ctx, line_buf + 4, g_theme_text_header);
        }
        // Unchecked: "[ ] "
        else if (strncmp(line_buf, "[ ] ", 4) == 0) {
            nk_layout_row_template_begin(ctx, 22);
            nk_layout_row_template_push_static(ctx, 30);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_end(ctx);
            nk_label_colored(ctx, "[  ]", NK_TEXT_LEFT, g_theme_text_muted);
            parse_and_render_line_tags(ctx, line_buf + 4, g_theme_text_muted);
        }
        // Quote / Callout: "> "
        else if (strncmp(line_buf, "> ", 2) == 0) {
            nk_layout_row_dynamic(ctx, 28, 1);
            char q_grp[64];
            snprintf(q_grp, sizeof(q_grp), "rte_q_%s_%d", group_id_prefix, line_idx);
            if (nk_group_begin(ctx, q_grp, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 20, 1);
                parse_and_render_line_tags(ctx, line_buf + 2, g_theme_accent);
                nk_group_end(ctx);
            }
        }
        // Inline code line
        else if (line_buf[0] == '`' || strchr(line_buf, '`') != NULL) {
            nk_layout_row_dynamic(ctx, 22, 1);
            parse_and_render_line_tags(ctx, line_buf, nk_rgb(234, 179, 8));
        }
        // Plain line
        else {
            nk_layout_row_dynamic(ctx, 22, 1);
            parse_and_render_line_tags(ctx, line_buf, g_theme_text_header);
        }
    }
}

static void draw_rich_text_editor(struct nk_context *ctx, const char *label, char *buffer, size_t max_size, float edit_height, int *mode, const char *group_id) {
    // 1. Header Title & Mode Switcher
    nk_layout_row_template_begin(ctx, 28);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_push_static(ctx, 330);
    nk_layout_row_template_end(ctx);

    char header_label[128];
    snprintf(header_label, sizeof(header_label), "%s  [WYSIWYG Rich Text Editor]", label);
    nk_label_colored(ctx, header_label, NK_TEXT_LEFT, g_theme_text_header);

    char mode_grp[64];
    snprintf(mode_grp, sizeof(mode_grp), "rte_hdr_%s", group_id);
    if (nk_group_begin(ctx, mode_grp, NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 24, 3);
        if (nk_button_label(ctx, *mode == 0 ? "[ Visual Canvas ]" : "Visual Canvas")) *mode = 0;
        if (nk_button_label(ctx, *mode == 1 ? "[ Raw Teks ]" : "Raw Teks")) *mode = 1;
        if (nk_button_label(ctx, *mode == 2 ? "[ Split View ]" : "Split View")) *mode = 2;
        nk_group_end(ctx);
    }

    // 2. Professional Formatting Toolbar (Row 1: Text Style & Structure)
    nk_layout_row_dynamic(ctx, 28, 13);
    if (nk_button_label(ctx, ICON_RTE_BOLD " B")) rte_apply_command(buffer, max_size, RTE_CMD_BOLD);
    if (nk_button_label(ctx, ICON_RTE_ITALIC " I")) rte_apply_command(buffer, max_size, RTE_CMD_ITALIC);
    if (nk_button_label(ctx, ICON_RTE_UNDERLINE " U")) rte_apply_command(buffer, max_size, RTE_CMD_UNDERLINE);
    if (nk_button_label(ctx, ICON_RTE_STRIKE " S")) rte_apply_command(buffer, max_size, RTE_CMD_STRIKE);
    if (nk_button_label(ctx, ICON_RTE_HEADING " H1")) rte_apply_command(buffer, max_size, RTE_CMD_H1);
    if (nk_button_label(ctx, ICON_RTE_HEADING " H2")) rte_apply_command(buffer, max_size, RTE_CMD_H2);
    if (nk_button_label(ctx, ICON_RTE_HEADING " H3")) rte_apply_command(buffer, max_size, RTE_CMD_H3);
    if (nk_button_label(ctx, ICON_RTE_LIST_UL " Poin")) rte_apply_command(buffer, max_size, RTE_CMD_LIST_UL);
    if (nk_button_label(ctx, ICON_RTE_LIST_OL " Angka")) rte_apply_command(buffer, max_size, RTE_CMD_LIST_OL);
    if (nk_button_label(ctx, ICON_RTE_CHECK " Check")) rte_apply_command(buffer, max_size, RTE_CMD_CHECKLIST);
    if (nk_button_label(ctx, ICON_RTE_QUOTE " Quote")) rte_apply_command(buffer, max_size, RTE_CMD_QUOTE);
    if (nk_button_label(ctx, ICON_RTE_CODE " Code")) rte_apply_command(buffer, max_size, RTE_CMD_CODE);
    if (nk_button_label(ctx, "― HR")) rte_apply_command(buffer, max_size, RTE_CMD_HR);

    // Row 2: Colors, Alignment & Tools
    nk_layout_row_dynamic(ctx, 28, 8);
    if (nk_button_label(ctx, "🔴 Merah")) rte_apply_command(buffer, max_size, RTE_CMD_COLOR_RED);
    if (nk_button_label(ctx, "🔵 Biru")) rte_apply_command(buffer, max_size, RTE_CMD_COLOR_BLUE);
    if (nk_button_label(ctx, "🟢 Hijau")) rte_apply_command(buffer, max_size, RTE_CMD_COLOR_GREEN);
    if (nk_button_label(ctx, "🟣 Ungu")) rte_apply_command(buffer, max_size, RTE_CMD_COLOR_PURPLE);
    if (nk_button_label(ctx, ICON_RTE_ALIGN_LEFT " Kiri")) rte_apply_command(buffer, max_size, RTE_CMD_ALIGN_LEFT);
    if (nk_button_label(ctx, ICON_RTE_ALIGN_CENTER " Tengah")) rte_apply_command(buffer, max_size, RTE_CMD_ALIGN_CENTER);
    if (nk_button_label(ctx, ICON_RTE_ERASER " Reset")) rte_apply_command(buffer, max_size, RTE_CMD_CLEAR);
    if (nk_button_label(ctx, ICON_RTE_TEMPLATE " Template")) rte_apply_command(buffer, max_size, RTE_CMD_TMPL_JOURNAL);

    // 3. Main Workspace Area according to selected mode
    if (*mode == 0) {
        nk_layout_row_dynamic(ctx, edit_height, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, buffer, max_size, nk_filter_default);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label_colored(ctx, "PRATINJAU VISUAL KANVAS WYSIWYG:", NK_TEXT_LEFT, g_theme_text_muted);

        char preview_grp[64];
        snprintf(preview_grp, sizeof(preview_grp), "rte_vis_%s", group_id);
        nk_layout_row_dynamic(ctx, edit_height + 40, 1);
        if (nk_group_begin(ctx, preview_grp, NK_WINDOW_BORDER)) {
            if (strlen(buffer) == 0) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label_colored(ctx, "(Kanvas visual kosong. Ketik teks atau gunakan tombol toolbar format di atas...)", NK_TEXT_LEFT, g_theme_text_muted);
            } else {
                render_rich_text_content(ctx, buffer, group_id);
            }
            nk_group_end(ctx);
        }
    } else if (*mode == 1) {
        nk_layout_row_dynamic(ctx, edit_height + 60, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, buffer, max_size, nk_filter_default);
    } else {
        nk_layout_row_dynamic(ctx, edit_height + 60, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, buffer, max_size, nk_filter_default);

        char preview_grp[64];
        snprintf(preview_grp, sizeof(preview_grp), "rte_split_%s", group_id);
        if (nk_group_begin(ctx, preview_grp, NK_WINDOW_BORDER)) {
            if (strlen(buffer) == 0) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label_colored(ctx, "(Pratinjau langsung...)", NK_TEXT_LEFT, g_theme_text_muted);
            } else {
                render_rich_text_content(ctx, buffer, group_id);
            }
            nk_group_end(ctx);
        }
    }

    // 4. Status Bar Metrics: Words, Characters, Lines
    int char_count = (int)strlen(buffer);
    int word_count = 0;
    int line_count = 1;
    bool in_word = false;
    for (int i = 0; i < char_count; i++) {
        if (buffer[i] == '\n') line_count++;
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            word_count++;
        }
    }

    nk_layout_row_dynamic(ctx, 20, 1);
    char status_str[256];
    double pct = max_size > 0 ? ((double)char_count / (double)max_size * 100.0) : 0.0;
    snprintf(status_str, sizeof(status_str), "📄 Karakter: %d / %zu (%.1f%% Kapasitas Pro) | 📝 Kata: %d | 📑 Baris: %d", char_count, max_size, pct, word_count, line_count);
    nk_label_colored(ctx, status_str, NK_TEXT_RIGHT, g_theme_text_muted);
}

// Data loading manager
static void load_tab_data(SidebarMenu menu) {
    g_show_form = false;
    switch (menu) {
        case MENU_DASHBOARD:
            db_get_dashboard_stats(&g_stats);
            break;
        case MENU_STUDENTS:
            db_get_students(g_students, 500, g_search_query, g_class_filter_id, &g_students_count);
            db_get_classes(g_classes, 100, &g_classes_count);
            break;
        case MENU_TEACHERS:
            db_get_teachers(g_teachers, 200, g_search_query, &g_teachers_count);
            break;
        case MENU_CLASSES:
            db_get_classes(g_classes, 100, &g_classes_count);
            db_get_teachers(g_teachers, 200, NULL, &g_teachers_count);
            break;
        case MENU_ATTENDANCE:
            db_get_classes(g_classes, 100, &g_classes_count);
            if (g_classes_count > 0) {
                if (g_selected_class_idx >= g_classes_count) g_selected_class_idx = 0;
                db_get_attendance(g_attendance, 500, g_attendance_date, g_classes[g_selected_class_idx].id, &g_attendance_count);
            }
            break;
        case MENU_ACADEMIC:
            db_get_cp(g_cp, 200, g_search_query, &g_cp_count);
            db_get_tp(g_tp, 300, 0, &g_tp_count);
            db_get_atp(g_atp, 300, 0, &g_atp_count);
            break;
        case MENU_JOURNAL:
            db_get_journal(g_journals, 200, g_attendance_date, 0, &g_journals_count);
            db_get_teachers(g_teachers, 200, NULL, &g_teachers_count);
            db_get_classes(g_classes, 100, &g_classes_count);
            break;
        case MENU_GRADES:
            db_get_students(g_students, 500, NULL, 0, &g_students_count);
            db_get_tp(g_tp, 300, 0, &g_tp_count);
            if (g_students_count > 0) {
                if (g_selected_student_idx >= g_students_count) g_selected_student_idx = 0;
                db_get_daily_grades(g_daily_grades, 500, g_students[g_selected_student_idx].id, 0, &g_daily_grades_count);
                db_get_exam_grades(g_exam_grades, 500, g_students[g_selected_student_idx].id, NULL, &g_exam_grades_count);
            }
            break;
        case MENU_USERS:
            db_get_users(g_users, 100, &g_users_count);
            break;
        case MENU_BACKUP:
            break;
        case MENU_SETTINGS:
            db_get_school_settings(&g_school_settings);
            snprintf(g_form_txt1, sizeof(g_form_txt1), "%s", g_school_settings.school_name);
            snprintf(g_form_txt2, sizeof(g_form_txt2), "%s", g_school_settings.school_npsn);
            snprintf(g_form_txt3, sizeof(g_form_txt3), "%s", g_school_settings.school_address);
            snprintf(g_form_txt4, sizeof(g_form_txt4), "%s", g_school_settings.principal_name);
            snprintf(g_form_txt5, sizeof(g_form_txt5), "%s", g_school_settings.principal_nip);
            snprintf(g_form_txt6, sizeof(g_form_txt6), "%s", g_school_settings.academic_year);
            break;
        case MENU_ABOUT:
            break;
    }
}

static void set_material_dark_theme(struct nk_context *ctx) {
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(240, 240, 240, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(18, 18, 18, 255);
    table[NK_COLOR_HEADER] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_BORDER] = nk_rgba(40, 40, 40, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(30, 136, 229, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(21, 101, 192, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(45, 45, 45, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_SELECT] = nk_rgba(45, 45, 45, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(45, 45, 45, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(100, 181, 246, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(21, 101, 192, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_EDIT] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(240, 240, 240, 255);
    table[NK_COLOR_COMBO] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_CHART] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(233, 30, 99, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(18, 18, 18, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(50, 50, 50, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(33, 150, 243, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(30, 30, 30, 255);
    nk_style_from_table(ctx, table);
    
    ctx->style.window.rounding = 8.0f;
    ctx->style.button.rounding = 4.0f;
    ctx->style.edit.rounding = 4.0f;
    ctx->style.property.rounding = 4.0f;
    ctx->style.combo.rounding = 4.0f;
    ctx->style.tab.rounding = 4.0f;
    ctx->style.window.header.align = NK_HEADER_LEFT;
}

static void set_material_light_theme(struct nk_context *ctx) {
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(33, 33, 33, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(245, 245, 247, 255);
    table[NK_COLOR_HEADER] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_BORDER] = nk_rgba(224, 224, 224, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(92, 107, 192, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(48, 63, 159, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(224, 224, 224, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_SELECT] = nk_rgba(232, 234, 246, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(224, 224, 224, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(92, 107, 192, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(48, 63, 159, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_EDIT] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(33, 33, 33, 255);
    table[NK_COLOR_COMBO] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_CHART] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(233, 30, 99, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(240, 240, 240, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(200, 200, 200, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(180, 180, 180, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(63, 81, 181, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(240, 240, 240, 255);
    nk_style_from_table(ctx, table);
    
    ctx->style.window.rounding = 8.0f;
    ctx->style.button.rounding = 4.0f;
    ctx->style.edit.rounding = 4.0f;
    ctx->style.property.rounding = 4.0f;
    ctx->style.combo.rounding = 4.0f;
    ctx->style.tab.rounding = 4.0f;
    ctx->style.window.header.align = NK_HEADER_LEFT;
}

static void ui_apply_theme(struct nk_context *ctx) {
    if (g_light_mode) {
        set_material_light_theme(ctx);
        g_theme_text_header = nk_rgb(33, 33, 33);
        g_theme_text_muted = nk_rgb(117, 117, 117);
        g_theme_border = nk_rgb(224, 224, 224);
        g_theme_primary = nk_rgb(63, 81, 181);
        g_theme_accent = nk_rgb(233, 30, 99);
    } else {
        set_material_dark_theme(ctx);
        g_theme_text_header = nk_rgb(255, 255, 255);
        g_theme_text_muted = nk_rgb(160, 170, 185);
        g_theme_border = nk_rgb(45, 54, 72);
        g_theme_primary = nk_rgb(33, 150, 243);
        g_theme_accent = nk_rgb(255, 64, 129);
    }
}

void ui_init(struct nk_context *ctx) {
    ui_apply_theme(ctx);
    get_today_date(g_attendance_date, sizeof(g_attendance_date));
    load_tab_data(MENU_DASHBOARD);
}

void ui_cleanup(void) {
    // Release cached structures if dynamic resources were used
}

// DRAW HELPER: Draw Notification Banner
static void draw_notification(struct nk_context *ctx, int screen_width) {
    if (time(NULL) < g_notification_expiry) {
        struct nk_rect bounds = nk_rect(screen_width - 320, 20, 300, 50);
        struct nk_color bg = g_notification_success ? nk_rgb(39, 174, 96) : nk_rgb(192, 57, 43);
        
        if (nk_begin(ctx, "NotificationPopup", bounds, 
            NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER)) {
            
            // Draw background rectangle
            struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
            struct nk_rect win_rect = nk_window_get_bounds(ctx);
            nk_fill_rect(canvas, win_rect, 4.0f, bg);
            
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label_colored(ctx, g_notification_msg, NK_TEXT_CENTERED, nk_rgb(255, 255, 255));
        }
        nk_end(ctx);
    }
}

// SUB-SCREEN: Login
static void draw_login_screen(struct nk_context *ctx, int width, int height) {
    // Centered login card with responsive layout positioning
    struct nk_rect login_bounds = nk_rect(width / 2 - 200, height / 2 - 210, 400, 420);
    
    // We omit NK_WINDOW_TITLE, NK_WINDOW_MOVABLE, and NK_WINDOW_SCALABLE to make it look like a clean floating card
    if (nk_begin(ctx, "Login", login_bounds, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        
        // 1. Hero Icon & Branding
        nk_layout_row_dynamic(ctx, 15, 1); // Spacer
        
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label_colored(ctx, ICON_ACADEMIC "  DANGERPCA ERP", NK_TEXT_CENTERED, nk_rgb(33, 150, 243));
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label_colored(ctx, "Sistem Informasi & Manajemen Sekolah", NK_TEXT_CENTERED, g_theme_text_muted);
        
        nk_layout_row_dynamic(ctx, 15, 1); // Spacer
        nk_rule_horizontal(ctx, g_theme_border, 1);
        nk_layout_row_dynamic(ctx, 15, 1); // Spacer
        
        // 2. Input Fields with Professional styling
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, ICON_USER_LOGIN "  Username / NIP / NISN", NK_TEXT_LEFT, g_theme_text_header);
        
        nk_layout_row_dynamic(ctx, 35, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, g_login_user, sizeof(g_login_user), nk_filter_ascii);
        
        nk_layout_row_dynamic(ctx, 15, 1); // Spacer
        
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, ICON_LOCK_LOGIN "  Kata Sandi / Password", NK_TEXT_LEFT, g_theme_text_header);
        
        nk_layout_row_dynamic(ctx, 35, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, g_login_pass, sizeof(g_login_pass), nk_filter_ascii);
        
        nk_layout_row_dynamic(ctx, 30, 1); // Spacer
        
        // 3. Action Buttons & Feedback
        nk_layout_row_dynamic(ctx, 42, 1);
        if (nk_button_label(ctx, "MASUK KE DASHBOARD")) {
            User user;
            if (db_authenticate_user(g_login_user, g_login_pass, &user)) {
                g_logged_in = true;
                g_current_user = user;
                show_notification("Login sukses! Selamat datang.", true);
                load_tab_data(MENU_DASHBOARD);
            } else {
                show_notification("Username atau Password salah!", false);
            }
        }
        
        // 4. Subtle Footer inside card
        nk_layout_row_dynamic(ctx, 25, 1); // Spacer
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label_colored(ctx, "v2.0-stable | Database: school_erp.db", NK_TEXT_CENTERED, g_theme_text_muted);
    }
    nk_end(ctx);
}

// SUB-SCREEN: Sidebar
static void draw_sidebar(struct nk_context *ctx, int height) {
    if (nk_group_begin(ctx, "Sidebar", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 35, 1);
        nk_label(ctx, " DANGERPCA ERP ", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);
        nk_layout_row_dynamic(ctx, 10, 1); // Spacer

        const char *menus[] = {
            ICON_DASHBOARD "   Dashboard",
            ICON_STUDENTS "   Data Murid",
            ICON_TEACHERS "   Data Guru",
            ICON_CLASSES "   Data Kelas",
            ICON_ATTENDANCE "   Absensi Harian",
            ICON_ACADEMIC "   Capaian CP/TP",
            ICON_JOURNAL "   Jurnal Harian",
            ICON_GRADES "   Nilai & Transkrip",
            ICON_USERS "   Pengguna",
            ICON_BACKUP "   Backup & Restore",
            ICON_SETTINGS "   Pengaturan",
            ICON_ABOUT "   Tentang Software"
        };
        SidebarMenu mapping[] = {
            MENU_DASHBOARD, MENU_STUDENTS, MENU_TEACHERS, MENU_CLASSES,
            MENU_ATTENDANCE, MENU_ACADEMIC, MENU_JOURNAL,
            MENU_GRADES, MENU_USERS, MENU_BACKUP,
            MENU_SETTINGS, MENU_ABOUT
        };

        for (int i = 0; i < 12; i++) {
            nk_layout_row_dynamic(ctx, 36, 1);
            if (g_current_menu == mapping[i]) {
                // Highlight active button (indigo in light mode, sky-blue in dark mode)
                ctx->style.button.normal.data.color = g_light_mode ? nk_rgb(63, 81, 181) : nk_rgb(30, 136, 229);
                ctx->style.button.hover.data.color = g_light_mode ? nk_rgb(92, 107, 192) : nk_rgb(33, 150, 243);
                ctx->style.button.active.data.color = g_light_mode ? nk_rgb(48, 63, 159) : nk_rgb(21, 101, 192);
                ctx->style.button.text_normal = nk_rgb(255, 255, 255);
                ctx->style.button.text_hover = nk_rgb(255, 255, 255);
                ctx->style.button.text_active = nk_rgb(255, 255, 255);
            } else {
                // Inactive button (subtle background, clean text contrast)
                ctx->style.button.normal.data.color = g_light_mode ? nk_rgb(235, 237, 240) : nk_rgb(30, 30, 30);
                ctx->style.button.hover.data.color = g_light_mode ? nk_rgb(220, 222, 225) : nk_rgb(45, 45, 45);
                ctx->style.button.active.data.color = g_light_mode ? nk_rgb(200, 202, 205) : nk_rgb(25, 25, 25);
                ctx->style.button.text_normal = g_light_mode ? nk_rgb(33, 33, 33) : nk_rgb(200, 200, 200);
                ctx->style.button.text_hover = g_light_mode ? nk_rgb(33, 33, 33) : nk_rgb(255, 255, 255);
                ctx->style.button.text_active = g_light_mode ? nk_rgb(33, 33, 33) : nk_rgb(255, 255, 255);
            }

            if (nk_button_label(ctx, menus[i])) {
                g_current_menu = mapping[i];
                load_tab_data(g_current_menu);
            }
            // Add a small spacing spacer row
            nk_layout_row_dynamic(ctx, 3, 1);
            nk_spacing(ctx, 1);
        }
        
        // Reset style configurations back to default active theme
        ui_apply_theme(ctx);

        nk_group_end(ctx);
    }
}

// SUB-SCREEN: Top Bar
static void draw_topbar(struct nk_context *ctx) {
    if (nk_group_begin(ctx, "Topbar", NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_static(ctx, 160); // Role & enterprise badge tag
        nk_layout_row_template_push_dynamic(ctx);     // Welcome message
        nk_layout_row_template_push_static(ctx, 130); // Pengaturan Button
        nk_layout_row_template_push_static(ctx, 110); // Tentang Button
        nk_layout_row_template_push_static(ctx, 130); // Theme Switcher Button
        nk_layout_row_template_push_static(ctx, 100); // Logout Button
        nk_layout_row_template_end(ctx);

        const char *role_str = "ADMINISTRATOR";
        if (g_current_user.role == ROLE_GURU) role_str = "GURU BINAAN";
        else if (g_current_user.role == ROLE_STAF) role_str = "STAF TU";

        char role_tag[150];
        snprintf(role_tag, sizeof(role_tag), "[%s | PRO]", role_str);
        nk_label_colored(ctx, role_tag, NK_TEXT_LEFT, g_light_mode ? nk_rgb(63, 81, 181) : nk_rgb(30, 136, 229));

        char welcome[200];
        snprintf(welcome, sizeof(welcome), "Selamat Bekerja, %s", g_current_user.name);
        nk_label(ctx, welcome, NK_TEXT_LEFT);

        if (nk_button_label(ctx, ICON_SETTINGS "  Pengaturan")) {
            g_current_menu = MENU_SETTINGS;
            load_tab_data(MENU_SETTINGS);
        }

        if (nk_button_label(ctx, ICON_ABOUT "  Tentang")) {
            g_current_menu = MENU_ABOUT;
            load_tab_data(MENU_ABOUT);
        }

        char theme_btn_label[64];
        snprintf(theme_btn_label, sizeof(theme_btn_label), ICON_THEME "  %s", g_light_mode ? "GELAP" : "TERANG");
        if (nk_button_label(ctx, theme_btn_label)) {
            g_light_mode = !g_light_mode;
            ui_apply_theme(ctx);
        }

        if (nk_button_label(ctx, ICON_LOGOUT "  LOGOUT")) {
            g_logged_in = false;
            memset(&g_current_user, 0, sizeof(g_current_user));
            memset(g_login_user, 0, sizeof(g_login_user));
            memset(g_login_pass, 0, sizeof(g_login_pass));
            show_notification("Anda telah keluar dari aplikasi.", true);
        }
        nk_group_end(ctx);
    }
}

// SUB-SCREEN: Dashboard
static void draw_dashboard(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, ICON_DASHBOARD "   DASHBOARD MONITORING SEKOLAH", NK_TEXT_LEFT, g_theme_text_header);

    // Stats Grid - Responsive columns
    int stats_cols = 4;
    if (screen_width < 900) stats_cols = 2;
    if (screen_width < 600) stats_cols = 1;
    nk_layout_row_dynamic(ctx, 100, stats_cols);

    // Card 1: Total Students
    if (nk_group_begin(ctx, "CardSiswa", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, ICON_STUDENTS "  TOTAL SISWA AKTIF", NK_TEXT_LEFT, g_theme_text_muted);
        nk_layout_row_dynamic(ctx, 40, 1);
        char buf[50];
        snprintf(buf, sizeof(buf), "   %d Siswa", g_stats.total_students);
        nk_label_colored(ctx, buf, NK_TEXT_LEFT, nk_rgb(46, 204, 113));
        nk_group_end(ctx);
    }

    // Card 2: Total Teachers
    if (nk_group_begin(ctx, "CardGuru", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, ICON_TEACHERS "  TOTAL GURU", NK_TEXT_LEFT, g_theme_text_muted);
        nk_layout_row_dynamic(ctx, 40, 1);
        char buf[50];
        snprintf(buf, sizeof(buf), "   %d Guru", g_stats.total_teachers);
        nk_label_colored(ctx, buf, NK_TEXT_LEFT, nk_rgb(52, 152, 219));
        nk_group_end(ctx);
    }

    // Card 3: Total Classes
    if (nk_group_begin(ctx, "CardKelas", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, ICON_CLASSES "  TOTAL KELAS", NK_TEXT_LEFT, g_theme_text_muted);
        nk_layout_row_dynamic(ctx, 40, 1);
        char buf[50];
        snprintf(buf, sizeof(buf), "   %d Rombel", g_stats.total_classes);
        nk_label_colored(ctx, buf, NK_TEXT_LEFT, nk_rgb(241, 196, 15));
        nk_group_end(ctx);
    }

    // Card 4: Attendance Today
    if (nk_group_begin(ctx, "CardAttendance", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, ICON_ATTENDANCE "  KEHADIRAN HARI INI", NK_TEXT_LEFT, g_theme_text_muted);
        nk_layout_row_dynamic(ctx, 40, 1);
        char buf[50];
        snprintf(buf, sizeof(buf), "   %.1f %%", g_stats.attendance_rate_today);
        nk_label_colored(ctx, buf, NK_TEXT_LEFT, nk_rgb(155, 89, 182));
        nk_group_end(ctx);
    }

    nk_layout_row_dynamic(ctx, 20, 1); // Spacer

    // Hero banner section
    ctx->style.window.background = g_light_mode ? nk_rgb(232, 234, 246) : nk_rgb(33, 33, 33);
    nk_layout_row_dynamic(ctx, 90, 1);
    if (nk_group_begin(ctx, "HeroSection", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label_colored(ctx, "DANGERPCA ERP SEKOLAH (OFFLINE DESKTOP)", NK_TEXT_LEFT, g_light_mode ? nk_rgb(63, 81, 181) : nk_rgb(30, 136, 229));
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Sistem berjalan penuh secara offline dengan local database SQLite berkecepatan tinggi.", NK_TEXT_LEFT);
        nk_group_end(ctx);
    }
    ui_apply_theme(ctx); // Restore theme styling

    nk_layout_row_dynamic(ctx, 15, 1); // Spacer

    // Overview details
    nk_layout_row_dynamic(ctx, 180, 1);
    if (nk_group_begin(ctx, "AppOverview", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Fitur-fitur utama yang tersedia:", NK_TEXT_LEFT);
        
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label(ctx, "  " ICON_STUDENTS "   Manajemen data Murid, Guru, dan Rombongan Belajar (Kelas).", NK_TEXT_LEFT);
        nk_label(ctx, "  " ICON_ATTENDANCE "   Pencatatan Kehadiran (Absensi) harian terintegrasi ekspor laporan.", NK_TEXT_LEFT);
        nk_label(ctx, "  " ICON_ACADEMIC "   Kurikulum: Capaian Pembelajaran (CP), Tujuan Pembelajaran (TP) & ATP.", NK_TEXT_LEFT);
        nk_label(ctx, "  " ICON_GRADES "   Evaluasi Belajar: Nilai Harian format TP & Ujian Akhir (UTS/UAS).", NK_TEXT_LEFT);
        nk_label(ctx, "  " ICON_BACKUP "   Database & Keamanan: Ekspor laporan PDF & CSV (Excel) serta backup instan.", NK_TEXT_LEFT);
        nk_group_end(ctx);
    }
}

// SUB-SCREEN: Students Tab
static void draw_students_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "MANAJEMEN DATA MURID", NK_TEXT_LEFT, g_theme_text_header);

    if (!g_show_form) {
        // Toolbar (Search, Filter, Export, Add Buttons) - Responsive
        if (screen_width < 750) {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_end(ctx);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_search_query, sizeof(g_search_query), nk_filter_ascii);
            if (nk_button_label(ctx, "Cari")) {
                load_tab_data(MENU_STUDENTS);
            }

            nk_layout_row_dynamic(ctx, 35, 3);
            if (nk_button_label(ctx, "CSV")) {
                if (service_export_students_csv("siswa_export.csv")) show_notification("Sukses mengekspor CSV!", true);
                else show_notification("Gagal mengekspor CSV!", false);
            }
            if (nk_button_label(ctx, "PDF")) {
                if (service_export_students_pdf("siswa_export.pdf")) show_notification("Sukses mengekspor PDF!", true);
                else show_notification("Gagal mengekspor PDF!", false);
            }
            if (nk_button_label(ctx, "+ SISWA")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // nisn
                g_form_txt2[0] = '\0'; // nama
                g_form_txt3[0] = '\0'; // dob
                g_form_txt4[0] = '\0'; // address
                g_form_int1 = 0;       // class index
                g_form_gender_idx = 0; // L
                g_form_status_idx = 0; // Aktif
            }
        } else {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_static(ctx, 150); // Search bar
            nk_layout_row_template_push_static(ctx, 100); // Search button
            nk_layout_row_template_push_static(ctx, 120); // Export CSV
            nk_layout_row_template_push_static(ctx, 120); // Export PDF
            nk_layout_row_template_push_dynamic(ctx);     // Spacer
            nk_layout_row_template_push_static(ctx, 150); // Tambah Siswa button
            nk_layout_row_template_end(ctx);

            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_search_query, sizeof(g_search_query), nk_filter_ascii);
            if (nk_button_label(ctx, "Cari")) {
                load_tab_data(MENU_STUDENTS);
            }
            if (nk_button_label(ctx, "Ekspor CSV")) {
                if (service_export_students_csv("siswa_export.csv")) {
                    show_notification("Sukses mengekspor siswa_export.csv!", true);
                } else {
                    show_notification("Gagal mengekspor CSV!", false);
                }
            }
            if (nk_button_label(ctx, "Ekspor PDF")) {
                if (service_export_students_pdf("siswa_export.pdf")) {
                    show_notification("Sukses mengekspor siswa_export.pdf!", true);
                } else {
                    show_notification("Gagal mengekspor PDF!", false);
                }
            }
            nk_spacing(ctx, 1);
            if (nk_button_label(ctx, "+ TAMBAH SISWA")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // nisn
                g_form_txt2[0] = '\0'; // nama
            g_form_txt3[0] = '\0'; // dob
            g_form_txt4[0] = '\0'; // address
            g_form_int1 = 0;       // class index
            g_form_gender_idx = 0; // L
            g_form_status_idx = 0; // Aktif
            }
        }

        nk_layout_row_dynamic(ctx, 15, 1); // Spacer

        // Tabular list
        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_static(ctx, 80);  // NISN
        nk_layout_row_template_push_dynamic(ctx);     // Nama
        nk_layout_row_template_push_static(ctx, 80);  // Kelas
        nk_layout_row_template_push_static(ctx, 50);  // JK
        nk_layout_row_template_push_static(ctx, 80);  // Status
        nk_layout_row_template_push_static(ctx, 120); // Action Buttons
        nk_layout_row_template_end(ctx);

        // Header
        nk_label_colored(ctx, "NISN", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Nama Siswa", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Kelas", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "JK", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Status", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);

        for (int i = 0; i < g_students_count; i++) {
            nk_layout_row_template_begin(ctx, 35); // increased height to 35 for better padding
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 50);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 160); // increased width slightly for icon buttons
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_students[i].nisn, NK_TEXT_LEFT);
            nk_label(ctx, g_students[i].name, NK_TEXT_LEFT);
            nk_label(ctx, g_students[i].class_name, NK_TEXT_LEFT);
            nk_label(ctx, g_students[i].gender, NK_TEXT_LEFT);
            nk_label(ctx, g_students[i].status, NK_TEXT_LEFT);

            // Edit / Delete Buttons inside group cell
            char act_grp[64];
            snprintf(act_grp, sizeof(act_grp), "std_act_%d", g_students[i].id);
            if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 25, 2);
                if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                    g_show_form = true;
                    g_is_editing = true;
                    g_editing_id = g_students[i].id;
                    strncpy(g_form_txt1, g_students[i].nisn, sizeof(g_form_txt1) - 1);
                    strncpy(g_form_txt2, g_students[i].name, sizeof(g_form_txt2) - 1);
                    strncpy(g_form_txt3, g_students[i].dob, sizeof(g_form_txt3) - 1);
                    strncpy(g_form_txt4, g_students[i].address, sizeof(g_form_txt4) - 1);
                    
                    g_form_gender_idx = (strcmp(g_students[i].gender, "L") == 0) ? 0 : 1;
                    
                    if (strcmp(g_students[i].status, "Aktif") == 0) g_form_status_idx = 0;
                    else if (strcmp(g_students[i].status, "Lulus") == 0) g_form_status_idx = 1;
                    else g_form_status_idx = 2;

                    g_form_int1 = 0;
                    for (int c = 0; c < g_classes_count; c++) {
                        if (g_classes[c].id == g_students[i].class_id) {
                            g_form_int1 = c + 1; // 0 index represents "No class"
                            break;
                        }
                    }
                }
                if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                    if (db_delete_student(g_students[i].id)) {
                        show_notification("Data murid berhasil dihapus.", true);
                        load_tab_data(MENU_STUDENTS);
                    } else {
                        show_notification("Gagal menghapus data murid!", false);
                    }
                }
                nk_group_end(ctx);
            }
        }
    } else {
        // Form Panel
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, g_is_editing ? "EDIT DATA MURID" : "TAMBAH MURID BARU", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "NISN", NK_TEXT_LEFT);
        nk_label(ctx, "Nama Lengkap", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_decimal);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt2, sizeof(g_form_txt2), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Kelas", NK_TEXT_LEFT);
        nk_label(ctx, "Jenis Kelamin", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        // Build class dropdown list
        const char *class_options[101];
        class_options[0] = "-- Tanpa Kelas --";
        for (int c = 0; c < g_classes_count; c++) {
            class_options[c + 1] = g_classes[c].name;
        }
        g_form_int1 = nk_combo(ctx, class_options, g_classes_count + 1, g_form_int1, 25, nk_vec2(200, 200));
        
        const char *gender_options[] = {"Laki-laki (L)", "Perempuan (P)"};
        g_form_gender_idx = nk_combo(ctx, gender_options, 2, g_form_gender_idx, 25, nk_vec2(200, 100));

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Tanggal Lahir (YYYY-MM-DD)", NK_TEXT_LEFT);
        nk_label(ctx, "Status Keaktifan", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt3, sizeof(g_form_txt3), nk_filter_ascii);
        
        const char *status_options[] = {"Aktif", "Lulus", "Keluar"};
        g_form_status_idx = nk_combo(ctx, status_options, 3, g_form_status_idx, 25, nk_vec2(200, 100));

        draw_rich_text_editor(ctx, "Alamat Rumah & Catatan Khusus Siswa", g_form_txt4, sizeof(g_form_txt4), 80.0f, &g_rte_mode_student, "std_addr");

        nk_layout_row_dynamic(ctx, 15, 1); // Spacer

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "SIMPAN")) {
            if (strlen(g_form_txt1) == 0 || strlen(g_form_txt2) == 0) {
                show_notification("NISN dan Nama wajib diisi!", false);
            } else {
                Student s;
                s.id = g_editing_id;
                strncpy(s.nisn, g_form_txt1, sizeof(s.nisn) - 1);
                strncpy(s.name, g_form_txt2, sizeof(s.name) - 1);
                s.class_id = (g_form_int1 > 0) ? g_classes[g_form_int1 - 1].id : 0;
                strncpy(s.gender, g_form_gender_idx == 0 ? "L" : "P", sizeof(s.gender) - 1);
                strncpy(s.dob, g_form_txt3, sizeof(s.dob) - 1);
                strncpy(s.address, g_form_txt4, sizeof(s.address) - 1);
                
                if (g_form_status_idx == 0) strncpy(s.status, "Aktif", sizeof(s.status) - 1);
                else if (g_form_status_idx == 1) strncpy(s.status, "Lulus", sizeof(s.status) - 1);
                else strncpy(s.status, "Keluar", sizeof(s.status) - 1);

                bool success = g_is_editing ? db_update_student(&s) : db_create_student(&s);
                if (success) {
                    show_notification("Data murid berhasil disimpan.", true);
                    g_show_form = false;
                    load_tab_data(MENU_STUDENTS);
                } else {
                    show_notification("Gagal menyimpan data murid (NISN unik)!", false);
                }
            }
        }
        if (nk_button_label(ctx, "BATAL")) {
            g_show_form = false;
        }
    }
}

// SUB-SCREEN: Teachers Tab
static void draw_teachers_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "MANAJEMEN DATA GURU", NK_TEXT_LEFT, g_theme_text_header);

    if (!g_show_form) {
        // Toolbar (Search, Filter, Export, Add Buttons) - Responsive
        if (screen_width < 750) {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_end(ctx);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_search_query, sizeof(g_search_query), nk_filter_ascii);
            if (nk_button_label(ctx, "Cari")) {
                load_tab_data(MENU_TEACHERS);
            }

            nk_layout_row_dynamic(ctx, 35, 3);
            if (nk_button_label(ctx, "CSV")) {
                if (service_export_teachers_csv("guru_export.csv")) show_notification("Sukses mengekspor CSV!", true);
                else show_notification("Gagal mengekspor CSV!", false);
            }
            if (nk_button_label(ctx, "PDF")) {
                if (service_export_teachers_pdf("guru_export.pdf")) show_notification("Sukses mengekspor PDF!", true);
                else show_notification("Gagal mengekspor PDF!", false);
            }
            if (nk_button_label(ctx, "+ GURU")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // nip
                g_form_txt2[0] = '\0'; // nama
                g_form_txt3[0] = '\0'; // mapel
                g_form_gender_idx = 0; // L
                g_form_status_idx = 0; // Aktif
            }
        } else {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_static(ctx, 150); // Search bar
            nk_layout_row_template_push_static(ctx, 100); // Search button
            nk_layout_row_template_push_static(ctx, 120); // Export CSV
            nk_layout_row_template_push_static(ctx, 120); // Export PDF
            nk_layout_row_template_push_dynamic(ctx);     // Spacer
            nk_layout_row_template_push_static(ctx, 150); // Tambah Guru button
            nk_layout_row_template_end(ctx);

            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_search_query, sizeof(g_search_query), nk_filter_ascii);
            if (nk_button_label(ctx, "Cari")) {
                load_tab_data(MENU_TEACHERS);
            }
            if (nk_button_label(ctx, "Ekspor CSV")) {
                if (service_export_teachers_csv("guru_export.csv")) {
                    show_notification("Sukses mengekspor guru_export.csv!", true);
                } else {
                    show_notification("Gagal mengekspor CSV!", false);
                }
            }
            if (nk_button_label(ctx, "Ekspor PDF")) {
                if (service_export_teachers_pdf("guru_export.pdf")) {
                    show_notification("Sukses mengekspor guru_export.pdf!", true);
                } else {
                    show_notification("Gagal mengekspor PDF!", false);
                }
            }
            nk_spacing(ctx, 1);
            if (nk_button_label(ctx, "+ TAMBAH GURU")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // nip
            g_form_txt2[0] = '\0'; // nama
            g_form_txt3[0] = '\0'; // subject
            g_form_txt4[0] = '\0'; // phone
            g_form_gender_idx = 0; // L
            g_form_status_idx = 0; // Aktif
            }
        }

        nk_layout_row_dynamic(ctx, 15, 1); // Spacer

        // Tabular list
        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_static(ctx, 120); // NIP
        nk_layout_row_template_push_dynamic(ctx);     // Nama
        nk_layout_row_template_push_static(ctx, 50);  // JK
        nk_layout_row_template_push_static(ctx, 120); // Subject
        nk_layout_row_template_push_static(ctx, 80);  // Status
        nk_layout_row_template_push_static(ctx, 120); // Action Buttons
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "NIP", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Nama Guru", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "JK", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Mata Pelajaran", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Status", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);

        for (int i = 0; i < g_teachers_count; i++) {
            nk_layout_row_template_begin(ctx, 35); // increased height to 35 for better padding
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 50);
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 160); // increased width slightly for icon buttons
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_teachers[i].nip, NK_TEXT_LEFT);
            nk_label(ctx, g_teachers[i].name, NK_TEXT_LEFT);
            nk_label(ctx, g_teachers[i].gender, NK_TEXT_LEFT);
            nk_label(ctx, g_teachers[i].subject, NK_TEXT_LEFT);
            nk_label(ctx, g_teachers[i].status, NK_TEXT_LEFT);

            // Edit / Delete Buttons inside group cell
            char act_grp[64];
            snprintf(act_grp, sizeof(act_grp), "tch_act_%d", g_teachers[i].id);
            if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 25, 2);
                if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                    g_show_form = true;
                    g_is_editing = true;
                    g_editing_id = g_teachers[i].id;
                    strncpy(g_form_txt1, g_teachers[i].nip, sizeof(g_form_txt1) - 1);
                    strncpy(g_form_txt2, g_teachers[i].name, sizeof(g_form_txt2) - 1);
                    strncpy(g_form_txt3, g_teachers[i].subject, sizeof(g_form_txt3) - 1);
                    strncpy(g_form_txt4, g_teachers[i].phone, sizeof(g_form_txt4) - 1);
                    g_form_gender_idx = (strcmp(g_teachers[i].gender, "L") == 0) ? 0 : 1;
                    g_form_status_idx = (strcmp(g_teachers[i].status, "Aktif") == 0) ? 0 : (strcmp(g_teachers[i].status, "Cuti") == 0 ? 1 : 2);
                }
                if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                    if (db_delete_teacher(g_teachers[i].id)) {
                        show_notification("Data guru berhasil dihapus.", true);
                        load_tab_data(MENU_TEACHERS);
                    } else {
                        show_notification("Gagal menghapus data guru!", false);
                    }
                }
                nk_group_end(ctx);
            }
        }
    } else {
        // Form Panel
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, g_is_editing ? "EDIT DATA GURU" : "TAMBAH GURU BARU", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "NIP", NK_TEXT_LEFT);
        nk_label(ctx, "Nama Lengkap", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_decimal);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt2, sizeof(g_form_txt2), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Mata Pelajaran Binaan", NK_TEXT_LEFT);
        nk_label(ctx, "Jenis Kelamin", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt3, sizeof(g_form_txt3), nk_filter_ascii);
        
        const char *gender_options[] = {"Laki-laki (L)", "Perempuan (P)"};
        g_form_gender_idx = nk_combo(ctx, gender_options, 2, g_form_gender_idx, 25, nk_vec2(200, 100));

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Nomor Telepon", NK_TEXT_LEFT);
        nk_label(ctx, "Status Keaktifan", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt4, sizeof(g_form_txt4), nk_filter_decimal);
        
        const char *status_options[] = {"Aktif", "Cuti", "Pensiun"};
        g_form_status_idx = nk_combo(ctx, status_options, 3, g_form_status_idx, 25, nk_vec2(200, 100));

        nk_layout_row_dynamic(ctx, 20, 1); // Spacer

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "SIMPAN")) {
            if (strlen(g_form_txt1) == 0 || strlen(g_form_txt2) == 0) {
                show_notification("NIP dan Nama wajib diisi!", false);
            } else {
                Teacher t;
                t.id = g_editing_id;
                strncpy(t.nip, g_form_txt1, sizeof(t.nip) - 1);
                strncpy(t.name, g_form_txt2, sizeof(t.name) - 1);
                strncpy(t.subject, g_form_txt3, sizeof(t.subject) - 1);
                strncpy(t.phone, g_form_txt4, sizeof(t.phone) - 1);
                strncpy(t.gender, g_form_gender_idx == 0 ? "L" : "P", sizeof(t.gender) - 1);
                
                if (g_form_status_idx == 0) strncpy(t.status, "Aktif", sizeof(t.status) - 1);
                else if (g_form_status_idx == 1) strncpy(t.status, "Cuti", sizeof(t.status) - 1);
                else strncpy(t.status, "Pensiun", sizeof(t.status) - 1);

                bool success = g_is_editing ? db_update_teacher(&t) : db_create_teacher(&t);
                if (success) {
                    show_notification("Data guru berhasil disimpan.", true);
                    g_show_form = false;
                    load_tab_data(MENU_TEACHERS);
                } else {
                    show_notification("Gagal menyimpan data guru (NIP unik)!", false);
                }
            }
        }
        if (nk_button_label(ctx, "BATAL")) {
            g_show_form = false;
        }
    }
}

// SUB-SCREEN: Classes Tab
static void draw_classes_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "MANAJEMEN ROMBONGAN BELAJAR (KELAS)", NK_TEXT_LEFT, g_theme_text_header);

    if (!g_show_form) {
        if (screen_width < 400) {
            nk_layout_row_dynamic(ctx, 35, 1);
        } else {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 180);
            nk_layout_row_template_end(ctx);
            nk_spacing(ctx, 1);
        }
        
        if (nk_button_label(ctx, "+ TAMBAH ROMBEL KELAS")) {
            g_show_form = true;
            g_is_editing = false;
            g_editing_id = 0;
            g_form_txt1[0] = '\0'; // nama kelas
            g_form_txt2[0] = '\0'; // tahun ajaran
            g_form_int1 = 0;       // wali kelas index
        }

        nk_layout_row_dynamic(ctx, 15, 1); // Spacer

        // Headers
        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_static(ctx, 150); // Nama Kelas
        nk_layout_row_template_push_static(ctx, 120); // Tahun Ajaran
        nk_layout_row_template_push_dynamic(ctx);     // Wali Kelas
        nk_layout_row_template_push_static(ctx, 120); // Action Buttons
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "Nama Kelas / Rombel", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Tahun Ajaran", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Wali Kelas", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);

        for (int i = 0; i < g_classes_count; i++) {
            nk_layout_row_template_begin(ctx, 35); // increased height to 35
            nk_layout_row_template_push_static(ctx, 150);
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 160); // increased width for action buttons
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_classes[i].name, NK_TEXT_LEFT);
            nk_label(ctx, g_classes[i].academic_year, NK_TEXT_LEFT);
            nk_label(ctx, g_classes[i].teacher_name, NK_TEXT_LEFT);

            // Edit / Delete Buttons inside group cell
            char act_grp[64];
            snprintf(act_grp, sizeof(act_grp), "cls_act_%d", g_classes[i].id);
            if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 25, 2);
                if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                    g_show_form = true;
                    g_is_editing = true;
                    g_editing_id = g_classes[i].id;
                    strncpy(g_form_txt1, g_classes[i].name, sizeof(g_form_txt1) - 1);
                    strncpy(g_form_txt2, g_classes[i].academic_year, sizeof(g_form_txt2) - 1);
                    
                    g_form_int1 = 0; // "Belum Ditentukan"
                    for (int t = 0; t < g_teachers_count; t++) {
                        if (g_teachers[t].id == g_classes[i].teacher_id) {
                            g_form_int1 = t + 1;
                            break;
                        }
                    }
                }
                if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                    if (db_delete_class(g_classes[i].id)) {
                        show_notification("Data kelas berhasil dihapus.", true);
                        load_tab_data(MENU_CLASSES);
                    } else {
                        show_notification("Gagal menghapus kelas!", false);
                    }
                }
                nk_group_end(ctx);
            }
        }
    } else {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, g_is_editing ? "EDIT ROMBONGAN BELAJAR" : "TAMBAH ROMBONGAN BELAJAR", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Nama Rombel / Kelas (e.g. Kelas 10-A)", NK_TEXT_LEFT);
        nk_label(ctx, "Tahun Ajaran (e.g. 2025/2026)", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_ascii);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt2, sizeof(g_form_txt2), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label(ctx, "Pilih Wali Kelas", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 1);
        const char *teacher_options[201];
        teacher_options[0] = "-- Belum Ditentukan --";
        for (int t = 0; t < g_teachers_count; t++) {
            teacher_options[t + 1] = g_teachers[t].name;
        }
        g_form_int1 = nk_combo(ctx, teacher_options, g_teachers_count + 1, g_form_int1, 25, nk_vec2(300, 200));

        nk_layout_row_dynamic(ctx, 20, 1); // Spacer

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "SIMPAN")) {
            if (strlen(g_form_txt1) == 0 || strlen(g_form_txt2) == 0) {
                show_notification("Nama Kelas dan Tahun Ajaran wajib diisi!", false);
            } else {
                ClassEntity c;
                c.id = g_editing_id;
                strncpy(c.name, g_form_txt1, sizeof(c.name) - 1);
                strncpy(c.academic_year, g_form_txt2, sizeof(c.academic_year) - 1);
                c.teacher_id = (g_form_int1 > 0) ? g_teachers[g_form_int1 - 1].id : 0;

                bool success = g_is_editing ? db_update_class(&c) : db_create_class(&c);
                if (success) {
                    show_notification("Data rombel berhasil disimpan.", true);
                    g_show_form = false;
                    load_tab_data(MENU_CLASSES);
                } else {
                    show_notification("Gagal menyimpan data rombel!", false);
                }
            }
        }
        if (nk_button_label(ctx, "BATAL")) {
            g_show_form = false;
        }
    }
}

// SUB-SCREEN: Attendance Tab
static void draw_attendance_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "PENCATATAN KEHADIRAN (ABSENSI SISWA)", NK_TEXT_LEFT, g_theme_text_header);

    // Toolbar (Select Class & Date) - Responsive
    if (screen_width < 750) {
        nk_layout_row_dynamic(ctx, 35, 2);
        const char *class_options[100];
        for (int c = 0; c < g_classes_count; c++) {
            class_options[c] = g_classes[c].name;
        }
        if (g_classes_count > 0) {
            g_selected_class_idx = nk_combo(ctx, class_options, g_classes_count, g_selected_class_idx, 25, nk_vec2(180, 200));
        } else {
            nk_label(ctx, "Belum ada kelas!", NK_TEXT_LEFT);
        }
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance_date, sizeof(g_attendance_date), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "Muat")) {
            load_tab_data(MENU_ATTENDANCE);
        }
        if (nk_button_label(ctx, "SIMPAN SEMUA")) {
            bool all_ok = true;
            for (int i = 0; i < g_attendance_count; i++) {
                if (!db_save_attendance(g_attendance[i].student_id, g_attendance_date, g_attendance[i].status, g_attendance[i].notes)) {
                    all_ok = false;
                }
            }
            if (all_ok) {
                show_notification("Absensi berhasil disimpan.", true);
                db_get_dashboard_stats(&g_stats); // update stats
            } else {
                show_notification("Gagal menyimpan beberapa absensi!", false);
            }
        }

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "Ekspor CSV")) {
            if (g_classes_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "absensi_%s_%s.csv", g_classes[g_selected_class_idx].name, g_attendance_date);
                if (service_export_attendance_csv(filename, g_attendance_date, g_classes[g_selected_class_idx].id, g_classes[g_selected_class_idx].name)) {
                    show_notification("CSV berhasil diekspor!", true);
                } else {
                    show_notification("Gagal ekspor CSV!", false);
                }
            }
        }
        if (nk_button_label(ctx, "Ekspor PDF")) {
            if (g_classes_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "absensi_%s_%s.pdf", g_classes[g_selected_class_idx].name, g_attendance_date);
                if (service_export_attendance_pdf(filename, g_attendance_date, g_classes[g_selected_class_idx].id, g_classes[g_selected_class_idx].name)) {
                    show_notification("PDF berhasil diekspor!", true);
                } else {
                    show_notification("Gagal ekspor PDF!", false);
                }
            }
        }
    } else {
        nk_layout_row_template_begin(ctx, 35);
        nk_layout_row_template_push_static(ctx, 150); // Class dropdown
        nk_layout_row_template_push_static(ctx, 120); // Date input
        nk_layout_row_template_push_static(ctx, 100); // Load Button
        nk_layout_row_template_push_static(ctx, 120); // Save Button
        nk_layout_row_template_push_dynamic(ctx);     // Spacer
        nk_layout_row_template_push_static(ctx, 120); // Export CSV
        nk_layout_row_template_push_static(ctx, 120); // Export PDF
        nk_layout_row_template_end(ctx);

        const char *class_options[100];
        for (int c = 0; c < g_classes_count; c++) {
            class_options[c] = g_classes[c].name;
        }
        
        if (g_classes_count > 0) {
            g_selected_class_idx = nk_combo(ctx, class_options, g_classes_count, g_selected_class_idx, 25, nk_vec2(180, 200));
        } else {
            nk_label(ctx, "Belum ada kelas!", NK_TEXT_LEFT);
        }
        
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance_date, sizeof(g_attendance_date), nk_filter_ascii);
        
        if (nk_button_label(ctx, "Muat")) {
            load_tab_data(MENU_ATTENDANCE);
        }

        if (nk_button_label(ctx, "SIMPAN SEMUA")) {
            bool all_ok = true;
            for (int i = 0; i < g_attendance_count; i++) {
                if (!db_save_attendance(g_attendance[i].student_id, g_attendance_date, g_attendance[i].status, g_attendance[i].notes)) {
                    all_ok = false;
                }
            }
            if (all_ok) {
                show_notification("Absensi berhasil disimpan ke database.", true);
                db_get_dashboard_stats(&g_stats); // update stats
            } else {
                show_notification("Gagal menyimpan beberapa absensi!", false);
            }
        }

        nk_spacing(ctx, 1);
        
        if (nk_button_label(ctx, "Ekspor CSV")) {
            if (g_classes_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "absensi_%s_%s.csv", g_classes[g_selected_class_idx].name, g_attendance_date);
                if (service_export_attendance_csv(filename, g_attendance_date, g_classes[g_selected_class_idx].id, g_classes[g_selected_class_idx].name)) {
                    show_notification("CSV berhasil diekspor!", true);
                } else {
                    show_notification("Gagal ekspor CSV!", false);
                }
            }
        }

        if (nk_button_label(ctx, "Ekspor PDF")) {
            if (g_classes_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "absensi_%s_%s.pdf", g_classes[g_selected_class_idx].name, g_attendance_date);
                if (service_export_attendance_pdf(filename, g_attendance_date, g_classes[g_selected_class_idx].id, g_classes[g_selected_class_idx].name)) {
                    show_notification("PDF berhasil diekspor!", true);
                } else {
                    show_notification("Gagal ekspor PDF!", false);
                }
            }
        }
    }

    nk_layout_row_dynamic(ctx, 15, 1); // Spacer

    // Attendance Table Sheet
    nk_layout_row_template_begin(ctx, 25);
    nk_layout_row_template_push_dynamic(ctx);     // Nama Siswa
    nk_layout_row_template_push_static(ctx, 200); // Attendance Status buttons
    nk_layout_row_template_push_static(ctx, 200); // Keterangan / Notes
    nk_layout_row_template_end(ctx);

    nk_label_colored(ctx, "Nama Siswa", NK_TEXT_LEFT, g_theme_text_muted);
    nk_label_colored(ctx, "Status Kehadiran (H | S | I | A)", NK_TEXT_LEFT, g_theme_text_muted);
    nk_label_colored(ctx, "Catatan / Keterangan", NK_TEXT_LEFT, g_theme_text_muted);

    nk_layout_row_dynamic(ctx, 1, 1);
    nk_rule_horizontal(ctx, g_theme_border, 1);

    for (int i = 0; i < g_attendance_count; i++) {
        nk_layout_row_template_begin(ctx, 35);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_push_static(ctx, 200);
        nk_layout_row_template_push_static(ctx, 200);
        nk_layout_row_template_end(ctx);

        nk_label(ctx, g_attendance[i].student_name, NK_TEXT_LEFT);

        // Status Choice: horizontal radio buttons or similar
        // Let's do horizontal radio/combo options
        nk_layout_row_dynamic(ctx, 25, 4);
        if (nk_option_label(ctx, "H", g_attendance[i].status == ATT_HADIR)) g_attendance[i].status = ATT_HADIR;
        if (nk_option_label(ctx, "S", g_attendance[i].status == ATT_SAKIT)) g_attendance[i].status = ATT_SAKIT;
        if (nk_option_label(ctx, "I", g_attendance[i].status == ATT_IZIN)) g_attendance[i].status = ATT_IZIN;
        if (nk_option_label(ctx, "A", g_attendance[i].status == ATT_ALPA)) g_attendance[i].status = ATT_ALPA;

        nk_layout_row_dynamic(ctx, 25, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance[i].notes, sizeof(g_attendance[i].notes), nk_filter_ascii);
    }
}

// SUB-SCREEN: Academic (CP / TP / ATP)
static void draw_academic_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "KURIKULUM (CP, TP, ATP)", NK_TEXT_LEFT, g_theme_text_header);

    // Show 3 distinct sections or tabs using Nuklear groups or simple header buttons
    static int sub_tab = 0; // 0=CP, 1=TP, 2=ATP
    if (screen_width < 600) {
        nk_layout_row_dynamic(ctx, 30, 3);
        if (nk_button_label(ctx, "CP")) sub_tab = 0;
        if (nk_button_label(ctx, "TP")) sub_tab = 1;
        if (nk_button_label(ctx, "ATP")) sub_tab = 2;
    } else {
        nk_layout_row_dynamic(ctx, 30, 3);
        if (nk_button_label(ctx, "Capaian Pembelajaran (CP)")) sub_tab = 0;
        if (nk_button_label(ctx, "Tujuan Pembelajaran (TP)")) sub_tab = 1;
        if (nk_button_label(ctx, "Alur Tujuan Pembelajaran (ATP)")) sub_tab = 2;
    }

    nk_layout_row_dynamic(ctx, 15, 1); // Spacer

    if (sub_tab == 0) {
        // CP Section
        if (!g_show_form) {
            if (screen_width < 500) {
                nk_layout_row_dynamic(ctx, 35, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_search_query, sizeof(g_search_query), nk_filter_ascii);
                nk_layout_row_dynamic(ctx, 35, 2);
                if (nk_button_label(ctx, "Cari")) load_tab_data(MENU_ACADEMIC);
                if (nk_button_label(ctx, "+ CP")) {
                    g_show_form = true;
                    g_is_editing = false;
                    g_editing_id = 0;
                    g_form_txt1[0] = '\0'; // Kode
                    g_form_txt2[0] = '\0'; // Subject
                    g_form_txt3[0] = '\0'; // Description
                }
            } else {
                nk_layout_row_template_begin(ctx, 35);
                nk_layout_row_template_push_static(ctx, 150); // Search bar
                nk_layout_row_template_push_static(ctx, 80);  // Search btn
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_static(ctx, 150); // Add btn
                nk_layout_row_template_end(ctx);

                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_search_query, sizeof(g_search_query), nk_filter_ascii);
                if (nk_button_label(ctx, "Cari")) load_tab_data(MENU_ACADEMIC);
                nk_spacing(ctx, 1);
                if (nk_button_label(ctx, "+ TAMBAH CP")) {
                    g_show_form = true;
                    g_is_editing = false;
                    g_editing_id = 0;
                    g_form_txt1[0] = '\0'; // Kode
                    g_form_txt2[0] = '\0'; // Subject
                    g_form_txt3[0] = '\0'; // Description
                }
            }

            nk_layout_row_dynamic(ctx, 15, 1); // Spacer

            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_static(ctx, 100);
            nk_layout_row_template_push_static(ctx, 150);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_end(ctx);

            nk_label_colored(ctx, "Kode CP", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Mata Pelajaran", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Deskripsi Kompetensi", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

            nk_layout_row_dynamic(ctx, 1, 1);
            nk_rule_horizontal(ctx, g_theme_border, 1);

            for (int i = 0; i < g_cp_count; i++) {
                nk_layout_row_template_begin(ctx, 35);
                nk_layout_row_template_push_static(ctx, 100);
                nk_layout_row_template_push_static(ctx, 150);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_static(ctx, 160); // increased for icons
                nk_layout_row_template_end(ctx);

                nk_label(ctx, g_cp[i].code, NK_TEXT_LEFT);
                nk_label(ctx, g_cp[i].subject, NK_TEXT_LEFT);
                nk_label(ctx, g_cp[i].description, NK_TEXT_LEFT);

                // Action Buttons inside group cell
                char act_grp[64];
                snprintf(act_grp, sizeof(act_grp), "cp_act_%d", g_cp[i].id);
                if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                    nk_layout_row_dynamic(ctx, 25, 2);
                    if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                        g_show_form = true;
                        g_is_editing = true;
                        g_editing_id = g_cp[i].id;
                        strncpy(g_form_txt1, g_cp[i].code, sizeof(g_form_txt1) - 1);
                        strncpy(g_form_txt2, g_cp[i].subject, sizeof(g_form_txt2) - 1);
                        strncpy(g_form_txt3, g_cp[i].description, sizeof(g_form_txt3) - 1);
                    }
                    if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                        if (db_delete_cp(g_cp[i].id)) {
                            show_notification("CP berhasil dihapus.", true);
                            load_tab_data(MENU_ACADEMIC);
                        } else {
                            show_notification("Gagal menghapus CP!", false);
                        }
                    }
                    nk_group_end(ctx);
                }
            }
        } else {
            // Add/Edit CP Form
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, g_is_editing ? "EDIT CAPAIAN PEMBELAJARAN" : "TAMBAH CAPAIAN PEMBELAJARAN", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 22, 2);
            nk_label(ctx, "Kode CP (e.g. CP-MTK-01)", NK_TEXT_LEFT);
            nk_label(ctx, "Mata Pelajaran", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt2, sizeof(g_form_txt2), nk_filter_ascii);
            draw_rich_text_editor(ctx, "Deskripsi Capaian Pembelajaran (CP)", g_form_txt3, sizeof(g_form_txt3), 90.0f, &g_rte_mode_cp, "cp_desc");
            nk_layout_row_dynamic(ctx, 15, 1);

            nk_layout_row_dynamic(ctx, 35, 2);
            if (nk_button_label(ctx, "SIMPAN")) {
                if (strlen(g_form_txt1) == 0 || strlen(g_form_txt3) == 0) {
                    show_notification("Kode CP dan Deskripsi wajib diisi!", false);
                } else {
                    CapaianPembelajaran cp;
                    cp.id = g_editing_id;
                    strncpy(cp.code, g_form_txt1, sizeof(cp.code) - 1);
                    strncpy(cp.subject, g_form_txt2, sizeof(cp.subject) - 1);
                    strncpy(cp.description, g_form_txt3, sizeof(cp.description) - 1);

                    bool success = g_is_editing ? db_update_cp(&cp) : db_create_cp(&cp);
                    if (success) {
                        show_notification("CP berhasil disimpan.", true);
                        g_show_form = false;
                        load_tab_data(MENU_ACADEMIC);
                    } else {
                        show_notification("Gagal menyimpan CP (Kode unik)!", false);
                    }
                }
            }
            if (nk_button_label(ctx, "BATAL")) g_show_form = false;
        }
    }
    else if (sub_tab == 1) {
        // TP Section
        if (!g_show_form) {
            if (screen_width < 400) {
                nk_layout_row_dynamic(ctx, 35, 1);
            } else {
                nk_layout_row_template_begin(ctx, 35);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_static(ctx, 150);
                nk_layout_row_template_end(ctx);
                nk_spacing(ctx, 1);
            }
            if (nk_button_label(ctx, "+ TAMBAH TP")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // Kode
                g_form_txt2[0] = '\0'; // Description
                g_form_int1 = 0;       // CP index
            }

            nk_layout_row_dynamic(ctx, 15, 1);

            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_static(ctx, 100);
            nk_layout_row_template_push_static(ctx, 100);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_end(ctx);

            nk_label_colored(ctx, "Kode TP", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "CP Binaan", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Tujuan Pembelajaran (TP)", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

            nk_layout_row_dynamic(ctx, 1, 1);
            nk_rule_horizontal(ctx, g_theme_border, 1);

            for (int i = 0; i < g_tp_count; i++) {
                nk_layout_row_template_begin(ctx, 35);
                nk_layout_row_template_push_static(ctx, 100);
                nk_layout_row_template_push_static(ctx, 100);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_static(ctx, 160); // increased for icons
                nk_layout_row_template_end(ctx);

                nk_label(ctx, g_tp[i].code, NK_TEXT_LEFT);
                nk_label(ctx, g_tp[i].cp_code, NK_TEXT_LEFT);
                nk_label(ctx, g_tp[i].description, NK_TEXT_LEFT);

                // Action Buttons inside group cell
                char act_grp[64];
                snprintf(act_grp, sizeof(act_grp), "tp_act_%d", g_tp[i].id);
                if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                    nk_layout_row_dynamic(ctx, 25, 2);
                    if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                        g_show_form = true;
                        g_is_editing = true;
                        g_editing_id = g_tp[i].id;
                        strncpy(g_form_txt1, g_tp[i].code, sizeof(g_form_txt1) - 1);
                        strncpy(g_form_txt2, g_tp[i].description, sizeof(g_form_txt2) - 1);
                        
                        g_form_int1 = 0;
                        for (int cp = 0; cp < g_cp_count; cp++) {
                            if (g_cp[cp].id == g_tp[i].cp_id) {
                                g_form_int1 = cp;
                                break;
                            }
                        }
                    }
                    if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                        if (db_delete_tp(g_tp[i].id)) {
                            show_notification("TP berhasil dihapus.", true);
                            load_tab_data(MENU_ACADEMIC);
                        } else {
                            show_notification("Gagal menghapus TP!", false);
                        }
                    }
                    nk_group_end(ctx);
                }
            }
        } else {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, g_is_editing ? "EDIT TUJUAN PEMBELAJARAN" : "TAMBAH TUJUAN PEMBELAJARAN", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 22, 2);
            nk_label(ctx, "Pilih Capaian Pembelajaran (CP) Terkait", NK_TEXT_LEFT);
            nk_label(ctx, "Kode TP (e.g. TP-MTK-01a)", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            const char *cp_options[200];
            for (int cp = 0; cp < g_cp_count; cp++) {
                cp_options[cp] = g_cp[cp].code;
            }
            if (g_cp_count > 0) {
                g_form_int1 = nk_combo(ctx, cp_options, g_cp_count, g_form_int1, 25, nk_vec2(250, 180));
            } else {
                nk_label(ctx, "Buat CP dahulu!", NK_TEXT_LEFT);
            }
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_ascii);

            draw_rich_text_editor(ctx, "Deskripsi Kompetensi TP", g_form_txt2, sizeof(g_form_txt2), 90.0f, &g_rte_mode_tp, "tp_desc");
            nk_layout_row_dynamic(ctx, 15, 1);

            nk_layout_row_dynamic(ctx, 35, 2);
            if (nk_button_label(ctx, "SIMPAN")) {
                if (g_cp_count == 0 || strlen(g_form_txt1) == 0 || strlen(g_form_txt2) == 0) {
                    show_notification("Pilih CP, isi Kode & Deskripsi TP!", false);
                } else {
                    TujuanPembelajaran tp;
                    tp.id = g_editing_id;
                    tp.cp_id = g_cp[g_form_int1].id;
                    strncpy(tp.code, g_form_txt1, sizeof(tp.code) - 1);
                    strncpy(tp.description, g_form_txt2, sizeof(tp.description) - 1);

                    bool success = g_is_editing ? db_update_tp(&tp) : db_create_tp(&tp);
                    if (success) {
                        show_notification("TP berhasil disimpan.", true);
                        g_show_form = false;
                        load_tab_data(MENU_ACADEMIC);
                    } else {
                        show_notification("Gagal menyimpan TP (Kode unik)!", false);
                    }
                }
            }
            if (nk_button_label(ctx, "BATAL")) g_show_form = false;
        }
    }
    else {
        // ATP Section
        if (!g_show_form) {
            if (screen_width < 400) {
                nk_layout_row_dynamic(ctx, 35, 1);
            } else {
                nk_layout_row_template_begin(ctx, 35);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_static(ctx, 150);
                nk_layout_row_template_end(ctx);
                nk_spacing(ctx, 1);
            }
            if (nk_button_label(ctx, "+ TAMBAH ALUR (ATP)")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // Description
                g_form_int1 = 0;       // TP index
                g_form_status_idx = 1; // Order number (using status_idx helper)
            }

            nk_layout_row_dynamic(ctx, 15, 1);

            nk_layout_row_template_begin(ctx, 25);
            nk_layout_row_template_push_static(ctx, 80);  // Order
            nk_layout_row_template_push_static(ctx, 100); // TP code
            nk_layout_row_template_push_dynamic(ctx);     // ATP description
            nk_layout_row_template_push_static(ctx, 120); // Action Buttons
            nk_layout_row_template_end(ctx);

            nk_label_colored(ctx, "No Urut", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Kode TP", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Deskripsi Alur Pembelajaran (ATP)", NK_TEXT_LEFT, g_theme_text_muted);
            nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

            nk_layout_row_dynamic(ctx, 1, 1);
            nk_rule_horizontal(ctx, g_theme_border, 1);

            for (int i = 0; i < g_atp_count; i++) {
                nk_layout_row_template_begin(ctx, 35);
                nk_layout_row_template_push_static(ctx, 80);
                nk_layout_row_template_push_static(ctx, 100);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_static(ctx, 160); // increased for icons
                nk_layout_row_template_end(ctx);

                char ord_str[20];
                snprintf(ord_str, sizeof(ord_str), "Alur %d", g_atp[i].order_num);
                nk_label(ctx, ord_str, NK_TEXT_LEFT);
                nk_label(ctx, g_atp[i].tp_code, NK_TEXT_LEFT);
                nk_label(ctx, g_atp[i].description, NK_TEXT_LEFT);

                // Action Buttons inside group cell
                char act_grp[64];
                snprintf(act_grp, sizeof(act_grp), "atp_act_%d", g_atp[i].id);
                if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                    nk_layout_row_dynamic(ctx, 25, 2);
                    if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                        g_show_form = true;
                        g_is_editing = true;
                        g_editing_id = g_atp[i].id;
                        strncpy(g_form_txt1, g_atp[i].description, sizeof(g_form_txt1) - 1);
                        g_form_status_idx = g_atp[i].order_num;
                        
                        g_form_int1 = 0;
                        for (int tp = 0; tp < g_tp_count; tp++) {
                            if (g_tp[tp].id == g_atp[i].tp_id) {
                                g_form_int1 = tp;
                                break;
                            }
                        }
                    }
                    if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                        if (db_delete_atp(g_atp[i].id)) {
                            show_notification("Alur ATP berhasil dihapus.", true);
                            load_tab_data(MENU_ACADEMIC);
                        } else {
                            show_notification("Gagal menghapus ATP!", false);
                        }
                    }
                    nk_group_end(ctx);
                }
            }
        } else {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, g_is_editing ? "EDIT ALUR TUJUAN PEMBELAJARAN" : "TAMBAH ALUR TUJUAN PEMBELAJARAN", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 22, 2);
            nk_label(ctx, "Pilih Tujuan Pembelajaran (TP) Terkait", NK_TEXT_LEFT);
            nk_label(ctx, "No Urut Urutan", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            const char *tp_options[300];
            for (int tp = 0; tp < g_tp_count; tp++) {
                tp_options[tp] = g_tp[tp].code;
            }
            if (g_cp_count > 0) {
                g_form_int1 = nk_combo(ctx, tp_options, g_tp_count, g_form_int1, 25, nk_vec2(250, 180));
            } else {
                nk_label(ctx, "Buat TP dahulu!", NK_TEXT_LEFT);
            }
            // Simple integer entry for order
            nk_property_int(ctx, "Urutan #", 1, &g_form_status_idx, 100, 1, 1);

            draw_rich_text_editor(ctx, "Deskripsi Langkah Alur / Skenario ATP", g_form_txt1, sizeof(g_form_txt1), 90.0f, &g_rte_mode_atp, "atp_desc");
            nk_layout_row_dynamic(ctx, 15, 1);

            nk_layout_row_dynamic(ctx, 35, 2);
            if (nk_button_label(ctx, "SIMPAN")) {
                if (g_tp_count == 0 || strlen(g_form_txt1) == 0) {
                    show_notification("Pilih TP dan isi Deskripsi ATP!", false);
                } else {
                    AlurTujuanPembelajaran atp;
                    atp.id = g_editing_id;
                    atp.tp_id = g_tp[g_form_int1].id;
                    atp.order_num = g_form_status_idx;
                    strncpy(atp.description, g_form_txt1, sizeof(atp.description) - 1);

                    bool success = g_is_editing ? db_update_atp(&atp) : db_create_atp(&atp);
                    if (success) {
                        show_notification("ATP berhasil disimpan.", true);
                        g_show_form = false;
                        load_tab_data(MENU_ACADEMIC);
                    } else {
                        show_notification("Gagal menyimpan ATP!", false);
                    }
                }
            }
            if (nk_button_label(ctx, "BATAL")) g_show_form = false;
        }
    }
}

// SUB-SCREEN: Daily Journal
static void draw_journal_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "JURNAL HARIAN GURU & AKTIVITAS ROMBEL", NK_TEXT_LEFT, g_theme_text_header);

    if (g_show_journal_detail) {
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label_colored(ctx, "DETAIL JURNAL HARIAN (PRATINJAU RICH TEXT)", NK_TEXT_LEFT, g_theme_text_header);

        nk_layout_row_dynamic(ctx, 22, 3);
        nk_label_colored(ctx, g_detail_journal.teacher_name, NK_TEXT_LEFT, g_theme_primary);
        nk_label_colored(ctx, g_detail_journal.class_name, NK_TEXT_LEFT, g_theme_accent);
        nk_label_colored(ctx, g_detail_journal.date, NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);

        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, "AKTIVITAS & MATERI POKOK PEMBELAJARAN:", NK_TEXT_LEFT, g_theme_text_header);

        nk_layout_row_dynamic(ctx, 140, 1);
        if (nk_group_begin(ctx, "jrn_det_act_grp", 0)) {
            render_rich_text_content(ctx, g_detail_journal.activity, "jrn_det_act");
            nk_group_end(ctx);
        }

        if (strlen(g_detail_journal.notes) > 0) {
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label_colored(ctx, "CATATAN KEJADIAN KHUSUS:", NK_TEXT_LEFT, g_theme_text_header);
            nk_layout_row_dynamic(ctx, 90, 1);
            if (nk_group_begin(ctx, "jrn_det_note_grp", 0)) {
                render_rich_text_content(ctx, g_detail_journal.notes, "jrn_det_note");
                nk_group_end(ctx);
            }
        }

        nk_layout_row_dynamic(ctx, 15, 1);
        nk_layout_row_dynamic(ctx, 35, 1);
        if (nk_button_label(ctx, "KEMBALI KE DAFTAR JURNAL")) g_show_journal_detail = false;
        return;
    }

    if (!g_show_form) {
        if (screen_width < 450) {
            nk_layout_row_dynamic(ctx, 35, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance_date, sizeof(g_attendance_date), nk_filter_ascii);
            nk_layout_row_dynamic(ctx, 35, 2);
            if (nk_button_label(ctx, "Filter")) load_tab_data(MENU_JOURNAL);
            if (nk_button_label(ctx, "+ JURNAL")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // activity
                g_form_txt2[0] = '\0'; // notes
                g_form_int1 = 0;       // teacher idx
                g_form_gender_idx = 0; // class idx
                get_today_date(g_form_txt3, sizeof(g_form_txt3)); // date
            }
        } else {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_static(ctx, 120); // Date filter
            nk_layout_row_template_push_static(ctx, 80);  // Filter btn
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 150); // Add btn
            nk_layout_row_template_end(ctx);

            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance_date, sizeof(g_attendance_date), nk_filter_ascii);
            if (nk_button_label(ctx, "Filter")) load_tab_data(MENU_JOURNAL);
            nk_spacing(ctx, 1);
            if (nk_button_label(ctx, "+ BUAT JURNAL")) {
                g_show_form = true;
                g_is_editing = false;
                g_editing_id = 0;
                g_form_txt1[0] = '\0'; // activity
                g_form_txt2[0] = '\0'; // notes
                g_form_int1 = 0;       // teacher idx
                g_form_gender_idx = 0; // class idx
                get_today_date(g_form_txt3, sizeof(g_form_txt3)); // date
            }
        }

        nk_layout_row_dynamic(ctx, 15, 1);

        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_static(ctx, 90);  // Date
        nk_layout_row_template_push_static(ctx, 120); // Teacher
        nk_layout_row_template_push_static(ctx, 80);  // Class
        nk_layout_row_template_push_dynamic(ctx);     // Activity
        nk_layout_row_template_push_static(ctx, 210); // Action Buttons (increased for Baca, Edit, Hapus)
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "Tanggal", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Guru Pelapor", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Kelas", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Aktivitas Pembelajaran", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);

        for (int i = 0; i < g_journals_count; i++) {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_static(ctx, 90);
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 210); // increased for icons & Baca
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_journals[i].date, NK_TEXT_LEFT);
            nk_label(ctx, g_journals[i].teacher_name, NK_TEXT_LEFT);
            nk_label(ctx, g_journals[i].class_name, NK_TEXT_LEFT);
            nk_label(ctx, g_journals[i].activity, NK_TEXT_LEFT);

            // Action Buttons inside group cell
            char act_grp[64];
            snprintf(act_grp, sizeof(act_grp), "jrn_act_%d", g_journals[i].id);
            if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 25, 3);
                if (nk_button_label(ctx, "Baca")) {
                    g_detail_journal = g_journals[i];
                    g_show_journal_detail = true;
                }
                if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                    g_show_form = true;
                    g_is_editing = true;
                    g_editing_id = g_journals[i].id;
                    strncpy(g_form_txt1, g_journals[i].activity, sizeof(g_form_txt1) - 1);
                    strncpy(g_form_txt2, g_journals[i].notes, sizeof(g_form_txt2) - 1);
                    strncpy(g_form_txt3, g_journals[i].date, sizeof(g_form_txt3) - 1);
                    
                    g_form_int1 = 0;
                    for (int t = 0; t < g_teachers_count; t++) {
                        if (g_teachers[t].id == g_journals[i].teacher_id) {
                            g_form_int1 = t;
                            break;
                        }
                    }
                    g_form_gender_idx = 0;
                    for (int c = 0; c < g_classes_count; c++) {
                        if (g_classes[c].id == g_journals[i].class_id) {
                            g_form_gender_idx = c;
                            break;
                        }
                    }
                }
                if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                    if (db_delete_journal(g_journals[i].id)) {
                        show_notification("Jurnal harian berhasil dihapus.", true);
                        load_tab_data(MENU_JOURNAL);
                    } else {
                        show_notification("Gagal menghapus jurnal!", false);
                    }
                }
                nk_group_end(ctx);
            }
        }
    } else {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, g_is_editing ? "EDIT JURNAL AKTIVITAS" : "TAMBAH JURNAL HARIAN", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 22, 3);
        nk_label(ctx, "Pilih Guru", NK_TEXT_LEFT);
        nk_label(ctx, "Pilih Kelas", NK_TEXT_LEFT);
        nk_label(ctx, "Tanggal (YYYY-MM-DD)", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 3);
        const char *teacher_options[200];
        for (int t = 0; t < g_teachers_count; t++) teacher_options[t] = g_teachers[t].name;
        if (g_teachers_count > 0) {
            g_form_int1 = nk_combo(ctx, teacher_options, g_teachers_count, g_form_int1, 25, nk_vec2(200, 180));
        } else {
            nk_label(ctx, "Buat data Guru dahulu!", NK_TEXT_LEFT);
        }

        const char *class_options[100];
        for (int c = 0; c < g_classes_count; c++) class_options[c] = g_classes[c].name;
        if (g_classes_count > 0) {
            g_form_gender_idx = nk_combo(ctx, class_options, g_classes_count, g_form_gender_idx, 25, nk_vec2(200, 180));
        } else {
            nk_label(ctx, "Buat data Kelas dahulu!", NK_TEXT_LEFT);
        }
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt3, sizeof(g_form_txt3), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 10, 1);

        draw_rich_text_editor(ctx, "Aktivitas & Materi Pokok Pembelajaran", g_form_txt1, sizeof(g_form_txt1), 110.0f, &g_rte_mode_journal_act, "jrn_act");

        nk_layout_row_dynamic(ctx, 10, 1);

        draw_rich_text_editor(ctx, "Catatan Kejadian Khusus (Optional)", g_form_txt2, sizeof(g_form_txt2), 80.0f, &g_rte_mode_journal_note, "jrn_note");

        nk_layout_row_dynamic(ctx, 15, 1);

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "SIMPAN")) {
            if (g_teachers_count == 0 || g_classes_count == 0 || strlen(g_form_txt1) == 0 || strlen(g_form_txt3) == 0) {
                show_notification("Lengkapi semua isian utama!", false);
            } else {
                DailyJournal j;
                j.id = g_editing_id;
                j.teacher_id = g_teachers[g_form_int1].id;
                j.class_id = g_classes[g_form_gender_idx].id;
                strncpy(j.date, g_form_txt3, sizeof(j.date) - 1);
                strncpy(j.activity, g_form_txt1, sizeof(j.activity) - 1);
                strncpy(j.notes, g_form_txt2, sizeof(j.notes) - 1);

                bool success = g_is_editing ? db_update_journal(&j) : db_create_journal(&j);
                if (success) {
                    show_notification("Jurnal harian berhasil disimpan.", true);
                    g_show_form = false;
                    load_tab_data(MENU_JOURNAL);
                } else {
                    show_notification("Gagal menyimpan jurnal harian!", false);
                }
            }
        }
        if (nk_button_label(ctx, "BATAL")) g_show_form = false;
    }
}

// SUB-SCREEN: Grades Tab
static void draw_grades_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "MANAJEMEN NILAI BELAJAR SISWA & TRANSKRIP", NK_TEXT_LEFT, g_theme_text_header);

    // Choose Student and Load their specific grades - Responsive
    if (screen_width < 650) {
        nk_layout_row_dynamic(ctx, 35, 2);
        const char *student_options[500];
        for (int s = 0; s < g_students_count; s++) student_options[s] = g_students[s].name;
        if (g_students_count > 0) {
            g_selected_student_idx = nk_combo(ctx, student_options, g_students_count, g_selected_student_idx, 25, nk_vec2(220, 200));
        } else {
            nk_label(ctx, "Tidak ada Siswa!", NK_TEXT_LEFT);
        }
        if (nk_button_label(ctx, "Tampilkan")) load_tab_data(MENU_GRADES);

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "CSV")) {
            if (g_students_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "nilai_%s.csv", g_students[g_selected_student_idx].name);
                if (service_export_grades_csv(filename, g_students[g_selected_student_idx].id, g_students[g_selected_student_idx].name)) {
                    show_notification("CSV Transkrip diekspor!", true);
                } else {
                    show_notification("Gagal ekspor CSV!", false);
                }
            }
        }
        if (nk_button_label(ctx, "PDF")) {
            if (g_students_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "nilai_%s.pdf", g_students[g_selected_student_idx].name);
                if (service_export_grades_pdf(filename, g_students[g_selected_student_idx].id, g_students[g_selected_student_idx].name)) {
                    show_notification("PDF Transkrip diekspor!", true);
                } else {
                    show_notification("Gagal ekspor PDF!", false);
                }
            }
        }
    } else {
        nk_layout_row_template_begin(ctx, 35);
        nk_layout_row_template_push_static(ctx, 220); // Student select Combo
        nk_layout_row_template_push_static(ctx, 80);  // Load btn
        nk_layout_row_template_push_dynamic(ctx);     // Spacer
        nk_layout_row_template_push_static(ctx, 130); // Export CSV
        nk_layout_row_template_push_static(ctx, 130); // Export PDF
        nk_layout_row_template_end(ctx);

        const char *student_options[500];
        for (int s = 0; s < g_students_count; s++) student_options[s] = g_students[s].name;
        if (g_students_count > 0) {
            g_selected_student_idx = nk_combo(ctx, student_options, g_students_count, g_selected_student_idx, 25, nk_vec2(220, 200));
        } else {
            nk_label(ctx, "Tidak ada Siswa!", NK_TEXT_LEFT);
        }
        if (nk_button_label(ctx, "Tampilkan")) load_tab_data(MENU_GRADES);
        
        nk_spacing(ctx, 1);

        if (nk_button_label(ctx, "Ekspor CSV")) {
            if (g_students_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "nilai_%s.csv", g_students[g_selected_student_idx].name);
                if (service_export_grades_csv(filename, g_students[g_selected_student_idx].id, g_students[g_selected_student_idx].name)) {
                    show_notification("CSV Transkrip berhasil diekspor!", true);
                } else {
                    show_notification("Gagal ekspor CSV!", false);
                }
            }
        }

        if (nk_button_label(ctx, "Ekspor PDF")) {
            if (g_students_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "nilai_%s.pdf", g_students[g_selected_student_idx].name);
                if (service_export_grades_pdf(filename, g_students[g_selected_student_idx].id, g_students[g_selected_student_idx].name)) {
                    show_notification("PDF Transkrip berhasil diekspor!", true);
                } else {
                    show_notification("Gagal ekspor PDF!", false);
                }
            }
        }
    }

    nk_layout_row_dynamic(ctx, 20, 1); // Spacer

    if (g_students_count == 0) return;

    // Two-Column Grid: Left Column is Grade Entry Forms, Right Column is Grade Display Tables - Responsive
    int grid_cols = 2;
    int group_height = 350;
    if (screen_width < 750) {
        grid_cols = 1;
        group_height = 320;
    }
    
    if (grid_cols == 1) nk_layout_row_dynamic(ctx, group_height, 1);
    else nk_layout_row_dynamic(ctx, group_height, 2);

    // Left Column: Entry Form Group
    if (nk_group_begin(ctx, "InputGradesGroup", NK_WINDOW_BORDER)) {
        static int grade_form_type = 0; // 0=Daily, 1=Exam
        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "Nilai Harian (TP)", grade_form_type == 0)) grade_form_type = 0;
        if (nk_option_label(ctx, "Nilai Ujian (Sumatif)", grade_form_type == 1)) grade_form_type = 1;

        nk_layout_row_dynamic(ctx, 10, 1); // Spacer

        if (grade_form_type == 0) {
            // Daily Grade Form (Student + TP + Score + Notes + Date)
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label_colored(ctx, "INPUT NILAI HARIAN SISWA", NK_TEXT_LEFT, nk_rgb(62, 141, 240));

            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label(ctx, "Pilih Tujuan Pembelajaran (TP)", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 30, 1);
            const char *tp_options[300];
            for (int t = 0; t < g_tp_count; t++) tp_options[t] = g_tp[t].code;
            if (g_tp_count > 0) {
                g_selected_tp_idx = nk_combo(ctx, tp_options, g_tp_count, g_selected_tp_idx, 25, nk_vec2(250, 200));
            } else {
                nk_label(ctx, "Buat TP dahulu!", NK_TEXT_LEFT);
            }

            nk_layout_row_dynamic(ctx, 22, 2);
            nk_label(ctx, "Nilai Angka (0-100)", NK_TEXT_LEFT);
            nk_label(ctx, "Tanggal (YYYY-MM-DD)", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            nk_property_double(ctx, "Score", 0.0, &g_form_dbl1, 100.0, 1.0, 1.0);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance_date, sizeof(g_attendance_date), nk_filter_ascii);

            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label(ctx, "Catatan Kompetensi", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_ascii);

            nk_layout_row_dynamic(ctx, 15, 1);

            nk_layout_row_dynamic(ctx, 35, 1);
            if (nk_button_label(ctx, "SIMPAN NILAI HARIAN")) {
                if (g_tp_count == 0 || strlen(g_attendance_date) == 0) {
                    show_notification("Lengkapi TP dan Tanggal!", false);
                } else {
                    int stud_id = g_students[g_selected_student_idx].id;
                    int tp_id = g_tp[g_selected_tp_idx].id;
                    if (db_save_daily_grade(stud_id, tp_id, g_form_dbl1, g_attendance_date, g_form_txt1)) {
                        show_notification("Nilai harian berhasil disimpan.", true);
                        load_tab_data(MENU_GRADES);
                    } else {
                        show_notification("Gagal menyimpan nilai harian!", false);
                    }
                }
            }
        } else {
            // Exam Grade Form
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label_colored(ctx, "INPUT NILAI UJIAN SEMESTER", NK_TEXT_LEFT, nk_rgb(62, 141, 240));

            nk_layout_row_dynamic(ctx, 22, 2);
            nk_label(ctx, "Mata Pelajaran", NK_TEXT_LEFT);
            nk_label(ctx, "Jenis Evaluasi", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_subject_filter, sizeof(g_subject_filter), nk_filter_ascii);
            
            const char *exam_options[] = {"UTS (Tengah Semester)", "UAS (Akhir Semester)"};
            g_form_int1 = nk_combo(ctx, exam_options, 2, g_form_int1, 25, nk_vec2(200, 100));

            nk_layout_row_dynamic(ctx, 22, 2);
            nk_label(ctx, "Nilai Angka (0-100)", NK_TEXT_LEFT);
            nk_label(ctx, "Tanggal Pelaksanaan", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            nk_property_double(ctx, "Score", 0.0, &g_form_dbl1, 100.0, 1.0, 1.0);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_attendance_date, sizeof(g_attendance_date), nk_filter_ascii);

            nk_layout_row_dynamic(ctx, 20, 1);

            nk_layout_row_dynamic(ctx, 35, 1);
            if (nk_button_label(ctx, "SIMPAN NILAI UJIAN")) {
                if (strlen(g_subject_filter) == 0 || strlen(g_attendance_date) == 0) {
                    show_notification("Lengkapi Mapel dan Tanggal!", false);
                } else {
                    int stud_id = g_students[g_selected_student_idx].id;
                    ExamType etype = (g_form_int1 == 0) ? EXAM_UTS : EXAM_UAS;
                    if (db_save_exam_grade(stud_id, g_subject_filter, etype, g_form_dbl1, g_attendance_date)) {
                        show_notification("Nilai ujian berhasil disimpan.", true);
                        load_tab_data(MENU_GRADES);
                    } else {
                        show_notification("Gagal menyimpan nilai ujian!", false);
                    }
                }
            }
        }
        nk_group_end(ctx);
    }

    if (grid_cols == 1) nk_layout_row_dynamic(ctx, group_height, 1);

    // Right Column: Display Tables Group
    if (nk_group_begin(ctx, "DisplayGradesGroup", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label_colored(ctx, "DAFTAR CAPAIAN NILAI HARIAN", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_static(ctx, 70);  // TP code
        nk_layout_row_template_push_static(ctx, 50);  // score
        nk_layout_row_template_push_dynamic(ctx);     // notes
        nk_layout_row_template_push_static(ctx, 60);  // delete action (increased slightly)
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "TP", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        nk_label_colored(ctx, "Nilai", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        nk_label_colored(ctx, "Catatan Kompetensi", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        
        for (int i = 0; i < g_daily_grades_count; i++) {
            nk_layout_row_template_begin(ctx, 32); // increased to 32
            nk_layout_row_template_push_static(ctx, 70);
            nk_layout_row_template_push_static(ctx, 50);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 60);
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_daily_grades[i].tp_code, NK_TEXT_LEFT);
            char sc_buf[20];
            snprintf(sc_buf, sizeof(sc_buf), "%.1f", g_daily_grades[i].score);
            nk_label_colored(ctx, sc_buf, NK_TEXT_LEFT, g_daily_grades[i].score >= 70.0 ? nk_rgb(46, 204, 113) : nk_rgb(192, 57, 43));
            nk_label(ctx, g_daily_grades[i].notes, NK_TEXT_LEFT);
            if (nk_button_label(ctx, ICON_TRASH)) {
                if (db_delete_daily_grade(g_daily_grades[i].id)) {
                    show_notification("Nilai harian berhasil dihapus.", true);
                    load_tab_data(MENU_GRADES);
                }
            }
        }

        nk_layout_row_dynamic(ctx, 15, 1); // Spacer

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label_colored(ctx, "DAFTAR HASIL EVALUASI UJIAN SUMATIF", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_dynamic(ctx);     // Subject
        nk_layout_row_template_push_static(ctx, 60);  // Type (UTS/UAS)
        nk_layout_row_template_push_static(ctx, 50);  // score
        nk_layout_row_template_push_static(ctx, 60);  // action
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "Mata Pelajaran", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        nk_label_colored(ctx, "Ujian", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        nk_label_colored(ctx, "Nilai", NK_TEXT_LEFT, nk_rgb(120, 130, 145));
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, nk_rgb(120, 130, 145));

        for (int i = 0; i < g_exam_grades_count; i++) {
            nk_layout_row_template_begin(ctx, 32); // increased to 32
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 60);
            nk_layout_row_template_push_static(ctx, 50);
            nk_layout_row_template_push_static(ctx, 60);
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_exam_grades[i].subject, NK_TEXT_LEFT);
            nk_label(ctx, g_exam_grades[i].exam_type == EXAM_UTS ? "UTS" : "UAS", NK_TEXT_LEFT);
            char sc_buf[20];
            snprintf(sc_buf, sizeof(sc_buf), "%.1f", g_exam_grades[i].score);
            nk_label_colored(ctx, sc_buf, NK_TEXT_LEFT, g_exam_grades[i].score >= 70.0 ? nk_rgb(46, 204, 113) : nk_rgb(192, 57, 43));
            if (nk_button_label(ctx, ICON_TRASH)) {
                if (db_delete_exam_grade(g_exam_grades[i].id)) {
                    show_notification("Nilai ujian berhasil dihapus.", true);
                    load_tab_data(MENU_GRADES);
                }
            }
        }
        nk_group_end(ctx);
    }
}

// SUB-SCREEN: Users Tab (Admin Only)
static void draw_users_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "MANAJEMEN PENGGUNA APLIKASI (USERS)", NK_TEXT_LEFT, g_theme_text_header);

    if (g_current_user.role != ROLE_ADMIN) {
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label_colored(ctx, "MAAF, MENU INI HANYA BISA DIAKSES OLEH ADMINISTRATOR UTAMA.", NK_TEXT_CENTERED, nk_rgb(192, 57, 43));
        return;
    }

    if (!g_show_form) {
        if (screen_width < 400) {
            nk_layout_row_dynamic(ctx, 35, 1);
        } else {
            nk_layout_row_template_begin(ctx, 35);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 180);
            nk_layout_row_template_end(ctx);
            nk_spacing(ctx, 1);
        }
        if (nk_button_label(ctx, "+ TAMBAH USER BARU")) {
            g_show_form = true;
            g_is_editing = false;
            g_editing_id = 0;
            g_form_txt1[0] = '\0'; // username
            g_form_txt2[0] = '\0'; // password
            g_form_txt3[0] = '\0'; // name
            g_form_role_idx = 1;   // GURU by default
        }

        nk_layout_row_dynamic(ctx, 15, 1);

        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_static(ctx, 120); // Username
        nk_layout_row_template_push_dynamic(ctx);     // Name
        nk_layout_row_template_push_static(ctx, 120); // Role
        nk_layout_row_template_push_static(ctx, 120); // Actions
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "Username", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Nama Lengkap", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Peran (Role)", NK_TEXT_LEFT, g_theme_text_muted);
        nk_label_colored(ctx, "Aksi", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 1, 1);
        nk_rule_horizontal(ctx, g_theme_border, 1);

        for (int i = 0; i < g_users_count; i++) {
            nk_layout_row_template_begin(ctx, 35); // increased height to 35
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 120);
            nk_layout_row_template_push_static(ctx, 160); // increased for icons
            nk_layout_row_template_end(ctx);

            nk_label(ctx, g_users[i].username, NK_TEXT_LEFT);
            nk_label(ctx, g_users[i].name, NK_TEXT_LEFT);
            
            const char *role_str = "ADMINISTRATOR";
            if (g_users[i].role == ROLE_GURU) role_str = "GURU";
            else if (g_users[i].role == ROLE_STAF) role_str = "STAF TATA USAHA";
            nk_label(ctx, role_str, NK_TEXT_LEFT);

            // Action Buttons inside group cell
            char act_grp[64];
            snprintf(act_grp, sizeof(act_grp), "usr_act_%d", g_users[i].id);
            if (nk_group_begin(ctx, act_grp, NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 25, 2);
                if (nk_button_label(ctx, ICON_EDIT " Edit")) {
                    g_show_form = true;
                    g_is_editing = true;
                    g_editing_id = g_users[i].id;
                    strncpy(g_form_txt1, g_users[i].username, sizeof(g_form_txt1) - 1);
                    g_form_txt2[0] = '\0'; // keep empty to not change password
                    strncpy(g_form_txt3, g_users[i].name, sizeof(g_form_txt3) - 1);
                    g_form_role_idx = (int)g_users[i].role;
                }
                if (nk_button_label(ctx, ICON_TRASH " Hapus")) {
                    if (g_users[i].id == g_current_user.id) {
                        show_notification("Tidak bisa menghapus akun sendiri!", false);
                    } else {
                        if (db_delete_user(g_users[i].id)) {
                            show_notification("User berhasil dihapus.", true);
                            load_tab_data(MENU_USERS);
                        } else {
                            show_notification("Gagal menghapus user!", false);
                        }
                    }
                }
                nk_group_end(ctx);
            }
        }
    } else {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, g_is_editing ? "EDIT USER DETAIL" : "DAFTAR USER BARU", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Username Pengguna (Unik)", NK_TEXT_LEFT);
        nk_label(ctx, g_is_editing ? "Password Baru (Kosongkan jika tidak diganti)" : "Password", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_ascii);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt2, sizeof(g_form_txt2), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 22, 2);
        nk_label(ctx, "Nama Lengkap", NK_TEXT_LEFT);
        nk_label(ctx, "Peran / Hak Akses", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt3, sizeof(g_form_txt3), nk_filter_ascii);
        
        const char *role_options[] = {"ADMINISTRATOR (0)", "GURU BINAAN (1)", "STAF TATA USAHA (2)"};
        g_form_role_idx = nk_combo(ctx, role_options, 3, g_form_role_idx, 25, nk_vec2(200, 100));

        nk_layout_row_dynamic(ctx, 20, 1); // Spacer

        nk_layout_row_dynamic(ctx, 35, 2);
        if (nk_button_label(ctx, "SIMPAN")) {
            if (strlen(g_form_txt1) == 0 || strlen(g_form_txt3) == 0 || (!g_is_editing && strlen(g_form_txt2) == 0)) {
                show_notification("Username, Nama, dan Password (untuk user baru) wajib diisi!", false);
            } else {
                User u;
                u.id = g_editing_id;
                strncpy(u.username, g_form_txt1, sizeof(u.username) - 1);
                strncpy(u.password_hash, g_form_txt2, sizeof(u.password_hash) - 1);
                strncpy(u.name, g_form_txt3, sizeof(u.name) - 1);
                u.role = (UserRole)g_form_role_idx;

                bool success = g_is_editing ? db_update_user(&u) : db_create_user(&u);
                if (success) {
                    show_notification("Akun pengguna berhasil disimpan.", true);
                    g_show_form = false;
                    load_tab_data(MENU_USERS);
                } else {
                    show_notification("Gagal menyimpan akun (Username unik)!", false);
                }
            }
        }
        if (nk_button_label(ctx, "BATAL")) g_show_form = false;
    }
}

// SUB-SCREEN: Backup & Restore
static void draw_backup_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, "BACKUP & RESTORE DATABASE SQLITE", NK_TEXT_LEFT, g_theme_text_header);

    nk_layout_row_dynamic(ctx, 20, 1); // Spacer

    int backup_cols = 2;
    int group_height = 160;
    if (screen_width < 600) {
        backup_cols = 1;
        group_height = 150;
    }

    if (backup_cols == 1) nk_layout_row_dynamic(ctx, group_height, 1);
    else nk_layout_row_dynamic(ctx, group_height, 2);

    // Backup Group
    if (nk_group_begin(ctx, "BackupGroup", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label_colored(ctx, "BACKUP DATABASES", NK_TEXT_LEFT, nk_rgb(46, 204, 113));
        nk_label(ctx, "Simpan salinan database ke file cadangan.", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_backup_path, sizeof(g_backup_path), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 10, 1); // Spacer
        nk_layout_row_dynamic(ctx, 35, 1);
        if (nk_button_label(ctx, "PROSES BACKUP")) {
            if (service_backup_db(g_backup_path)) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Backup sukses ke: %s", g_backup_path);
                show_notification(msg, true);
            } else {
                show_notification("Gagal melakukan backup database!", false);
            }
        }
        nk_group_end(ctx);
    }

    if (backup_cols == 1) nk_layout_row_dynamic(ctx, group_height, 1);

    // Restore Group
    if (nk_group_begin(ctx, "RestoreGroup", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label_colored(ctx, "RESTORE DATABASES", NK_TEXT_LEFT, nk_rgb(192, 57, 43));
        nk_label(ctx, "WARNING: Restore akan menimpa database aktif!", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_restore_path, sizeof(g_restore_path), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 10, 1); // Spacer
        nk_layout_row_dynamic(ctx, 35, 1);
        if (nk_button_label(ctx, "PROSES RESTORE")) {
            if (service_restore_db(g_restore_path)) {
                show_notification("Database berhasil di-restore!", true);
                load_tab_data(MENU_DASHBOARD); // reload
            } else {
                show_notification("Gagal me-restore database (Cek file)!", false);
            }
        }
        nk_group_end(ctx);
    }
}

// SUB-SCREEN: Settings Tab
static void draw_settings_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, ICON_SETTINGS "   PENGATURAN SISTEM & INSTANSI (PRO ENTERPRISE)", NK_TEXT_LEFT, g_theme_text_header);

    int cols = screen_width > 900 ? 2 : 1;
    nk_layout_row_dynamic(ctx, 450, cols);

    // Section 1: Profil Sekolah / Instansi
    if (nk_group_begin(ctx, "SchoolProfileGroup", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label_colored(ctx, "PROFIL INSTANSI & KEPALA SEKOLAH", NK_TEXT_LEFT, g_light_mode ? nk_rgb(63, 81, 181) : nk_rgb(30, 136, 229));
        nk_rule_horizontal(ctx, g_theme_border, 1);
        nk_layout_row_dynamic(ctx, 10, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Nama Sekolah / Lembaga:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt1, sizeof(g_form_txt1), nk_filter_default);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "NPSN / Kode Registrasi Resmi:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt2, sizeof(g_form_txt2), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Alamat Instansi / Lokasi Sekolah:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt3, sizeof(g_form_txt3), nk_filter_default);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Nama Kepala Sekolah / Penanggung Jawab:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt4, sizeof(g_form_txt4), nk_filter_default);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "NIP Kepala Sekolah:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt5, sizeof(g_form_txt5), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 15, 1);
        nk_layout_row_dynamic(ctx, 35, 1);
        if (nk_button_label(ctx, "💾 SIMPAN PROFIL INSTANSI")) {
            strncpy(g_school_settings.school_name, g_form_txt1, sizeof(g_school_settings.school_name) - 1);
            strncpy(g_school_settings.school_npsn, g_form_txt2, sizeof(g_school_settings.school_npsn) - 1);
            strncpy(g_school_settings.school_address, g_form_txt3, sizeof(g_school_settings.school_address) - 1);
            strncpy(g_school_settings.principal_name, g_form_txt4, sizeof(g_school_settings.principal_name) - 1);
            strncpy(g_school_settings.principal_nip, g_form_txt5, sizeof(g_school_settings.principal_nip) - 1);
            if (db_save_school_settings(&g_school_settings)) {
                show_notification("Profil instansi berhasil diperbarui!", true);
            } else {
                show_notification("Gagal menyimpan profil instansi!", false);
            }
        }
        nk_group_end(ctx);
    }

    // Section 2: Maintenance & Performance Diagnostics
    if (nk_group_begin(ctx, "MaintenanceGroup", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label_colored(ctx, "PEMELIHARAAN DATABASE & PERFORMA", NK_TEXT_LEFT, g_light_mode ? nk_rgb(63, 81, 181) : nk_rgb(30, 136, 229));
        nk_rule_horizontal(ctx, g_theme_border, 1);
        nk_layout_row_dynamic(ctx, 10, 1);

        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label(ctx, "Tahun Ajaran Aktif (Default):", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, g_form_txt6, sizeof(g_form_txt6), nk_filter_ascii);

        nk_layout_row_dynamic(ctx, 15, 1);
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, "Optimasi & Integritas SQLite:", NK_TEXT_LEFT, g_theme_text_header);
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label_colored(ctx, "Lakukan Vacuum & Reindex untuk mempercepat pencarian data.", NK_TEXT_LEFT, g_theme_text_muted);

        nk_layout_row_dynamic(ctx, 35, 1);
        if (nk_button_label(ctx, "⚡ OPTIMASI DATABASE (VACUUM & REINDEX)")) {
            if (db_execute("VACUUM; REINDEX;")) {
                show_notification("Database SQLite berhasil di-vacuum & dioptimasi!", true);
            } else {
                show_notification("Gagal melakukan optimasi database!", false);
            }
        }

        nk_layout_row_dynamic(ctx, 15, 1);
        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, "Status Buffer & Kapasitas Teks Jurnal:", NK_TEXT_LEFT, g_theme_text_header);
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "✓ Kapasitas Input Jurnal Harian: 65,536 Karakter (64 KB Pro)", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "✓ Kapasitas CP/TP Kurikulum: 32,768 Karakter (32 KB)", NK_TEXT_LEFT);

        nk_group_end(ctx);
    }
}

// SUB-SCREEN: About Tab
static void draw_about_tab(struct nk_context *ctx, int screen_width) {
    nk_layout_row_dynamic(ctx, 30, 1);
    nk_label_colored(ctx, ICON_ABOUT "   TENTANG SOFTWARE DANGERPCA ERP PRO", NK_TEXT_LEFT, g_theme_text_header);

    nk_layout_row_dynamic(ctx, 130, 1);
    if (nk_group_begin(ctx, "AboutBanner", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 35, 1);
        nk_label_colored(ctx, "DANGERPCA SCHOOL ERP ENTERPRISE EDITION v2.5 PRO", NK_TEXT_CENTERED, g_light_mode ? nk_rgb(63, 81, 181) : nk_rgb(30, 136, 229));
        
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Sistem ERP Sekolah Offline Desktop Berkinerja Tinggi & Manajemen Jurnal Akademik Professional", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label_colored(ctx, "STATUS LISENSI: PRO ENTERPRISE UNLIMITED (LIFETIME LICENSE)", NK_TEXT_CENTERED, nk_rgb(39, 174, 96));
        nk_group_end(ctx);
    }

    nk_layout_row_dynamic(ctx, 15, 1); // Spacer

    int cols = screen_width > 900 ? 2 : 1;
    nk_layout_row_dynamic(ctx, 320, cols);

    // Card 1: Spesifikasi & Arsitektur Sistem
    if (nk_group_begin(ctx, "AboutSpecs", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label_colored(ctx, "INFORMASI LISENSI & SPESIFIKASI SISTEM", NK_TEXT_LEFT, g_theme_text_header);
        nk_rule_horizontal(ctx, g_theme_border, 1);
        nk_layout_row_dynamic(ctx, 10, 1);

        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label(ctx, "• Versi Aplikasi: v2.5.0 Pro Enterprise", NK_TEXT_LEFT);
        nk_label(ctx, "• Serial Key: DPCA-ENT-2026-992A-8812-LIFETIME", NK_TEXT_LEFT);
        nk_label(ctx, "• Engine GUI: Nuklear Immediate Mode (GPU Accelerated)", NK_TEXT_LEFT);
        nk_label(ctx, "• Backend Graphics: SDL2 Renderer Engine (60 FPS)", NK_TEXT_LEFT);
        nk_label(ctx, "• Database Engine: SQLite Embedded Engine v3.x", NK_TEXT_LEFT);
        nk_label(ctx, "• Bahasa Pemrograman: Pure C99 High-Speed Native", NK_TEXT_LEFT);
        nk_label(ctx, "• Kompatibilitas Platform: Linux (DEB/Binary) & Windows x64", NK_TEXT_LEFT);
        nk_group_end(ctx);
    }

    // Card 2: Keunggulan & Fitur Unggulan Pro
    if (nk_group_begin(ctx, "AboutFeatures", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label_colored(ctx, "KEUNGGULAN FITUR ENTERPRISE PRO", NK_TEXT_LEFT, g_theme_text_header);
        nk_rule_horizontal(ctx, g_theme_border, 1);
        nk_layout_row_dynamic(ctx, 10, 1);

        nk_layout_row_dynamic(ctx, 22, 1);
        nk_label(ctx, "✓ Jurnal Pembelajaran Tanpa Batas (Text Buffer up to 64 KB)", NK_TEXT_LEFT);
        nk_label(ctx, "✓ Editor Rich Text WYSIWYG & Visual Live Preview Sync", NK_TEXT_LEFT);
        nk_label(ctx, "✓ Manajemen Kurikulum Merdeka (CP, TP & ATP)", NK_TEXT_LEFT);
        nk_label(ctx, "✓ Pencatatan Absensi Harian & Nilai Transkrip Rapor", NK_TEXT_LEFT);
        nk_label(ctx, "✓ Generator Laporan Otomatis Ekspor PDF & Excel CSV", NK_TEXT_LEFT);
        nk_label(ctx, "✓ Keamanan Data 100% Offline Tanpa Ketergantungan Internet", NK_TEXT_LEFT);
        nk_label(ctx, "✓ Backup & Restore Instant Sekali Klik", NK_TEXT_LEFT);
        nk_group_end(ctx);
    }
}

// Footer Status Bar
static void draw_footer(struct nk_context *ctx) {
    if (nk_group_begin(ctx, "Footer", NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_template_begin(ctx, 20);
        nk_layout_row_template_push_static(ctx, 240); // System status
        nk_layout_row_template_push_dynamic(ctx);     // Center info
        nk_layout_row_template_push_static(ctx, 180); // Database path info
        nk_layout_row_template_end(ctx);

        nk_label_colored(ctx, "  Status: Terhubung (Offline Mode)", NK_TEXT_LEFT, nk_rgb(39, 174, 96));
        nk_label_colored(ctx, "DangerPCA ERP v2.5 Enterprise Pro", NK_TEXT_CENTERED, g_theme_text_muted);
        nk_label(ctx, "DB: school_erp.db   ", NK_TEXT_RIGHT);
        nk_group_end(ctx);
    }
}

// Main Draw Dispatcher
void ui_render(struct nk_context *ctx, int screen_width, int screen_height) {
    if (!g_logged_in) {
        draw_login_screen(ctx, screen_width, screen_height);
        draw_notification(ctx, screen_width);
        return;
    }

    // Full screen Main ERP frame window
    struct nk_rect full_bounds = nk_rect(0, 0, screen_width, screen_height);
    if (nk_begin(ctx, "DangerPCA_ERP", full_bounds, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        
        nk_layout_row_template_begin(ctx, screen_height - 35);
        nk_layout_row_template_push_static(ctx, 180); // Left Sidebar
        nk_layout_row_template_push_dynamic(ctx);     // Right Main Workspace
        nk_layout_row_template_end(ctx);

        // Column 1: Sidebar Navigation
        draw_sidebar(ctx, screen_height - 35);

        // Column 2: Main Workspace Group
        if (nk_group_begin(ctx, "WorkspaceGroup", NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 40, 1);
            draw_topbar(ctx);

            nk_layout_row_dynamic(ctx, 10, 1); // Spacer
            nk_layout_row_dynamic(ctx, screen_height - 110, 1);
            if (nk_group_begin(ctx, "ContentPane", NK_WINDOW_BORDER)) {
                switch (g_current_menu) {
                    case MENU_DASHBOARD: draw_dashboard(ctx, screen_width); break;
                    case MENU_STUDENTS: draw_students_tab(ctx, screen_width); break;
                    case MENU_TEACHERS: draw_teachers_tab(ctx, screen_width); break;
                    case MENU_CLASSES: draw_classes_tab(ctx, screen_width); break;
                    case MENU_ATTENDANCE: draw_attendance_tab(ctx, screen_width); break;
                    case MENU_ACADEMIC: draw_academic_tab(ctx, screen_width); break;
                    case MENU_JOURNAL: draw_journal_tab(ctx, screen_width); break;
                    case MENU_GRADES: draw_grades_tab(ctx, screen_width); break;
                    case MENU_USERS: draw_users_tab(ctx, screen_width); break;
                    case MENU_BACKUP: draw_backup_tab(ctx, screen_width); break;
                    case MENU_SETTINGS: draw_settings_tab(ctx, screen_width); break;
                    case MENU_ABOUT: draw_about_tab(ctx, screen_width); break;
                }
                nk_group_end(ctx);
            }
            nk_group_end(ctx);
        }

        // Draw Footer Status Bar
        nk_layout_row_dynamic(ctx, 25, 1);
        draw_footer(ctx);
    }
    nk_end(ctx);

    // Draw overlay notification toast
    draw_notification(ctx, screen_width);
}
