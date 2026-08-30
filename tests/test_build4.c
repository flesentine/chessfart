#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"

static int failures;

#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static int has_move(const CfMoveList *list, int file, int rank, unsigned flag)
{
    int i;
    for (i = 0; i < list->count; ++i) {
        if (list->moves[i].to_file == file && list->moves[i].to_rank == rank &&
            (flag == 0U || (list->moves[i].flags & flag) != 0U)) return 1;
    }
    return 0;
}

static unsigned long perft(CfBoard *board, int depth)
{
    CfMoveList list;
    CfMove made;
    unsigned long nodes = 0UL;
    int file;
    int rank;
    int i;
    if (depth == 0) return 1UL;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].color != board->side_to_move) continue;
            board_generate_legal_moves(board, file, rank, &list);
            for (i = 0; i < list.count; ++i) {
                CHECK(board_make_move_ex(board, file, rank,
                                         list.moves[i].to_file, list.moves[i].to_rank,
                                         list.moves[i].promotion, &made));
                nodes += perft(board, depth - 1);
                board_unmake_move(board, &made);
            }
        }
    }
    return nodes;
}

static void kings_only(CfBoard *board)
{
    board_clear(board);
    board_set_piece(board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(board, 4, 7, CF_PIECE_KING, CF_COLOR_BLACK);
}

static void test_start_and_perft(void)
{
    CfBoard board;
    board_init_starting_position(&board);
    CHECK(board.castling_rights == CF_CASTLE_ALL);
    CHECK(board.en_passant_file == -1);
    CHECK(board.halfmove_clock == 0U);
    CHECK(board.fullmove_number == 1U);
    CHECK(perft(&board, 1) == 20UL);
    CHECK(perft(&board, 2) == 400UL);
    CHECK(perft(&board, 3) == 8902UL);
    CHECK(perft(&board, 4) == 197281UL);
}

static CfPieceType fen_piece_type(char ch)
{
    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    switch (ch) {
    case 'P': return CF_PIECE_PAWN;
    case 'N': return CF_PIECE_KNIGHT;
    case 'B': return CF_PIECE_BISHOP;
    case 'R': return CF_PIECE_ROOK;
    case 'Q': return CF_PIECE_QUEEN;
    case 'K': return CF_PIECE_KING;
    default: return CF_PIECE_NONE;
    }
}

static void set_fen_rank(CfBoard *board, int rank, const char *text)
{
    int file = 0;
    int n;
    CfPieceColor color;
    CfPieceType type;
    while (*text != '\0' && file < 8) {
        if (*text >= '1' && *text <= '8') {
            n = *text - '0';
            file += n;
        } else {
            color = (*text >= 'A' && *text <= 'Z') ? CF_COLOR_WHITE : CF_COLOR_BLACK;
            type = fen_piece_type(*text);
            board_set_piece(board, file, rank, type, color);
            ++file;
        }
        ++text;
    }
}

static void test_kiwipete_perft(void)
{
    CfBoard board;
    board_clear(&board);
    set_fen_rank(&board, 7, "r3k2r");
    set_fen_rank(&board, 6, "p1ppqpb1");
    set_fen_rank(&board, 5, "bn2pnp1");
    set_fen_rank(&board, 4, "3PN3");
    set_fen_rank(&board, 3, "1p2P3");
    set_fen_rank(&board, 2, "2N2Q1p");
    set_fen_rank(&board, 1, "PPPBBPPP");
    set_fen_rank(&board, 0, "R3K2R");
    board.side_to_move = CF_COLOR_WHITE;
    board.castling_rights = CF_CASTLE_ALL;
    CHECK(perft(&board, 1) == 48UL);
    CHECK(perft(&board, 2) == 2039UL);
    CHECK(perft(&board, 3) == 97862UL);
}

static void test_en_passant_perft_position(void)
{
    CfBoard board;
    board_clear(&board);
    set_fen_rank(&board, 7, "8");
    set_fen_rank(&board, 6, "2p5");
    set_fen_rank(&board, 5, "3p4");
    set_fen_rank(&board, 4, "KP5r");
    set_fen_rank(&board, 3, "1R3p1k");
    set_fen_rank(&board, 2, "8");
    set_fen_rank(&board, 1, "4P1P1");
    set_fen_rank(&board, 0, "8");
    board.side_to_move = CF_COLOR_WHITE;
    CHECK(perft(&board, 1) == 14UL);
    CHECK(perft(&board, 2) == 191UL);
    CHECK(perft(&board, 3) == 2812UL);
    CHECK(perft(&board, 4) == 43238UL);
    CHECK(perft(&board, 5) == 674624UL);
}

static void test_promotion_perft_position(void)
{
    CfBoard board;
    board_clear(&board);
    set_fen_rank(&board, 7, "r3k2r");
    set_fen_rank(&board, 6, "Pppp1ppp");
    set_fen_rank(&board, 5, "1b3nbN");
    set_fen_rank(&board, 4, "nP6");
    set_fen_rank(&board, 3, "BBP1P3");
    set_fen_rank(&board, 2, "q4N2");
    set_fen_rank(&board, 1, "Pp1P2PP");
    set_fen_rank(&board, 0, "R2Q1RK1");
    board.side_to_move = CF_COLOR_WHITE;
    board.castling_rights = CF_CASTLE_BLACK_KING | CF_CASTLE_BLACK_QUEEN;
    CHECK(perft(&board, 1) == 6UL);
    CHECK(perft(&board, 2) == 264UL);
    CHECK(perft(&board, 3) == 9467UL);
    CHECK(perft(&board, 4) == 422333UL);
}

static void test_castling(void)
{
    CfBoard board;
    CfMoveList list;
    CfMove made;

    kings_only(&board);
    board_set_piece(&board, 0, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board.castling_rights = CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN;
    board_generate_legal_moves(&board, 4, 0, &list);
    CHECK(has_move(&list, 6, 0, CF_MOVE_CASTLE_KING));
    CHECK(has_move(&list, 2, 0, CF_MOVE_CASTLE_QUEEN));
    CHECK(board_make_move(&board, 4, 0, 6, 0, &made));
    CHECK(board.squares[0][6].type == CF_PIECE_KING);
    CHECK(board.squares[0][5].type == CF_PIECE_ROOK);
    CHECK((board.castling_rights & (CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN)) == 0U);
    board_unmake_move(&board, &made);
    CHECK(board.squares[0][4].type == CF_PIECE_KING);
    CHECK(board.squares[0][7].type == CF_PIECE_ROOK);
    CHECK(board.castling_rights == (CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN));

    board_set_piece(&board, 5, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_generate_legal_moves(&board, 4, 0, &list);
    CHECK(!has_move(&list, 6, 0, CF_MOVE_CASTLE_KING));
    CHECK(has_move(&list, 2, 0, CF_MOVE_CASTLE_QUEEN));
}

static void test_castling_rights(void)
{
    CfBoard board;
    CfMove made;
    kings_only(&board);
    board_set_piece(&board, 0, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 0, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board.castling_rights = CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN;
    CHECK(board_make_move(&board, 7, 0, 7, 1, &made));
    CHECK((board.castling_rights & CF_CASTLE_WHITE_KING) == 0U);
    CHECK((board.castling_rights & CF_CASTLE_WHITE_QUEEN) != 0U);
    board_unmake_move(&board, &made);
    CHECK((board.castling_rights & CF_CASTLE_WHITE_KING) != 0U);
}

static void test_en_passant(void)
{
    CfBoard board;
    CfMoveList list;
    CfMove white;
    CfMove black;
    kings_only(&board);
    board_set_piece(&board, 4, 1, CF_PIECE_PAWN, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    CHECK(board_make_move(&board, 4, 1, 4, 3, &white));
    CHECK(board.en_passant_file == 4 && board.en_passant_rank == 2);
    board_generate_legal_moves(&board, 3, 3, &list);
    CHECK(has_move(&list, 4, 2, CF_MOVE_EN_PASSANT));
    CHECK(board_make_move(&board, 3, 3, 4, 2, &black));
    CHECK(board.squares[3][4].type == CF_PIECE_NONE);
    CHECK(board.squares[2][4].type == CF_PIECE_PAWN && board.squares[2][4].color == CF_COLOR_BLACK);
    CHECK(board.halfmove_clock == 0U);
    board_unmake_move(&board, &black);
    CHECK(board.squares[3][4].type == CF_PIECE_PAWN && board.squares[3][4].color == CF_COLOR_WHITE);
    CHECK(board.en_passant_file == 4 && board.en_passant_rank == 2);
    board_unmake_move(&board, &white);
    CHECK(board.squares[1][4].type == CF_PIECE_PAWN);
}

static void test_en_passant_self_check(void)
{
    CfBoard board;
    CfMoveList list;
    board_clear(&board);
    board_set_piece(&board, 4, 4, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 5, CF_PIECE_PAWN, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 5, CF_PIECE_PAWN, CF_COLOR_BLACK);
    board.side_to_move = CF_COLOR_WHITE;
    board.en_passant_file = 3;
    board.en_passant_rank = 6;
    board_generate_legal_moves(&board, 4, 5, &list);
    CHECK(!has_move(&list, 3, 6, CF_MOVE_EN_PASSANT));
}

static void test_promotion(void)
{
    CfBoard board;
    CfMoveList list;
    CfMove made;
    int i;
    int promos = 0;
    kings_only(&board);
    board_set_piece(&board, 0, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    board_generate_legal_moves(&board, 0, 6, &list);
    for (i = 0; i < list.count; ++i)
        if (list.moves[i].to_file == 0 && list.moves[i].to_rank == 7 &&
            (list.moves[i].flags & CF_MOVE_PROMOTION) != 0U) ++promos;
    CHECK(promos == 4);
    CHECK(board_make_move_ex(&board, 0, 6, 0, 7, CF_PIECE_KNIGHT, &made));
    CHECK(board.squares[7][0].type == CF_PIECE_KNIGHT);
    board_unmake_move(&board, &made);
    CHECK(board.squares[6][0].type == CF_PIECE_PAWN);
    CHECK(board_make_move(&board, 0, 6, 0, 7, &made));
    CHECK(board.squares[7][0].type == CF_PIECE_QUEEN);
}

static void test_checkmate_and_stalemate(void)
{
    CfBoard board;
    CfHistory history;
    CfMove move;
    board_init_starting_position(&board);
    board_history_init(&history, &board);
    CHECK(board_make_move(&board, 5, 1, 5, 2, &move)); board_history_record_move(&history, &board, &move);
    CHECK(board_make_move(&board, 4, 6, 4, 4, &move)); board_history_record_move(&history, &board, &move);
    CHECK(board_make_move(&board, 6, 1, 6, 3, &move)); board_history_record_move(&history, &board, &move);
    CHECK(board_make_move(&board, 3, 7, 7, 3, &move)); board_history_record_move(&history, &board, &move);
    CHECK(board_game_status(&board, &history) == CF_GAME_CHECKMATE);

    board_clear(&board);
    board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 5, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 1, 5, CF_PIECE_QUEEN, CF_COLOR_WHITE);
    board.side_to_move = CF_COLOR_BLACK;
    board_history_init(&history, &board);
    CHECK(!board_is_in_check(&board, CF_COLOR_BLACK));
    CHECK(board_game_status(&board, &history) == CF_GAME_STALEMATE);
}

static void test_counters_and_draws(void)
{
    CfBoard board;
    CfHistory history;
    CfMove move;

    kings_only(&board);
    board_set_piece(&board, 1, 0, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 1, 7, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
    CHECK(board_make_move(&board, 1, 0, 2, 2, &move));
    CHECK(board.halfmove_clock == 1U && board.fullmove_number == 1U);
    CHECK(board_make_move(&board, 1, 7, 2, 5, &move));
    CHECK(board.halfmove_clock == 2U && board.fullmove_number == 2U);

    board.halfmove_clock = 100U;
    board_history_init(&history, &board);
    CHECK(board_game_status(&board, &history) == CF_GAME_DRAW_FIFTY_MOVE);

    kings_only(&board);
    board_history_init(&history, &board);
    CHECK(board_is_insufficient_material(&board));
    CHECK(board_game_status(&board, &history) == CF_GAME_DRAW_INSUFFICIENT);

    board_set_piece(&board, 2, 0, CF_PIECE_BISHOP, CF_COLOR_WHITE);
    CHECK(board_is_insufficient_material(&board));
    board_set_piece(&board, 5, 7, CF_PIECE_BISHOP, CF_COLOR_BLACK);
    CHECK(board_is_insufficient_material(&board));
    board_set_piece(&board, 2, 7, CF_PIECE_BISHOP, CF_COLOR_BLACK);
    CHECK(!board_is_insufficient_material(&board));
}

static void record(CfBoard *board, CfHistory *history,
                   int ff, int fr, int tf, int tr)
{
    CfMove move;
    CHECK(board_make_move(board, ff, fr, tf, tr, &move));
    board_history_record_move(history, board, &move);
}

static void test_threefold(void)
{
    CfBoard board;
    CfHistory history;
    board_init_starting_position(&board);
    board_history_init(&history, &board);
    record(&board, &history, 6, 0, 5, 2);
    record(&board, &history, 6, 7, 5, 5);
    record(&board, &history, 5, 2, 6, 0);
    record(&board, &history, 5, 5, 6, 7);
    CHECK(board_history_repetition_count(&history, &board) == 2);
    record(&board, &history, 6, 0, 5, 2);
    record(&board, &history, 6, 7, 5, 5);
    record(&board, &history, 5, 2, 6, 0);
    record(&board, &history, 5, 5, 6, 7);
    CHECK(board_history_repetition_count(&history, &board) == 3);
    CHECK(board_game_status(&board, &history) == CF_GAME_DRAW_THREEFOLD);
}

int main(void)
{
    test_start_and_perft();
    test_kiwipete_perft();
    test_en_passant_perft_position();
    test_promotion_perft_position();
    test_castling();
    test_castling_rights();
    test_en_passant();
    test_en_passant_self_check();
    test_promotion();
    test_checkmate_and_stalemate();
    test_counters_and_draws();
    test_threefold();
    if (failures != 0) {
        printf("Build 4 tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 4 rules tests passed.\n");
    return 0;
}
