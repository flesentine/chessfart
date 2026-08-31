#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "audio_game.h"
#include "board_view_build7.h"
#include "cpu.h"
#include "font.h"
#include "input_build5.h"
#include "persistence.h"
#include "persistence_ui.h"
#include "vga.h"

static CfCpuConfig g_cpu_config;
static CfCpuStats g_cpu_stats;
static CfGasHistory g_cpu_search_history;
static CfGasHistory g_cpu_history_backup;
static char g_cpu_message[32];
static int g_cpu_message_pending;

int cpu_title_screen(const char *config_path);
void cpu_render_game(const CfBoard *, const CfGasState *, int, int, int, int, int,
                     const CfMoveList *, CfGameStatus, int, CfPieceType, int,
                     CfFartDirection, CfFartPreview, int, CfPieceType, const char *);
CfPersistenceResult cpu_load_game(const char *, CfBoard *, CfGasState *, CfGasHistory *);
int cpu_human_make_move(CfBoard *, CfGasState *, int, int, int, int, CfGasMove *);
int cpu_human_make_move_ex(CfBoard *, CfGasState *, int, int, int, int,
                           CfPieceType, CfGasMove *);
int cpu_human_make_fart(CfBoard *, CfGasState *, int, int, CfFartDirection,
                        CfPieceType, CfFartAction *);

#define persistence_title_screen cpu_title_screen
#define persistence_render_game cpu_render_game
#define persistence_load_game cpu_load_game
#define audio_game_make_move cpu_human_make_move
#define audio_game_make_move_ex cpu_human_make_move_ex
#define audio_game_make_fart cpu_human_make_fart
#define main chessfart_build9_cpu_main
#include "main_build9.c"
#undef main
#undef audio_game_make_fart
#undef audio_game_make_move_ex
#undef audio_game_make_move
#undef persistence_load_game
#undef persistence_render_game
#undef persistence_title_screen

#include "main_build10_hooks.inc"
#include "main_build10_ui.inc"

int main(void)
{
    cpu_config_for_difficulty(&g_cpu_config, CF_CPU_MEDIUM);
    memset(&g_cpu_stats, 0, sizeof(g_cpu_stats));
    g_cpu_message[0] = '\0';
    g_cpu_message_pending = 0;
    return chessfart_build9_cpu_main();
}
