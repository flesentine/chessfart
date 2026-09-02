#include <stdio.h>

#include "cpu_internal.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static void test_fart_edge_bonus_alpha_beta_window(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;

    board_clear(&board);
    gas_init(&gas);

    board_set_piece(&board, 4, 0, CF_PIECE_KING,   CF_COLOR_WHITE);
    board_set_piece(&board, 7, 0, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 1, CF_PIECE_ROOK,   CF_COLOR_WHITE);
    board_set_piece(&board, 3, 4, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    board_set_piece(&board, 1, 5, CF_PIECE_PAWN,   CF_COLOR_BLACK);
    board_set_piece(&board, 2, 5, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 5, 5, CF_PIECE_ROOK,   CF_COLOR_BLACK);
    board_set_piece(&board, 0, 7, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    board_set_piece(&board, 4, 7, CF_PIECE_KING,   CF_COLOR_BLACK);
    board_set_piece(&board, 6, 7, CF_PIECE_BISHOP, CF_COLOR_WHITE);

    gas_set(&gas, 4, 0, 2U);
    gas_set(&gas, 7, 0, 3U);
    gas_set(&gas, 3, 4, 3U);
    gas_set(&gas, 1, 5, 1U);
    gas_set(&gas, 2, 5, 2U);
    gas_set(&gas, 5, 5, 1U);
    gas_set(&gas, 4, 7, 1U);
    gas_set(&gas, 6, 7, 2U);

    board.side_to_move = CF_COLOR_BLACK;
    board.castling_rights = 0U;
    board.en_passant_file = -1;
    board.en_passant_rank = -1;
    board.halfmove_clock = 0U;
    board.fullmove_number = 1U;
    gas_history_init(&history, &board, &gas);

    cpu_config_for_difficulty(&config, CF_CPU_HARD);
    config.max_depth = 3;
    config.node_budget = 0UL;
    config.time_limit_ms = 0UL;

    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_MOVE);
    CHECK(action.from_file == 4 && action.from_rank == 7);
    CHECK(action.to_file == 3 && action.to_rank == 6);
    CHECK(stats.depth_completed == 3);

    /* The pre-v8 unshifted Fart edge window scores this position -339.
       The correct [bonus-beta, bonus-alpha] child window scores it -304. */
    CHECK(stats.score == -304);
}

int main(void)
{
    test_fart_edge_bonus_alpha_beta_window();
    if (failures != 0) {
        printf("Build 10 alpha-beta regression failed: %d\n", failures);
        return 1;
    }
    printf("Build 10 alpha-beta regression passed.\n");
    return 0;
}
