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
    board_set_piece(board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
}

static void add_actor(CfBoard *board, CfGasState *gas, int file, int rank)
{
    board_set_piece(board, file, rank, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    gas_set(gas, file, rank, 3U);
}

static unsigned long fart_scan_rng_next(unsigned long *state)
{
    *state = *state * 1664525UL + 1013904223UL;
    return *state;
}

static void test_batch_fart_scan_matches_public_api(void)
{
    static const CfPieceType promotions[4] = {
        CF_PIECE_QUEEN, CF_PIECE_ROOK, CF_PIECE_BISHOP, CF_PIECE_KNIGHT
    };
    CfBoard board;
    CfGasState gas;
    CfFartScan scan;
    CfFartPreview expected;
    unsigned long rng = 0xF47CA11UL;
    cf_u8 expected_mask;
    CfPieceColor actor_color;
    CfPieceColor color;
    CfPieceType type;
    int sample;
    int file;
    int rank;
    int actor_file;
    int actor_rank;
    int d;
    int p;
    int placed;

    for (sample = 0; sample < 128; ++sample) {
        board_clear(&board);
        gas_init(&gas);
        board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);

        actor_file = 1 + (int)(fart_scan_rng_next(&rng) % 6UL);
        actor_rank = 1 + (int)(fart_scan_rng_next(&rng) % 6UL);
        actor_color = (sample & 1) != 0 ? CF_COLOR_BLACK : CF_COLOR_WHITE;
        board_set_piece(&board, actor_file, actor_rank,
                        CF_PIECE_KNIGHT, actor_color);
        gas_set(&gas, actor_file, actor_rank, 3U);
        board.side_to_move = actor_color;

        placed = 0;
        while (placed < 12) {
            file = (int)(fart_scan_rng_next(&rng) % 8UL);
            rank = (int)(fart_scan_rng_next(&rng) % 8UL);
            if (board.squares[rank][file].type != CF_PIECE_NONE) continue;
            type = (CfPieceType)(1 + fart_scan_rng_next(&rng) % 5UL);
            color = (fart_scan_rng_next(&rng) & 1UL) != 0UL ?
                    CF_COLOR_WHITE : CF_COLOR_BLACK;
            board_set_piece(&board, file, rank, type, color);
            ++placed;
        }

        gas_scan_farts(&board, &gas, actor_file, actor_rank, &scan);
        for (d = 0; d < 8; ++d) {
            expected = gas_preview_fart(&board, &gas,
                                        actor_file, actor_rank,
                                        (CfFartDirection)d);
            expected_mask = 0U;
            if (expected == CF_FART_PROMOTION) {
                for (p = 0; p < 4; ++p)
                    if (gas_fart_promotion_choice_legal(
                            &board, &gas, actor_file, actor_rank,
                            (CfFartDirection)d, promotions[p]))
                        expected_mask = (cf_u8)(expected_mask |
                                                (cf_u8)(1U << p));
            }
            CHECK((CfFartPreview)scan.preview[d] == expected);
            CHECK(scan.promotion_mask[d] == expected_mask);
        }
    }
}

static void test_push_and_unmake(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 3, 3, 1U);
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_NE) == CF_FART_PUSH);
    CHECK(gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                        CF_PIECE_QUEEN, &action));
    CHECK(board.squares[3][3].type == CF_PIECE_NONE);
    CHECK(board.squares[4][4].type == CF_PIECE_PAWN);
    CHECK(board.squares[4][4].color == CF_COLOR_BLACK);
    CHECK(gas_at(&gas, 4, 4) == 1U);
    CHECK(gas_at(&gas, 2, 2) == 1U);
    CHECK(board.side_to_move == CF_COLOR_BLACK);
    CHECK(board.halfmove_clock == 1U);
    gas_unmake_fart(&board, &gas, &action);
    CHECK(board.squares[3][3].type == CF_PIECE_PAWN);
    CHECK(board.squares[4][4].type == CF_PIECE_NONE);
    CHECK(gas_at(&gas, 3, 3) == 1U);
    CHECK(gas_at(&gas, 2, 2) == 3U);
    CHECK(board.side_to_move == CF_COLOR_WHITE);
}

static void test_friendly_push(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    board_set_piece(&board, 3, 3, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    gas_set(&gas, 3, 3, 2U);
    CHECK(gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                        CF_PIECE_QUEEN, &action));
    CHECK(board.squares[4][4].type == CF_PIECE_BISHOP);
    CHECK(board.squares[4][4].color == CF_COLOR_WHITE);
    CHECK(gas_at(&gas, 4, 4) == 2U);
}

static void test_blocked_push_spends_turn(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    board_set_piece(&board, 3, 3, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    board_set_piece(&board, 4, 4, CF_PIECE_ROOK, CF_COLOR_BLACK);
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_NE) == CF_FART_BLOCKED);
    CHECK(gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                        CF_PIECE_QUEEN, &action));
    CHECK(board.squares[3][3].type == CF_PIECE_BISHOP);
    CHECK(board.squares[4][4].type == CF_PIECE_ROOK);
    CHECK(gas_at(&gas, 2, 2) == 1U);
    CHECK(board.side_to_move == CF_COLOR_BLACK);
    gas_unmake_fart(&board, &gas, &action);
    CHECK(gas_at(&gas, 2, 2) == 3U);
}

