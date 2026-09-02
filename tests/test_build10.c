#include <stdio.h>
#include <string.h>

#include "cpu_internal.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static int same_board(const CfBoard *a, const CfBoard *b)
{
    int file;
    int rank;
    if (a->side_to_move != b->side_to_move ||
        a->castling_rights != b->castling_rights ||
        a->en_passant_file != b->en_passant_file ||
        a->en_passant_rank != b->en_passant_rank ||
        a->halfmove_clock != b->halfmove_clock ||
        a->fullmove_number != b->fullmove_number) return 0;
    for (rank = 0; rank < 8; ++rank)
        for (file = 0; file < 8; ++file)
            if (a->squares[rank][file].type != b->squares[rank][file].type ||
                a->squares[rank][file].color != b->squares[rank][file].color)
                return 0;
    return 1;
}

static int same_gas(const CfGasState *a, const CfGasState *b)
{
    return memcmp(a->squares, b->squares, sizeof(a->squares)) == 0;
}

static void tactical_hard_config(CfCpuConfig *config)
{
    cpu_config_for_difficulty(config, CF_CPU_HARD);
    config->max_depth = 2;
    config->node_budget = 0UL;
    config->time_limit_ms = 0UL;
}

static void test_difficulty_config(void)
{
    CfCpuConfig easy;
    CfCpuConfig medium;
    CfCpuConfig hard;
    cpu_config_for_difficulty(&easy, CF_CPU_EASY);
    CHECK(easy.max_depth == 1);
    CHECK(easy.node_budget > 0UL);
    cpu_config_for_difficulty(&medium, CF_CPU_MEDIUM);
    CHECK(medium.max_depth == 2);
    cpu_config_for_difficulty(&hard, CF_CPU_HARD);
    CHECK(hard.max_depth == 3);
    CHECK(easy.fart_bias < medium.fart_bias);
    CHECK(medium.fart_bias < hard.fart_bias);
    cpu_cycle_difficulty(&easy);
    CHECK(easy.difficulty == CF_CPU_MEDIUM);
    cpu_cycle_difficulty(&easy);
    CHECK(easy.difficulty == CF_CPU_HARD);
}

static void test_starting_position_deterministic(void)
{
    CfBoard board;
    CfBoard before;
    CfGasState gas;
    CfGasState gas_before;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction a;
    CfCpuAction b;
    CfCpuStats stats;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    before = board;
    gas_before = gas;
    cpu_config_for_difficulty(&config, CF_CPU_MEDIUM);
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &a, &stats));
    CHECK(a.type == CF_CPU_ACTION_MOVE);
    CHECK(stats.depth_completed >= 1);
    CHECK(same_board(&board, &before));
    CHECK(same_gas(&gas, &gas_before));
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &b, &stats));
    CHECK(a.type == b.type);
    CHECK(a.from_file == b.from_file);
    CHECK(a.from_rank == b.from_rank);
    CHECK(a.to_file == b.to_file);
    CHECK(a.to_rank == b.to_rank);
}

static void test_mate_in_one(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    CfCpuUndo undo;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 7, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 5, 2, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 6, 2, CF_PIECE_QUEEN, CF_COLOR_BLACK);
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);
    cpu_config_for_difficulty(&config, CF_CPU_EASY);
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_MOVE);
    CHECK(action.from_file == 6 && action.from_rank == 2);
    CHECK(action.to_file == 6 && action.to_rank == 1);
    CHECK(cpu_apply_action(&board, &gas, &action, &undo));
    gas_history_record(&history, &board, &gas);
    CHECK(gas_game_status(&board, &gas, &history) == CF_GAME_CHECKMATE);
    cpu_unapply_action(&board, &gas, &undo);
}

