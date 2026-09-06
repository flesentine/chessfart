#include <stdio.h>
#include <string.h>

#include "practice_undo.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static void test_memory_budget(void)
{
    CHECK(sizeof(CfPracticeUndoJournal) < 8192U);
    CHECK(sizeof(CfPracticeUndoEntry) < 256U);
}

static void test_move_undo_exact(void)
{
    CfBoard board;
    CfBoard before_board;
    CfGasState gas;
    CfGasState before_gas;
    CfGasHistory history;
    CfGasHistory before_history;
    CfGasMove move;
    CfPracticeUndoJournal journal;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    before_board = board;
    before_gas = gas;
    before_history = history;
    practice_undo_init(&journal);

    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    practice_undo_record_move(&journal, &move, &history);
    gas_history_record(&history, &board, &gas);
    CHECK(practice_undo_can_undo(&journal));
    CHECK(practice_undo_apply_last(&journal, &board, &gas, &history));
    CHECK(memcmp(&board, &before_board, sizeof(board)) == 0);
    CHECK(memcmp(&gas, &before_gas, sizeof(gas)) == 0);
    CHECK(memcmp(&history, &before_history, sizeof(history)) == 0);
    CHECK(!practice_undo_can_undo(&journal));
}

static void test_fart_undo_exact(void)
{
    CfBoard board;
    CfBoard before_board;
    CfGasState gas;
    CfGasState before_gas;
    CfGasHistory history;
    CfGasHistory before_history;
    CfFartAction fart;
    CfPracticeUndoJournal journal;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 2, 2, 3U);
    gas_set(&gas, 3, 3, 2U);
    gas_history_init(&history, &board, &gas);
    before_board = board;
    before_gas = gas;
    before_history = history;
    practice_undo_init(&journal);

    CHECK(gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                        CF_PIECE_QUEEN, &fart));
    practice_undo_record_fart(&journal, &fart, &history);
    gas_history_record(&history, &board, &gas);
    CHECK(practice_undo_apply_last(&journal, &board, &gas, &history));
    CHECK(memcmp(&board, &before_board, sizeof(board)) == 0);
    CHECK(memcmp(&gas, &before_gas, sizeof(gas)) == 0);
    CHECK(memcmp(&history, &before_history, sizeof(history)) == 0);
}

static void test_full_history_undo_restores_dropped_key(void)
{
    CfBoard board;
    CfBoard before_board;
    CfGasState gas;
    CfGasState before_gas;
    CfGasHistory history;
    CfGasHistory before_history;
    CfGasMove move;
    CfPracticeUndoJournal journal;
    int i;

    board_init_starting_position(&board);
    gas_init(&gas);
    memset(&history, 0, sizeof(history));
    history.count = CF_GAS_HISTORY;
    for (i = 0; i < CF_GAS_HISTORY; ++i) {
        history.keys[i].state = (cf_u8)i;
        history.keys[i].en_passant_file = (cf_u8)(i % 9);
        history.keys[i].squares[0] = (cf_u8)(i & 0x3FU);
        history.keys[i].squares[63] = (cf_u8)((i * 3) & 0x3FU);
    }
    before_board = board;
    before_gas = gas;
    before_history = history;
    practice_undo_init(&journal);

    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    practice_undo_record_move(&journal, &move, &history);
    gas_history_record(&history, &board, &gas);
    CHECK(history.count == CF_GAS_HISTORY);
    CHECK(practice_undo_apply_last(&journal, &board, &gas, &history));
    CHECK(memcmp(&board, &before_board, sizeof(board)) == 0);
    CHECK(memcmp(&gas, &before_gas, sizeof(gas)) == 0);
    CHECK(memcmp(&history, &before_history, sizeof(history)) == 0);
}

static void test_history_mismatch_is_transactional(void)
{
    CfBoard board;
    CfBoard after_move;
    CfGasState gas;
    CfGasState after_gas;
    CfGasHistory history;
    CfGasMove move;
    CfPracticeUndoJournal journal;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    practice_undo_init(&journal);

    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    practice_undo_record_move(&journal, &move, &history);
    after_move = board;
    after_gas = gas;

    CHECK(!practice_undo_apply_last(&journal, &board, &gas, &history));
    CHECK(memcmp(&board, &after_move, sizeof(board)) == 0);
    CHECK(memcmp(&gas, &after_gas, sizeof(gas)) == 0);
    CHECK(journal.count == 1);
}

static void test_journal_is_bounded_ring(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfGasMove move;
    CfPracticeUndoJournal journal;
    int i;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    practice_undo_init(&journal);

    for (i = 0; i < CF_PRACTICE_UNDO_CAPACITY + 9; ++i)
        practice_undo_record_move(&journal, &move, &history);

    CHECK(journal.count == CF_PRACTICE_UNDO_CAPACITY);
    CHECK(journal.start == 9);
}

int main(void)
{
    test_memory_budget();
    test_move_undo_exact();
    test_fart_undo_exact();
    test_full_history_undo_restores_dropped_key();
    test_history_mismatch_is_transactional();
    test_journal_is_bounded_ring();

    if (failures != 0) {
        printf("Build 16 practice-undo tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 16 practice-undo tests passed.\n");
    return 0;
}