static void test_edge_block_and_offboard_aim(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    add_actor(&board, &gas, 6, 6);
    board_set_piece(&board, 7, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    CHECK(gas_preview_fart(&board, &gas, 6, 6, CF_FART_NE) == CF_FART_BLOCKED);
    CHECK(gas_make_fart(&board, &gas, 6, 6, CF_FART_NE,
                        CF_PIECE_QUEEN, &action));
    gas_unmake_fart(&board, &gas, &action);
    board_set_piece(&board, 7, 7, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    gas_set(&gas, 7, 7, 3U);
    CHECK(gas_preview_fart(&board, &gas, 7, 7, CF_FART_NE) == CF_FART_INVALID);
}

static void test_self_check_and_check_resolution(void)
{
    CfBoard board;
    CfGasState gas;
    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 1, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    add_actor(&board, &gas, 3, 1);
    CHECK(!board_is_in_check(&board, CF_COLOR_WHITE));
    CHECK(gas_preview_fart(&board, &gas, 3, 1, CF_FART_E) == CF_FART_INVALID);

    board_set_piece(&board, 4, 1, CF_PIECE_NONE, CF_COLOR_NONE);
    board_set_piece(&board, 4, 7, CF_PIECE_NONE, CF_COLOR_NONE);
    board_set_piece(&board, 4, 2, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 3, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    gas_set(&gas, 3, 2, 3U);
    CHECK(board_is_in_check(&board, CF_COLOR_WHITE));
    CHECK(gas_preview_fart(&board, &gas, 3, 2, CF_FART_E) == CF_FART_PUSH);
}

static void test_enemy_king_push_can_give_check(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    add_actor(&board, &gas, 2, 2);
    CHECK(gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                        CF_PIECE_QUEEN, &action));
    CHECK(board.squares[4][4].type == CF_PIECE_KING);
    CHECK(board.side_to_move == CF_COLOR_BLACK);
    CHECK(board_is_in_check(&board, CF_COLOR_BLACK));
    CHECK(gas_game_status(&board, &gas, 0) == CF_GAME_CHECK);
}

static void test_king_adjacency_rejected(void)
{
    CfBoard board;
    CfGasState gas;
    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 5, 3, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_KING, CF_COLOR_BLACK);
    add_actor(&board, &gas, 2, 2);
    CHECK(gas_preview_fart(&board, &gas, 2, 2, CF_FART_NE) == CF_FART_INVALID);
}

static void test_displaced_king_loses_castling_and_ep_expires(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    add_actor(&board, &gas, 3, 0);
    board.castling_rights = CF_CASTLE_ALL;
    board.en_passant_file = 2;
    board.en_passant_rank = 5;
    CHECK(gas_make_fart(&board, &gas, 3, 0, CF_FART_E,
                        CF_PIECE_QUEEN, &action));
    CHECK(board.squares[0][5].type == CF_PIECE_KING);
    CHECK((board.castling_rights &
          (CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN)) == 0U);
    CHECK(board.en_passant_file == -1 && board.en_passant_rank == -1);
    gas_unmake_fart(&board, &gas, &action);
    CHECK(board.castling_rights == CF_CASTLE_ALL);
    CHECK(board.en_passant_file == 2 && board.en_passant_rank == 5);
}

static void test_pushed_pawn_promotion(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 4, 5);
    board_set_piece(&board, 5, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    gas_set(&gas, 5, 6, 2U);
    board.halfmove_clock = 9U;
    CHECK(gas_preview_fart(&board, &gas, 4, 5, CF_FART_NE) == CF_FART_PROMOTION);
    CHECK(gas_fart_promotion_choice_legal(&board, &gas, 4, 5,
                                          CF_FART_NE, CF_PIECE_KNIGHT));
    CHECK(gas_make_fart(&board, &gas, 4, 5, CF_FART_NE,
                        CF_PIECE_KNIGHT, &action));
    CHECK(board.squares[7][6].type == CF_PIECE_KNIGHT);
    CHECK(board.squares[7][6].color == CF_COLOR_WHITE);
    CHECK(gas_at(&gas, 6, 7) == 2U);
    CHECK(board.halfmove_clock == 10U);
    gas_unmake_fart(&board, &gas, &action);
    CHECK(board.squares[6][5].type == CF_PIECE_PAWN);
    CHECK(gas_at(&gas, 5, 6) == 2U);
}

static void test_push_history_uses_gas_and_position(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfFartAction action;
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_history_init(&history, &board, &gas);
    CHECK(gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                        CF_PIECE_QUEEN, &action));
    gas_history_record(&history, &board, &gas);
    CHECK(gas_history_repetition_count(&history, &board, &gas) == 1);
}

int main(void)
{
    test_batch_fart_scan_matches_public_api();
    test_push_and_unmake();
    test_friendly_push();
    test_blocked_push_spends_turn();
    test_edge_block_and_offboard_aim();
    test_self_check_and_check_resolution();
    test_enemy_king_push_can_give_check();
    test_king_adjacency_rejected();
    test_displaced_king_loses_castling_and_ep_expires();
    test_pushed_pawn_promotion();
    test_push_history_uses_gas_and_position();

    if (failures != 0) {
        printf("Build 6 tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 6 fart displacement tests passed.\n");
    return 0;
}
