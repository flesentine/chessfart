#include <stdio.h>
#include <stdlib.h>
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
#include "replay.h"
#include "ux.h"
#include "vga.h"

static CfCpuConfig g_cpu_config;
static CfCpuStats g_cpu_stats;
static CfMatchMode g_match_mode;
static CfGasHistory g_cpu_search_history;
static CfGasHistory g_cpu_history_backup;
static char g_cpu_message[32];
static int g_cpu_message_pending;
static CfUxHistory g_ux_history;
static CfReplayTimeline *g_replay_timeline;
static int g_replay_viewer_active;
static int g_replay_viewer_index;

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
static int g_ux_mouse_fart_pending;

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
void ux_show_replay_modal(void);

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
#include "main_build11_ui.inc"

#ifdef CF_WEB_REVIEW
#include "main_build11_review.inc"
#endif

int main(void)
{
    int result;
    if (0) ux_stamp_build11();
    cpu_config_for_difficulty(&g_cpu_config, CF_CPU_MEDIUM);
    g_match_mode = CF_MATCH_CPU;
    memset(&g_cpu_stats, 0, sizeof(g_cpu_stats));
    memset(&g_ux_cache, 0, sizeof(g_ux_cache));
    ux_history_init(&g_ux_history);
    g_replay_timeline = (CfReplayTimeline *)malloc(sizeof(CfReplayTimeline));
    if (g_replay_timeline != 0) replay_timeline_init(g_replay_timeline);
    g_cpu_message[0] = '\0';
    g_cpu_message_pending = 0;
    g_replay_viewer_active = 0;
    g_replay_viewer_index = -1;
    g_ux_mouse_buttons = 0U;
    g_ux_mouse_target_active = 0;
    g_ux_mouse_confirm_pending = 0;
    g_ux_mouse_fart_pending = 0;
    (void)mouse_init();
    result = chessfart_build9_ux_main();
    mouse_shutdown();
    if (g_replay_timeline != 0) free(g_replay_timeline);
    g_replay_timeline = 0;
    return result;
}
