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
    CfFartScan prechecked_scan;
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
    int actor_in_check;
    int d;
    int p;
    int placed;
    int square;

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
            square = (int)((fart_scan_rng_next(&rng) >> 16) % 64UL);
            file = square & 7;
            rank = square >> 3;
            if (board.squares[rank][file].type != CF_PIECE_NONE) continue;
            type = (CfPieceType)(1 +
                   ((fart_scan_rng_next(&rng) >> 16) % 5UL));
            color = ((fart_scan_rng_next(&rng) >> 16) & 1UL) != 0UL ?
                    CF_COLOR_WHITE : CF_COLOR_BLACK;
            board_set_piece(&board, file, rank, type, color);
            ++placed;
        }

        gas_scan_farts(&board, &gas, actor_file, actor_rank, &scan);
        actor_in_check = board_is_in_check(&board, board.side_to_move);
        gas_scan_farts_prechecked(&board, &gas, actor_file, actor_rank,
                                  actor_in_check, &prechecked_scan);
        for (d = 0; d < 8; ++d) {
            CHECK(prechecked_scan.preview[d] == scan.preview[d]);
            CHECK(prechecked_scan.promotion_mask[d] ==
                  scan.promotion_mask[d]);
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

static int same_board_state(const CfBoard *a, const CfBoard *b)
{
    int file;
    int rank;
    for (rank = 0; rank < 8; ++rank)
        for (file = 0; file < 8; ++file)
            if (a->squares[rank][file].type != b->squares[rank][file].type ||
                a->squares[rank][file].color != b->squares[rank][file].color)
                return 0;
    return a->side_to_move == b->side_to_move &&
           a->castling_rights == b->castling_rights &&
           a->en_passant_file == b->en_passant_file &&
           a->en_passant_rank == b->en_passant_rank &&
           a->halfmove_clock == b->halfmove_clock &&
           a->fullmove_number == b->fullmove_number;
}

static int same_gas_state(const CfGasState *a, const CfGasState *b)
{
    int file;
    int rank;
    for (rank = 0; rank < 8; ++rank)
        for (file = 0; file < 8; ++file)
            if (a->squares[rank][file] != b->squares[rank][file])
                return 0;
    return 1;
}

static int same_move(const CfMove *a, const CfMove *b)
{
    return a->from_file == b->from_file &&
           a->from_rank == b->from_rank &&
           a->to_file == b->to_file &&
           a->to_rank == b->to_rank &&
           a->captured_file == b->captured_file &&
           a->captured_rank == b->captured_rank &&
           a->moved.type == b->moved.type &&
           a->moved.color == b->moved.color &&
           a->captured.type == b->captured.type &&
           a->captured.color == b->captured.color &&
           a->promotion == b->promotion &&
           a->flags == b->flags &&
           a->prev_side_to_move == b->prev_side_to_move &&
           a->prev_castling_rights == b->prev_castling_rights &&
           a->prev_en_passant_file == b->prev_en_passant_file &&
           a->prev_en_passant_rank == b->prev_en_passant_rank &&
           a->prev_halfmove_clock == b->prev_halfmove_clock &&
           a->prev_fullmove_number == b->prev_fullmove_number;
}

static int same_gas_move(const CfGasMove *a, const CfGasMove *b)
{
    return same_move(&a->chess_move, &b->chess_move) &&
           a->previous_from_gas == b->previous_from_gas &&
           a->previous_to_gas == b->previous_to_gas &&
           a->previous_captured_gas == b->previous_captured_gas &&
           a->previous_rook_from_gas == b->previous_rook_from_gas &&
           a->previous_rook_to_gas == b->previous_rook_to_gas;
}

static void check_prevalidated_move_matches_public(
    CfBoard *board, CfGasState *gas,
    int from_file, int from_rank,
    int to_file, int to_rank,
    CfPieceType promotion)
{
    CfBoard public_board;
    CfBoard fast_board;
    CfGasState public_gas;
    CfGasState fast_gas;
    CfGasMove public_move;
    CfGasMove fast_move;

    public_board = *board;
    fast_board = *board;
    public_gas = *gas;
    fast_gas = *gas;

    CHECK(gas_make_move_ex(&public_board, &public_gas,
                           from_file, from_rank, to_file, to_rank,
                           promotion, &public_move));
    CHECK(gas_make_move_prevalidated(&fast_board, &fast_gas,
                                     from_file, from_rank, to_file, to_rank,
                                     promotion, &fast_move));
    CHECK(same_board_state(&public_board, &fast_board));
    CHECK(same_gas_state(&public_gas, &fast_gas));
    CHECK(same_gas_move(&public_move, &fast_move));

    gas_unmake_move(&public_board, &public_gas, &public_move);
    gas_unmake_move(&fast_board, &fast_gas, &fast_move);
    CHECK(same_board_state(&public_board, board));
    CHECK(same_board_state(&fast_board, board));
    CHECK(same_gas_state(&public_gas, gas));
    CHECK(same_gas_state(&fast_gas, gas));
}

static void test_prevalidated_move_matches_public_apply(void)
{
    CfBoard board;
    CfGasState gas;

    /* Ordinary move. */
    board_init_starting_position(&board);
    gas_init(&gas);
    gas_set(&gas, 1, 0, 2U);
    check_prevalidated_move_matches_public(&board, &gas,
                                           1, 0, 2, 2,
                                           CF_PIECE_NONE);

    /* Capture. */
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 1, 1, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 1, 4, CF_PIECE_BISHOP, CF_COLOR_BLACK);
    gas_set(&gas, 1, 1, 2U);
    gas_set(&gas, 1, 4, 1U);
    check_prevalidated_move_matches_public(&board, &gas,
                                           1, 1, 1, 4,
                                           CF_PIECE_NONE);

    /* Pawn double. */
    board_init_starting_position(&board);
    gas_init(&gas);
    gas_set(&gas, 4, 1, 1U);
    check_prevalidated_move_matches_public(&board, &gas,
                                           4, 1, 4, 3,
                                           CF_PIECE_NONE);

    /* En passant. */
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 4, 4, CF_PIECE_PAWN, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 4, CF_PIECE_PAWN, CF_COLOR_BLACK);
    board.en_passant_file = 3;
    board.en_passant_rank = 5;
    gas_set(&gas, 4, 4, 2U);
    gas_set(&gas, 3, 4, 1U);
    check_prevalidated_move_matches_public(&board, &gas,
                                           4, 4, 3, 5,
                                           CF_PIECE_NONE);

    /* King-side castle. */
    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board.castling_rights = CF_CASTLE_WHITE_KING;
    gas_set(&gas, 4, 0, 2U);
    gas_set(&gas, 7, 0, 1U);
    check_prevalidated_move_matches_public(&board, &gas,
                                           4, 0, 6, 0,
                                           CF_PIECE_NONE);

    /* Promotion. */
    kings_only(&board);
    gas_init(&gas);
    board_set_piece(&board, 1, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    gas_set(&gas, 1, 6, 2U);
    check_prevalidated_move_matches_public(&board, &gas,
                                           1, 6, 1, 7,
                                           CF_PIECE_KNIGHT);
}