static void test_fart_action_and_unmake(void)
{
    CfBoard board;
    CfBoard before;
    CfGasState gas;
    CfGasState gas_before;
    CfCpuActionList list;
    CfCpuUndo undo;
    int i;
    int found = -1;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 2, 2, 3U);
    board.side_to_move = CF_COLOR_WHITE;
    before = board;
    gas_before = gas;

    cpu_generate_actions(&board, &gas, &list);
    for (i = 0; i < list.count; ++i) {
        if (list.actions[i].type == CF_CPU_ACTION_FART &&
            list.actions[i].from_file == 2 && list.actions[i].from_rank == 2 &&
            list.actions[i].direction == CF_FART_NE &&
            list.actions[i].fart_result == CF_FART_PUSH) {
            found = i;
            break;
        }
    }
    CHECK(found >= 0);
    if (found >= 0) {
        CHECK(cpu_apply_action(&board, &gas, &list.actions[found], &undo));
        CHECK(board.squares[4][4].type == CF_PIECE_PAWN);
        CHECK(board.squares[3][3].type == CF_PIECE_NONE);
        cpu_unapply_action(&board, &gas, &undo);
        CHECK(same_board(&board, &before));
        CHECK(same_gas(&gas, &gas_before));
    }
}

static void test_cpu_chooses_useful_enemy_push(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 3, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 3, CF_PIECE_ROOK, CF_COLOR_WHITE);
    gas_set(&gas, 2, 3, 3U);
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);

    cpu_config_for_difficulty(&config, CF_CPU_MEDIUM);
    config.max_depth = 1;
    config.time_limit_ms = 0UL;
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_FART);
    CHECK(action.from_file == 2 && action.from_rank == 3);
    CHECK(action.direction == CF_FART_E);
    CHECK(action.fart_result == CF_FART_PUSH);
}

static void test_cpu_avoids_useless_puff(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 0, 1, CF_PIECE_PAWN, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 3, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    gas_set(&gas, 2, 3, 2U);
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);

    cpu_config_for_difficulty(&config, CF_CPU_MEDIUM);
    config.max_depth = 1;
    config.time_limit_ms = 0UL;
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_MOVE);
}

static void test_enemy_fart_promotion_is_penalized(void)
{
    CfBoard board;
    CfGasState gas;
    CfCpuActionList list;
    CfCpuConfig config;
    CfCpuUndo undo;
    int i;
    int found;
    int bonus;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 2, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 5, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    gas_set(&gas, 2, 5, 3U);
    board.side_to_move = CF_COLOR_BLACK;
    cpu_config_for_difficulty(&config, CF_CPU_MEDIUM);

    cpu_generate_actions(&board, &gas, &list);
    found = 0;
    for (i = 0; i < list.count; ++i) {
        if (list.actions[i].type != CF_CPU_ACTION_FART ||
            list.actions[i].from_file != 2 || list.actions[i].from_rank != 5 ||
            list.actions[i].direction != CF_FART_NE ||
            list.actions[i].fart_result != CF_FART_PROMOTION) continue;
        ++found;
        CHECK(list.actions[i].order_score < 0);
        CHECK(cpu_apply_action(&board, &gas, &list.actions[i], &undo));
        bonus = cpu_internal_action_bonus(&board, &gas, &list.actions[i],
                                          &undo, &config, 0);
        CHECK(bonus < 0);
        cpu_unapply_action(&board, &gas, &undo);
    }
    CHECK(found == 4);
}

static void test_cpu_fart_escapes_check(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    CfCpuUndo undo;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 1, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 0, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 1, 2, CF_PIECE_ROOK, CF_COLOR_BLACK);
    gas_set(&gas, 0, 2, 3U);
    board.side_to_move = CF_COLOR_WHITE;
    gas_history_init(&history, &board, &gas);
    CHECK(board_is_in_check(&board, CF_COLOR_WHITE));

    tactical_hard_config(&config);
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_FART);
    CHECK(action.from_file == 0 && action.from_rank == 2);
    CHECK(action.direction == CF_FART_E);
    CHECK(action.fart_result == CF_FART_PUSH);
    CHECK(cpu_apply_action(&board, &gas, &action, &undo));
    CHECK(!board_is_in_check(&board, CF_COLOR_WHITE));
    cpu_unapply_action(&board, &gas, &undo);
}

