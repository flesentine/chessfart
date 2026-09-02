#include <stdio.h>
#include <string.h>

#include "cpu_internal.h"

#define SCAN_INF 32000
#define SCAN_MATE 30000
#define SCAN_PLY 5

static CfCpuActionList old_lists[SCAN_PLY];

static int same_action(const CfCpuAction *a, const CfCpuAction *b)
{
    return a->type == b->type &&
           a->from_file == b->from_file &&
           a->from_rank == b->from_rank &&
           a->to_file == b->to_file &&
           a->to_rank == b->to_rank &&
           a->promotion == b->promotion &&
           a->direction == b->direction &&
           a->fart_result == b->fart_result;
}

static int old_negamax(CfBoard *board, CfGasState *gas,
                       const CfCpuConfig *config,
                       int depth, int alpha, int beta, int ply)
{
    CfCpuActionList *list;
    CfCpuUndo undo;
    int best;
    int score;
    int bonus;
    int actor_was_in_check;
    int i;

    if (board->halfmove_clock >= 100U || board_is_insufficient_material(board))
        return 0;
    if (ply >= SCAN_PLY) return cpu_internal_evaluate(board, gas);

    list = &old_lists[ply];
    cpu_generate_actions(board, gas, list);
    if (list->count == 0) {
        if (board_is_in_check(board, board->side_to_move))
            return -SCAN_MATE + ply;
        return 0;
    }
    if (depth <= 0) return cpu_internal_evaluate(board, gas);

    cpu_internal_sort_actions(list);
    actor_was_in_check = board_is_in_check(board, board->side_to_move);
    best = -SCAN_INF;
    for (i = 0; i < list->count; ++i) {
        if (!cpu_apply_action(board, gas, &list->actions[i], &undo)) continue;
        bonus = cpu_internal_action_bonus(board, gas, &list->actions[i],
                                          &undo, config,
                                          actor_was_in_check);
        /* Deliberately reproduces the pre-v8 bug: the child window is not
           shifted by the edge bonus. */
        score = -old_negamax(board, gas, config, depth - 1,
                             -beta, -alpha, ply + 1) + bonus;
        cpu_unapply_action(board, gas, &undo);
        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }
    return best == -SCAN_INF ? cpu_internal_evaluate(board, gas) : best;
}

static int old_choose_action(CfBoard *board, CfGasState *gas,
                             const CfGasHistory *history,
                             const CfCpuConfig *config,
                             CfCpuAction *chosen, int *chosen_score)
{
    CfCpuActionList list;
    CfCpuUndo undo;
    CfGameStatus status;
    int actor_was_in_check;
    int best_score;
    int score;
    int bonus;
    int i;
    int found;

    status = gas_game_status(board, gas, history);
    if (status == CF_GAME_CHECKMATE || status == CF_GAME_STALEMATE ||
        status == CF_GAME_DRAW_FIFTY_MOVE ||
        status == CF_GAME_DRAW_THREEFOLD ||
        status == CF_GAME_DRAW_INSUFFICIENT) return 0;

    cpu_generate_actions(board, gas, &list);
    if (list.count == 0) return 0;
    cpu_internal_sort_actions(&list);
    actor_was_in_check = board_is_in_check(board, board->side_to_move);
    best_score = -SCAN_INF;
    found = 0;

    for (i = 0; i < list.count; ++i) {
        if (!cpu_apply_action(board, gas, &list.actions[i], &undo)) continue;
        bonus = cpu_internal_action_bonus(board, gas, &list.actions[i],
                                          &undo, config,
                                          actor_was_in_check);
        score = -old_negamax(board, gas, config, config->max_depth - 1,
                             -SCAN_INF, SCAN_INF, 1) + bonus;
        cpu_unapply_action(board, gas, &undo);
        if (!found || score > best_score) {
            *chosen = list.actions[i];
            best_score = score;
            found = 1;
        }
    }
    if (chosen_score != 0) *chosen_score = best_score;
    return found;
}

static unsigned long scan_rand(unsigned long *state)
{
    *state = (*state * 1664525UL + 1013904223UL) & 0xffffffffUL;
    return *state;
}