static int same_fart_action(const CfFartAction *a, const CfFartAction *b)
{
    return a->actor_file == b->actor_file &&
           a->actor_rank == b->actor_rank &&
           a->direction == b->direction &&
           a->result == b->result &&
           a->target_file == b->target_file &&
           a->target_rank == b->target_rank &&
           a->destination_file == b->destination_file &&
           a->destination_rank == b->destination_rank &&
           a->previous_target_piece.type == b->previous_target_piece.type &&
           a->previous_target_piece.color == b->previous_target_piece.color &&
           a->previous_destination_piece.type ==
               b->previous_destination_piece.type &&
           a->previous_destination_piece.color ==
               b->previous_destination_piece.color &&
           a->previous_actor_gas == b->previous_actor_gas &&
           a->previous_target_gas == b->previous_target_gas &&
           a->previous_destination_gas == b->previous_destination_gas &&
           a->promotion == b->promotion &&
           a->previous_side == b->previous_side &&
           a->previous_castling_rights == b->previous_castling_rights &&
           a->previous_ep_file == b->previous_ep_file &&
           a->previous_ep_rank == b->previous_ep_rank &&
           a->previous_halfmove == b->previous_halfmove &&
           a->previous_fullmove == b->previous_fullmove;
}

static void check_prevalidated_matches_public(CfBoard *board, CfGasState *gas,
                                              int file, int rank,
                                              CfFartDirection direction,
                                              CfPieceType promotion)
{
    CfBoard public_board;
    CfBoard fast_board;
    CfGasState public_gas;
    CfGasState fast_gas;
    CfFartAction public_action;
    CfFartAction fast_action;
    CfFartPreview preview;

    public_board = *board;
    fast_board = *board;
    public_gas = *gas;
    fast_gas = *gas;
    preview = gas_preview_fart(board, gas, file, rank, direction);
    CHECK(preview != CF_FART_INVALID);
    if (preview == CF_FART_PROMOTION)
        CHECK(gas_fart_promotion_choice_legal(board, gas, file, rank,
                                              direction, promotion));

    CHECK(gas_make_fart(&public_board, &public_gas, file, rank,
                        direction, promotion, &public_action));
    CHECK(gas_make_fart_prevalidated(&fast_board, &fast_gas, file, rank,
                                     direction, preview, promotion,
                                     &fast_action));
    CHECK(same_board_state(&public_board, &fast_board));
    CHECK(same_gas_state(&public_gas, &fast_gas));
    CHECK(same_fart_action(&public_action, &fast_action));

    gas_unmake_fart(&public_board, &public_gas, &public_action);
    gas_unmake_fart(&fast_board, &fast_gas, &fast_action);
    CHECK(same_board_state(&public_board, board));
    CHECK(same_board_state(&fast_board, board));
    CHECK(same_gas_state(&public_gas, gas));
    CHECK(same_gas_state(&fast_gas, gas));
}

static void test_prevalidated_fart_matches_public_apply(void)
{
    CfBoard board;
    CfGasState gas;

    /* PUFF */
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    check_prevalidated_matches_public(&board, &gas, 2, 2,
                                      CF_FART_N, CF_PIECE_NONE);

    /* BLOCKED */
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    board_set_piece(&board, 3, 3, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    board_set_piece(&board, 4, 4, CF_PIECE_ROOK, CF_COLOR_BLACK);
    check_prevalidated_matches_public(&board, &gas, 2, 2,
                                      CF_FART_NE, CF_PIECE_NONE);

    /* PUSH */
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 2, 2);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 3, 3, 1U);
    check_prevalidated_matches_public(&board, &gas, 2, 2,
                                      CF_FART_NE, CF_PIECE_NONE);

    /* PROMOTION */
    kings_only(&board);
    gas_init(&gas);
    add_actor(&board, &gas, 4, 5);
    board_set_piece(&board, 5, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    gas_set(&gas, 5, 6, 2U);
    check_prevalidated_matches_public(&board, &gas, 4, 5,
                                      CF_FART_NE, CF_PIECE_KNIGHT);
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
    test_prevalidated_move_matches_public_apply();
    test_prevalidated_fart_matches_public_apply();
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
