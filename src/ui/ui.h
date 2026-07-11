#ifndef UI_H
#define UI_H

#include "nuklear.h"

typedef enum {
    MENU_DASHBOARD,
    MENU_STUDENTS,
    MENU_TEACHERS,
    MENU_CLASSES,
    MENU_ATTENDANCE,
    MENU_ACADEMIC,
    MENU_JOURNAL,
    MENU_GRADES,
    MENU_USERS,
    MENU_BACKUP
} SidebarMenu;

// Initialize UI state
void ui_init(struct nk_context *ctx);

// Main rendering routine for the immediate-mode GUI
void ui_render(struct nk_context *ctx, int screen_width, int screen_height);

// Cleanup UI resources
void ui_cleanup(void);

#endif // UI_H