static int occupied(const CfBoard *board, int file, int rank)
{
    return board->squares[rank][file].type != CF_PIECE_NONE;
}

static void build_position(CfBoard *board, CfGasState *gas,
                           unsigned long seed)
{
    unsigned long state;
    int i;
    int file;
    int rank;
    CfPieceType type;
    CfPieceColor color;
    cf_u8 amount;

    board_clear(board);
    gas_init(gas);
    board_set_piece(board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(board, 4, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    state = seed;

    for (i = 0; i < 8; ++i) {
        do {
            file = (int)(scan_rand(&state) & 7UL);
            rank = (int)((scan_rand(&state) >> 3) & 7UL);
        } while (occupied(board, file, rank));
        type = (CfPieceType)(CF_PIECE_PAWN +
               (int)(scan_rand(&state) % 5UL));
        color = (scan_rand(&state) & 1UL) != 0UL ?
                CF_COLOR_WHITE : CF_COLOR_BLACK;
        board_set_piece(board, file, rank, type, color);
        amount = (cf_u8)(scan_rand(&state) % 4UL);
        gas_set(gas, file, rank, amount);
    }

    gas_set(gas, 4, 0, (cf_u8)(scan_rand(&state) % 4UL));
    gas_set(gas, 4, 7, (cf_u8)(scan_rand(&state) % 4UL));
    board->side_to_move = (scan_rand(&state) & 1UL) != 0UL ?
                          CF_COLOR_WHITE : CF_COLOR_BLACK;
    board->castling_rights = 0U;
    board->en_passant_file = -1;
    board->en_passant_rank = -1;
    board->halfmove_clock = 0U;
    board->fullmove_number = 1U;
}

static void dump_action(const char *name, const CfCpuAction *action, int score)
{
    printf("%s score=%d type=%d from=%d,%d to=%d,%d promo=%d dir=%d fart=%d\n",
           name, score, (int)action->type,
           action->from_file, action->from_rank,
           action->to_file, action->to_rank,
           (int)action->promotion, (int)action->direction,
           (int)action->fart_result);
}

static void dump_position(const CfBoard *board, const CfGasState *gas,
                          unsigned long seed)
{
    int file;
    int rank;
    const CfPiece *piece;
    printf("ALPHA_BETA_FIXTURE seed=%lu side=%d rights=%u\n",
           seed, (int)board->side_to_move, board->castling_rights);
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = &board->squares[rank][file];
            if (piece->type == CF_PIECE_NONE) continue;
            printf("PIECE f=%d r=%d type=%d color=%d gas=%u\n",
                   file, rank, (int)piece->type, (int)piece->color,
                   (unsigned)gas_at(gas, file, rank));
        }
    }
}

int main(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction current;
    CfCpuAction old;
    CfCpuStats stats;
    int old_score;
    unsigned long seed;
    int current_ok;
    int old_ok;

    memset(&current, 0, sizeof(current));
    memset(&old, 0, sizeof(old));
    memset(&stats, 0, sizeof(stats));
    old_score = 0;

    cpu_config_for_difficulty(&config, CF_CPU_HARD);
    config.max_depth = 3;
    config.node_budget = 0UL;
    config.time_limit_ms = 0UL;

    for (seed = 1UL; seed <= 256UL; ++seed) {
        build_position(&board, &gas, seed);
        gas_history_init(&history, &board, &gas);
        current_ok = cpu_choose_action(&board, &gas, &history,
                                       &config, &current, &stats);
        old_ok = old_choose_action(&board, &gas, &history,
                                   &config, &old, &old_score);
        if (current_ok && old_ok &&
            (!same_action(&current, &old) || stats.score != old_score)) {
            dump_position(&board, &gas, seed);
            dump_action("CORRECTED", &current, stats.score);
            dump_action("OLD_WINDOW", &old, old_score);
            printf("ACTION_DIFFERENT=%d SCORE_DIFFERENT=%d\n",
                   !same_action(&current, &old), stats.score != old_score);
            return 0;
        }
    }

    printf("No alpha-beta edge-window divergence found in scan.\n");
    return 1;
}