static void test_cpu_fart_gives_forcing_check(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    CfCpuUndo undo;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 1, CF_PIECE_PAWN, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 1, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 0, CF_PIECE_BISHOP, CF_COLOR_BLACK);
    board_set_piece(&board, 1, 2, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    gas_set(&gas, 3, 1, 3U);
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);
    CHECK(!board_is_in_check(&board, CF_COLOR_WHITE));

    tactical_hard_config(&config);
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_FART);
    CHECK(action.from_file == 3 && action.from_rank == 1);
    CHECK(action.direction == CF_FART_E);
    CHECK(action.fart_result == CF_FART_PUSH);
    CHECK(cpu_apply_action(&board, &gas, &action, &undo));
    CHECK(board.squares[1][5].type == CF_PIECE_PAWN);
    CHECK(board.squares[1][5].color == CF_COLOR_BLACK);
    CHECK(board_is_in_check(&board, CF_COLOR_WHITE));
    cpu_unapply_action(&board, &gas, &undo);
}

static void test_cpu_fart_wrecks_castling_with_check(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    CfCpuUndo undo;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 0, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 1, 0, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 2, 0, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 6, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 5, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 0, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    gas_set(&gas, 3, 0, 3U);
    board.castling_rights = CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN;
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);

    tactical_hard_config(&config);
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_FART);
    CHECK(action.from_file == 3 && action.from_rank == 0);
    CHECK(action.direction == CF_FART_E);
    CHECK(action.fart_result == CF_FART_PUSH);
    CHECK(cpu_apply_action(&board, &gas, &action, &undo));
    CHECK(board.squares[0][5].type == CF_PIECE_KING);
    CHECK(board.squares[0][5].color == CF_COLOR_WHITE);
    CHECK((board.castling_rights &
           (CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN)) == 0U);
    CHECK(board_is_in_check(&board, CF_COLOR_WHITE));
    cpu_unapply_action(&board, &gas, &undo);
}

static void test_cpu_fart_promotes_own_pawn(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    CfCpuUndo undo;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 2, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 1, CF_PIECE_PAWN, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 0, CF_PIECE_BISHOP, CF_COLOR_BLACK);
    gas_set(&gas, 3, 2, 3U);
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);

    tactical_hard_config(&config);
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(action.type == CF_CPU_ACTION_FART);
    CHECK(action.from_file == 3 && action.from_rank == 2);
    CHECK(action.direction == CF_FART_SE);
    CHECK(action.fart_result == CF_FART_PROMOTION);
    CHECK(action.promotion == CF_PIECE_QUEEN);
    CHECK(cpu_apply_action(&board, &gas, &action, &undo));
    CHECK(board.squares[0][5].type == CF_PIECE_QUEEN);
    CHECK(board.squares[0][5].color == CF_COLOR_BLACK);
    cpu_unapply_action(&board, &gas, &undo);
}

static void test_budget_cap(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    cpu_config_for_difficulty(&config, CF_CPU_HARD);
    config.max_depth = 4;
    config.node_budget = 8UL;
    config.time_limit_ms = 0UL;
    CHECK(cpu_choose_action(&board, &gas, &history, &config, &action, &stats));
    CHECK(stats.nodes <= config.node_budget);
    CHECK(stats.budget_hit);
    CHECK(action.type != CF_CPU_ACTION_NONE);
}

int main(void)
{
    test_difficulty_config();
    test_starting_position_deterministic();
    test_mate_in_one();
    test_fart_action_and_unmake();
    test_cpu_chooses_useful_enemy_push();
    test_cpu_avoids_useless_puff();
    test_enemy_fart_promotion_is_penalized();
    test_cpu_fart_escapes_check();
    test_cpu_fart_gives_forcing_check();
    test_cpu_fart_wrecks_castling_with_check();
    test_cpu_fart_promotes_own_pawn();
    test_budget_cap();
    if (failures != 0) {
        printf("Build 10 CPU tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 10 CPU tests passed.\n");
    return 0;
}
