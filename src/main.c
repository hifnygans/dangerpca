#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"
#undef NK_IMPLEMENTATION
#undef NK_SDL_RENDERER_IMPLEMENTATION

#include "database/db.h"
#include "ui/ui.h"

int main(int argc, char *argv[]) {
    // 1. Initialize SQLite database and migrations
    const char *db_path = "school_erp.db";
    printf("Initializing database: %s\n", db_path);
    if (!db_init(db_path)) {
        fprintf(stderr, "Failed to initialize database!\n");
        return EXIT_FAILURE;
    }

    // 2. Initialize SDL2
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        db_close();
        return EXIT_FAILURE;
    }

    // 3. Create Windows Window
    SDL_Window *window = SDL_CreateWindow(
        "DangerPCA School ERP (Offline Desktop App)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        db_close();
        return EXIT_FAILURE;
    }

    // 4. Create SDL2 Hardware Accelerated Renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        db_close();
        return EXIT_FAILURE;
    }

    // 5. Initialize Nuklear SDL Renderer Context
    struct nk_context *ctx = nk_sdl_init(window, renderer);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize Nuklear context!\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        db_close();
        return EXIT_FAILURE;
    }

    // 6. Load TrueType Font (Roboto & FontAwesome) with robust path fallback
        struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    
    struct nk_font *roboto = NULL;
    struct nk_font *fa = NULL;
    struct nk_font_config cfg = nk_font_config(15);
    static const nk_rune fa_range[] = {0xf000, 0xf900, 0};
    struct nk_font_config fa_cfg = nk_font_config(14);
    fa_cfg.merge_mode = nk_true;
    fa_cfg.range = fa_range;

    // Fallback 1: Local assets folder (current working directory)
    printf("Font Load Fallback 1: Checking local assets folder...\n");
    roboto = nk_font_atlas_add_from_file(atlas, "assets/Roboto-Regular.ttf", 15, &cfg);
    if (roboto) {
        printf("Successfully loaded assets/Roboto-Regular.ttf\n");
        fa = nk_font_atlas_add_from_file(atlas, "assets/fa-solid-900.ttf", 14, &fa_cfg);
        if (fa) {
            printf("Successfully merged assets/fa-solid-900.ttf\n");
        } else {
            printf("Failed to load assets/fa-solid-900.ttf!\n");
        }
    }
    
    // Fallback 2: Executable relative path
    if (!roboto) {
        char *base_path = SDL_GetBasePath();
        if (base_path) {
            char font_path[512];
            char fa_path[512];
            snprintf(font_path, sizeof(font_path), "%sassets/Roboto-Regular.ttf", base_path);
            printf("Font Load Fallback 2: Checking base path: %s\n", font_path);
            roboto = nk_font_atlas_add_from_file(atlas, font_path, 15, &cfg);
            if (roboto) {
                printf("Successfully loaded %s\n", font_path);
                snprintf(fa_path, sizeof(fa_path), "%sassets/fa-solid-900.ttf", base_path);
                fa = nk_font_atlas_add_from_file(atlas, fa_path, 14, &fa_cfg);
                if (fa) {
                    printf("Successfully merged %s\n", fa_path);
                } else {
                    printf("Failed to load %s!\n", fa_path);
                }
            }
            
            // Fallback 3: Installed Debian path (relative to /usr/bin)
            if (!roboto) {
                snprintf(font_path, sizeof(font_path), "%s../share/dangerpca/assets/Roboto-Regular.ttf", base_path);
                printf("Font Load Fallback 3: Checking relative share path: %s\n", font_path);
                roboto = nk_font_atlas_add_from_file(atlas, font_path, 15, &cfg);
                if (roboto) {
                    printf("Successfully loaded %s\n", font_path);
                    snprintf(fa_path, sizeof(fa_path), "%s../share/dangerpca/assets/fa-solid-900.ttf", base_path);
                    fa = nk_font_atlas_add_from_file(atlas, fa_path, 14, &fa_cfg);
                    if (fa) {
                        printf("Successfully merged %s\n", fa_path);
                    } else {
                        printf("Failed to load %s!\n", fa_path);
                    }
                }
            }
            SDL_free(base_path);
        }
    }
    
    // Fallback 4: Absolute Linux installation directory
    if (!roboto) {
        printf("Font Load Fallback 4: Checking absolute share path /usr/share/dangerpca/assets/...\n");
        roboto = nk_font_atlas_add_from_file(atlas, "/usr/share/dangerpca/assets/Roboto-Regular.ttf", 15, &cfg);
        if (roboto) {
            printf("Successfully loaded absolute path Roboto\n");
            fa = nk_font_atlas_add_from_file(atlas, "/usr/share/dangerpca/assets/fa-solid-900.ttf", 14, &fa_cfg);
            if (fa) {
                printf("Successfully merged absolute path FontAwesome\n");
            } else {
                printf("Failed to load absolute path FontAwesome!\n");
            }
        }
    }

    nk_sdl_font_stash_end();
    if (roboto) {
        nk_style_set_font(ctx, &roboto->handle);
    } else {
        printf("Warning: Font assets/Roboto-Regular.ttf not loaded, using fallback font.\n");
    }

    // 7. Initialize UI State and Theme
    ui_init(ctx);

    // 8. Main Application Loop
    bool running = true;
    while (running) {
        SDL_Event event;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            nk_sdl_handle_event(&event);
        }
        nk_input_end(ctx);

        // Get window actual size
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        // Render immediate mode UI
        ui_render(ctx, width, height);

        // Clear screen and draw Nuklear layers
        SDL_SetRenderDrawColor(renderer, 20, 24, 33, 255);
        SDL_RenderClear(renderer);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(renderer);
    }

    // 9. Cleanup & Shutdown
    printf("Shutting down ERP application...\n");
    ui_cleanup();
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    db_close();

    return EXIT_SUCCESS;
}
