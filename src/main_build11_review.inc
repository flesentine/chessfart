#include <emscripten/emscripten.h>
#include "cpu_internal.h"

static CfCpuAction g_review_action;
static int g_review_action_valid;
static int g_review_score;

static void review_reset_common(void)
{
    board_clear(&g_board);
    gas_init(&g_gas);
    memset(&g_review_action, 0, sizeof(g_review_action));
    g_review_action_valid = 0;
    g_review_score = 0;
    g_board.en_passant_file = -1;
    g_board.en_passant_rank = -1;
    g_board.halfmove_clock = 0U;
    g_board.fullmove_number = 1U;
}

static void review_setup_tactic(int tactic)
{
    review_reset_common();

    if (tactic == 1) {
        /* White is in rook check. A charged knight can push the checking rook away. */
        board_set_piece(&g_board, 1, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 0, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
        board_set_piece(&g_board, 1, 2, CF_PIECE_ROOK, CF_COLOR_BLACK);
        gas_set(&g_gas, 0, 2, 3U);
        g_board.side_to_move = CF_COLOR_WHITE;
    } else if (tactic == 2) {
        /* Push our own e2 pawn aside to reveal the rook on e8 against the white king. */
        board_set_piece(&g_board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
        board_set_piece(&g_board, 4, 1, CF_PIECE_PAWN, CF_COLOR_BLACK);
        board_set_piece(&g_board, 3, 1, CF_PIECE_ROOK, CF_COLOR_BLACK);
        board_set_piece(&g_board, 3, 0, CF_PIECE_BISHOP, CF_COLOR_BLACK);
        gas_set(&g_gas, 3, 1, 3U);
        g_board.side_to_move = CF_COLOR_BLACK;
    } else if (tactic == 3) {
        /* Push the white king off e1 and destroy both white castling rights. */
        board_set_piece(&g_board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 0, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 3, 0, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
        gas_set(&g_gas, 3, 0, 3U);
        g_board.castling_rights = CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN;
        g_board.side_to_move = CF_COLOR_BLACK;
    } else if (tactic == 4) {
        /* Black pawn cannot advance normally, but a SE Fart promotes it on f1. */
        board_set_piece(&g_board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 3, 2, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
        board_set_piece(&g_board, 4, 1, CF_PIECE_PAWN, CF_COLOR_BLACK);
        board_set_piece(&g_board, 4, 0, CF_PIECE_BISHOP, CF_COLOR_BLACK);
        gas_set(&g_gas, 3, 2, 3U);
        g_board.side_to_move = CF_COLOR_BLACK;
    }

    gas_history_init(&g_history, &g_board, &g_gas);
}

static int review_target_action(int tactic, CfCpuAction *out)
{
    CfCpuActionList list;
    int from_file;
    int from_rank;
    int direction;
    int i;

    if (out == 0) return 0;
    if (tactic == 1) { from_file = 0; from_rank = 2; direction = CF_FART_E; }
    else if (tactic == 2) { from_file = 3; from_rank = 1; direction = CF_FART_E; }
    else if (tactic == 3) { from_file = 3; from_rank = 0; direction = CF_FART_E; }
    else if (tactic == 4) { from_file = 3; from_rank = 2; direction = CF_FART_SE; }
    else return 0;

    cpu_generate_actions(&g_board, &g_gas, &list);
    for (i = 0; i < list.count; ++i) {
        if (list.actions[i].type != CF_CPU_ACTION_FART ||
            list.actions[i].from_file != from_file ||
            list.actions[i].from_rank != from_rank ||
            (int)list.actions[i].direction != direction) continue;
        if (tactic == 4 && list.actions[i].promotion != CF_PIECE_QUEEN) continue;
        *out = list.actions[i];
        return 1;
    }
    return 0;
}

static int review_target_metric(int tactic, int want_bonus)
{
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuUndo undo;
    int actor_was_in_check;
    int bonus;
    int score;

    review_setup_tactic(tactic);
    if (!review_target_action(tactic, &action)) return -32000;
    cpu_config_for_difficulty(&config, CF_CPU_HARD);
    actor_was_in_check = board_is_in_check(&g_board, g_board.side_to_move);
    if (!cpu_apply_action(&g_board, &g_gas, &action, &undo)) return -32000;
    bonus = cpu_internal_action_bonus(&g_board, &g_gas, &action,
                                      &undo, &config, actor_was_in_check);
    score = -cpu_internal_evaluate(&g_board, &g_gas) + bonus;
    cpu_unapply_action(&g_board, &g_gas, &undo);
    return want_bonus ? bonus : score;
}

EMSCRIPTEN_KEEPALIVE int cf_review_run_tactic(int tactic)
{
    CfCpuConfig config;
    CfCpuStats stats;
    CfCpuUndo undo;

    review_setup_tactic(tactic);
    cpu_config_for_difficulty(&config, CF_CPU_HARD);
    config.max_depth = 2;
    config.node_budget = 0UL;
    config.time_limit_ms = 0UL;

    if (!cpu_choose_action(&g_board, &g_gas, &g_history,
                           &config, &g_review_action, &stats)) return 0;
    g_review_action_valid = 1;
    g_review_score = stats.score;
    if (!cpu_apply_action(&g_board, &g_gas, &g_review_action, &undo)) return -1;
    gas_history_record(&g_history, &g_board, &g_gas);
    return 1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_target_score(int tactic)
{
    return review_target_metric(tactic, 0);
}

EMSCRIPTEN_KEEPALIVE int cf_review_target_bonus(int tactic)
{
    return review_target_metric(tactic, 1);
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_type(void)
{
    return g_review_action_valid ? (int)g_review_action.type : 0;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_from_file(void)
{
    return g_review_action_valid ? g_review_action.from_file : -1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_from_rank(void)
{
    return g_review_action_valid ? g_review_action.from_rank : -1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_to_file(void)
{
    return g_review_action_valid ? g_review_action.to_file : -1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_to_rank(void)
{
    return g_review_action_valid ? g_review_action.to_rank : -1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_direction(void)
{
    return g_review_action_valid ? (int)g_review_action.direction : -1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_fart_result(void)
{
    return g_review_action_valid ? (int)g_review_action.fart_result : -1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_promotion(void)
{
    return g_review_action_valid ? (int)g_review_action.promotion : 0;
}

EMSCRIPTEN_KEEPALIVE int cf_review_action_score(void)
{
    return g_review_score;
}

EMSCRIPTEN_KEEPALIVE int cf_review_check(int color)
{
    return board_is_in_check(&g_board, (CfPieceColor)color);
}

EMSCRIPTEN_KEEPALIVE int cf_review_castling_rights(void)
{
    return (int)g_board.castling_rights;
}

EMSCRIPTEN_KEEPALIVE int cf_review_piece_code(int file, int rank)
{
    const CfPiece *piece;
    int gas;
    piece = board_piece_at(&g_board, file, rank);
    if (piece == 0 || piece->type == CF_PIECE_NONE) return 0;
    gas = (int)gas_at(&g_gas, file, rank);
    return ((int)piece->type & 15) |
           (((int)piece->color & 15) << 4) |
           ((gas & 15) << 8);
}
