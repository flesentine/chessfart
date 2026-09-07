#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "audio_game.h"
#include "board_view.h"
#include "cpu.h"
#include "font.h"
#include "input.h"
#include "mouse.h"
#include "persistence.h"
#include "presentation.h"
#include "practice_undo.h"
#include "replay.h"
#include "replay_file.h"
#include "ui_layout.h"
#include "ui_theme.h"
#include "ux.h"
#include "vga.h"

static CfCpuConfig g_cpu_config;
static CfCpuStats g_cpu_stats;
static CfMatchMode g_match_mode;
static int g_practice_mode;
static CfPracticeUndoJournal g_practice_undo;

typedef struct CfPracticePresentationUndoEntry {
    CfUxHistoryDelta history;
    CfReplayTimelineDelta replay;
} CfPracticePresentationUndoEntry;

typedef struct CfPracticePresentationUndoJournal {
    CfPracticePresentationUndoEntry entries[CF_PRACTICE_UNDO_CAPACITY];
    int start;
    int count;
} CfPracticePresentationUndoJournal;

typedef char CfPracticePresentationUndoJournalFits[
    sizeof(CfPracticePresentationUndoJournal) < 8192U ? 1 : -1];
typedef char CfPracticeCombinedUndoJournalsFit[
    sizeof(CfPracticePresentationUndoJournal) +
    sizeof(CfPracticeUndoJournal) < 16384U ? 1 : -1];

static CfPracticePresentationUndoJournal g_practice_presentation_undo;
static CfGasHistory g_cpu_search_history;
static CfGasHistory g_cpu_history_backup;
static char g_cpu_message[32];
static int g_cpu_message_pending;
static CfUxHistory g_ux_history;
static CfReplayTimeline *g_replay_timeline;
static int g_replay_viewer_active;
static int g_replay_viewer_index;
static char g_replay_file_notice[24];

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
int ux_practice_undo_game(CfBoard *, CfGasState *, CfGasHistory *);
CfPersistenceResult ux_load_config(const char *, CfAudioConfig *);
CfPersistenceResult ux_save_config(const char *, const CfAudioConfig *);
CfInputKey ux_poll_key(void);
void ux_show_help_modal(void);
void ux_show_history_modal(void);
void ux_show_credits_modal(void);
void ux_show_replay_modal(void);

#include "main_loop.inc"

#include "main_hooks.inc"
#include "main_ui.inc"

#ifdef CF_WEB_REVIEW
#include "main_review.inc"
#endif

int main(void)
{
    int result;
    cpu_config_for_difficulty(&g_cpu_config, CF_CPU_MEDIUM);
    (void)ui_theme_set(CF_UI_THEME_ROYAL_BASEMENT);
    g_match_mode = CF_MATCH_CPU;
    g_practice_mode = 0;
    practice_undo_init(&g_practice_undo);
    ux_practice_presentation_init();
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
    result = game_loop();
    mouse_shutdown();
    if (g_replay_timeline != 0) free(g_replay_timeline);
    g_replay_timeline = 0;
    return result;
}
