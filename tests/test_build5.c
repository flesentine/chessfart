#include <stdio.h>

#include "board.h"
#include "gas.h"

static int failures;

#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static void kings_only(CfBoard *board)
{
    board_clear(board);
    board_set_piece(board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(board, 4, 7, CF_PIECE_KING, CF_COLOR_BLACK);
}

static void test_quiet_move_and_unmake(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasMove move;
    board_init_starting_position(&board);
    gas_init(&gas);
    CHECK(gas_make_move(&board, &gas, 4, 1, 4, 3, &move));
    CHECK(gas_at(&gas, 4, 1) == 0U);
    CHECK(gas_at(&gas, 4, 3) == 1U);
    gas_unmake_move(&board, &gas, &move);
    CHECK(board.squares[1][4].type == CF_PIECE_PAWN);
    CHECK(gas_at(&gas, 4, 1) == 0U);
}

static void test_capture_and_cap(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasMove move;
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 1, 0, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 2, 2, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 1, 0, 2U);
    gas_set(&gas, 2, 2, 3U);
    CHECK(gas_make_move(&board, &gas, 1, 0, 2, 2, &move));
    CHECK(board.squares[2][2].type == CF_PIECE_KNIGHT);
    CHECK(gas_at(&gas, 2, 2) == 3U);
    gas_unmake_move(&board, &gas, &move);
    CHECK(gas_at(&gas, 1, 0) == 2U);
    CHECK(gas_at(&gas, 2, 2) == 3U);
}

static void test_castling_gas(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasMove move;
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 7, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board.castling_rights = CF_CASTLE_WHITE_KING;
    gas_set(&gas, 4, 0, 2U);
    gas_set(&gas, 7, 0, 1U);
    CHECK(gas_make_move(&board, &gas, 4, 0, 6, 0, &move));
    CHECK(gas_at(&gas, 6, 0) == 3U);
    CHECK(gas_at(&gas, 5, 0) == 2U);
    CHECK(gas_at(&gas, 7, 0) == 0U);
    gas_unmake_move(&board, &gas, &move);
    CHECK(gas_at(&gas, 4, 0) == 2U);
    CHECK(gas_at(&gas, 7, 0) == 1U);
}

static void test_promotion_gas(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasMove move;
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    gas_set(&gas, 0, 6, 2U);
    CHECK(gas_make_move_ex(&board, &gas, 0, 6, 0, 7,
                           CF_PIECE_KNIGHT, &move));
    CHECK(board.squares[7][0].type == CF_PIECE_KNIGHT);
    CHECK(gas_at(&gas, 0, 7) == 3U);
    gas_unmake_move(&board, &gas, &move);
    CHECK(board.squares[6][0].type == CF_PIECE_PAWN);
    CHECK(gas_at(&gas, 0, 6) == 2U);
}

static void test_puff(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfFartAction action;
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    gas_set(&gas, 2, 2, 3U);
    board.en_passant_file = 3;
    board.en_passant_rank = 2;
    board.halfmove_clock = 7U;
    gas_history_init(&history, &board, &gas);
    CHECK(gas_piece_can_fart(&board, &gas, 2, 2));
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_N) == CF_FART_PUFF);
    CHECK(gas_make_puff(&board, &gas, 2, 2, CF_FART_N, &action));
    CHECK(gas_at(&gas, 2, 2) == 1U);
    CHECK(board.side_to_move == CF_COLOR_BLACK);
    CHECK(board.en_passant_file == -1 && board.en_passant_rank == -1);
    CHECK(board.halfmove_clock == 8U);
    CHECK(board.fullmove_number == 1U);
    gas_history_record(&history, &board, &gas);
    gas_unmake_puff(&board, &gas, &action);
    CHECK(gas_at(&gas, 2, 2) == 3U);
    CHECK(board.side_to_move == CF_COLOR_WHITE);
    CHECK(board.en_passant_file == 3 && board.en_passant_rank == 2);
    CHECK(board.halfmove_clock == 7U);
}

static void test_preview_and_check_safety(void)
{
    CfBoard board;
    CfGasState gas;
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 2, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 2, 2, 2U);
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_E) == CF_FART_PUSH_BUILD6);
    CHECK(!gas_make_puff(&board, &gas, 2, 2, CF_FART_E, 0));
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_N) == CF_FART_PUFF);
    board_set_piece(&board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    CHECK(board_is_in_check(&board, CF_COLOR_WHITE));
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_N) == CF_FART_INVALID);
}

static void record_move(CfBoard *board, CfGasState *gas, CfGasHistory *history,
                        int ff, int fr, int tf, int tr)
{
    CfGasMove move;
    CHECK(gas_make_move(board, gas, ff, fr, tf, tr, &move));
    gas_history_record(history, board, gas);
}

static void knight_cycle(CfBoard *board, CfGasState *gas, CfGasHistory *history)
{
    record_move(board, gas, history, 6, 0, 5, 2);
    record_move(board, gas, history, 6, 7, 5, 5);
    record_move(board, gas, history, 5, 2, 6, 0);
    record_move(board, gas, history, 5, 5, 6, 7);
}

static void test_gas_repetition(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    knight_cycle(&board, &gas, &history);
    CHECK(gas_history_repetition_count(&history, &board, &gas) == 1);
    knight_cycle(&board, &gas, &history);
    CHECK(gas_at(&gas, 6, 0) == 3U);
    CHECK(gas_at(&gas, 6, 7) == 3U);
    CHECK(gas_history_repetition_count(&history, &board, &gas) == 1);
    knight_cycle(&board, &gas, &history);
    CHECK(gas_history_repetition_count(&history, &board, &gas) == 2);
    knight_cycle(&board, &gas, &history);
    CHECK(gas_history_repetition_count(&history, &board, &gas) == 3);
    CHECK(gas_game_status(&board, &gas, &history) == CF_GAME_DRAW_THREEFOLD);
}

int main(void)
{
    test_quiet_move_and_unmake();
    test_capture_and_cap();
    test_castling_gas();
    test_promotion_gas();
    test_puff();
    test_preview_and_check_safety();
    test_gas_repetition();
    if (failures != 0) {
        printf("Build 5 tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 5 gas tests passed.\n");
    return 0;
}
