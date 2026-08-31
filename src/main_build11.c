#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "audio_game.h"
#include "board_view_build7.h"
#include "cpu.h"
#include "font.h"
#include "input_build5.h"
#include "mouse.h"
#include "persistence.h"
#include "persistence_ui.h"
#include "presentation.h"
#include "ux.h"
#include "vga.h"

static CfCpuConfig g_cpu_config;
static CfCpuStats g_cpu_stats;
static CfGasHistory g_cpu_search_history;
static CfGasHistory g_cpu_history_backup;
static char g_cpu_message[32];
static int g_cpu_message_pending;
static CfUxHistory g_ux_history;

typedef struct CfUxRenderCache {
    int valid;
    CfBoard board;
    CfGasState gas;
    int cursor_file;
    int cursor_rank;
    int has_selection;
    int selected_file;
    int selected_rank;
    CfMoveList moves;
    CfGameStatus status;
    int promotion_pending;
    CfPieceType promotion_choice;
    int fart_mode;
    CfFartDirection fart_direction;
    CfFartPreview fart_preview;
    int fart_promotion_pending;
    CfPieceType fart_promotion_choice;
    char message[32];
} CfUxRenderCache;

static CfUxRenderCache g_ux_cache;
static unsigned g_ux_mouse_buttons;
static int g_ux_mouse_target_active;
static int g_ux_mouse_target_file;
static int g_ux_mouse_target_rank;
static int g_ux_mouse_confirm_pending;

int ux_title_screen(const char *config_path);
void ux_render_game(const CfBoard *, const CfGasState *, int, int, int, int, int,
                    const CfMoveList *, CfGameStatus, int, CfPieceType, int,
                    CfFartDirection, CfFartPreview, int, CfPieceType, const char *);
CfPersistenceResult ux_load_game(const char *, CfBoard *, CfGasState *, CfGasHistory *);
CfPersistenceResult ux_save_game(const char *, const CfBoard *, const CfGasState *,
                                 const CfGasHistory *);
int ux_human_make_move(CfBoard *, CfGasState *, int, int, int, int, CfGasMove *);
int ux_human_make_move_ex(CfBoard *, CfGasState *, int, int, int, int,
                          CfPieceType, CfGasMove *);
int ux_human_make_fart(CfBoard *, CfGasState *, int, int, CfFartDirection,
                       CfPieceType, CfFartAction *);
CfInputKey5 ux_poll_key(void);
void ux_show_help_modal(void);
void ux_show_history_modal(void);
void ux_show_credits_modal(void);

#define persistence_title_screen ux_title_screen
#define persistence_render_game ux_render_game
#define persistence_load_game ux_load_game
#define persistence_save_game ux_save_game
#define audio_game_make_move ux_human_make_move
#define audio_game_make_move_ex ux_human_make_move_ex
#define audio_game_make_fart ux_human_make_fart
#define input5_poll_key ux_poll_key
#define main chessfart_build9_ux_main
#include "main_build9.c"
#undef main
#undef input5_poll_key
#undef audio_game_make_fart
#undef audio_game_make_move_ex
#undef audio_game_make_move
#undef persistence_save_game
#undef persistence_load_game
#undef persistence_render_game
#undef persistence_title_screen

#include "main_build11_hooks.inc"

/* Keep Build 11 functionality while presenting a retail-style panel instead of
   search/debug instrumentation. This wrapper only affects the Build 11 UI
   overlay; the bitmap font and underlying renderers are unchanged. */
static void ux_art_font_draw_text(int x, int y, const char *text,
                                  cf_u8 color, int scale)
{
    char clean[48];
    const char *cut;
    size_t n;

    if (text == 0) return;
    if (strncmp(text, "CUT ", 4) == 0) return;

    if (strncmp(text, "CPU ", 4) == 0) {
        cut = strstr(text + 4, " D");
        if (cut != 0) {
            n = (size_t)(cut - text);
            if (n >= sizeof(clean)) n = sizeof(clean) - 1U;
            memcpy(clean, text, n);
            clean[n] = '\0';
            font_draw_text(x, y, clean, color, scale);
            return;
        }
    }

    if (strncmp(text, "BUILD 11  H HELP", 16) == 0) {
        font_draw_text(x, y, "H HELP  M LOG  C CREDITS  S/L SAVE",
                       color, scale);
        return;
    }
    if (strncmp(text, "BUILD 11  D CPU", 15) == 0) {
        font_draw_text(x, y, "D CPU  F SFX  H HELP  MOUSE OPTIONAL",
                       color, scale);
        return;
    }
    if (strncmp(text, "BUILD 11  TERMINAL", 18) == 0) {
        font_draw_text(x, y, "GAME OVER  H HELP  M LOG",
                       color, scale);
        return;
    }
    font_draw_text(x, y, text, color, scale);
}

#define font_draw_text ux_art_font_draw_text
#include "main_build11_ui.inc"
#undef font_draw_text

int main(void)
{
    int result;
    cpu_config_for_difficulty(&g_cpu_config, CF_CPU_MEDIUM);
    memset(&g_cpu_stats, 0, sizeof(g_cpu_stats));
    memset(&g_ux_cache, 0, sizeof(g_ux_cache));
    ux_history_init(&g_ux_history);
    g_cpu_message[0] = '\0';
    g_cpu_message_pending = 0;
    g_ux_mouse_buttons = 0U;
    g_ux_mouse_target_active = 0;
    g_ux_mouse_confirm_pending = 0;
    (void)mouse_init();
    result = chessfart_build9_ux_main();
    mouse_shutdown();
    return result;
}
