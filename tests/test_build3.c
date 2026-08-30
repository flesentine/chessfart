#include <stdio.h>

#include "board.h"

static int failures;

static void check(int condition, const char *name)
{
    if (!condition) {
        printf("FAIL: %s\n", name);
        ++failures;
    }
}

static int has_destination(const CfMoveList *list, int file, int rank)
{
    int i;
    for (i = 0; i < list->count; ++i)
        if (list->moves[i].to_file == file && list->moves[i].to_rank == rank) return 1;
    return 0;
}

static void test_starting_position(void)
{
    CfBoard board;
    CfMoveList list;
    board_init_starting_position(&board);
    check(board_piece_count(&board) == 32, "starting position has 32 pieces");
    check(board.side_to_move == CF_COLOR_WHITE, "white starts");
    check(board_count_legal_moves(&board, CF_COLOR_WHITE) == 20, "white has 20 opening moves");
    check(board_count_legal_moves(&board, CF_COLOR_BLACK) == 20, "black has 20 opening moves");
    board_generate_legal_moves(&board, 4, 1, &list);
    check(list.count == 2, "E2 pawn has two opening moves");
    check(has_destination(&list, 4, 2), "E2 pawn can move to E3");
    check(has_destination(&list, 4, 3), "E2 pawn can move to E4");
    board_generate_legal_moves(&board, 2, 0, &list);
    check(list.count == 0, "C1 bishop starts blocked");
    board_generate_legal_moves(&board, 1, 0, &list);
    check(list.count == 2, "B1 knight has two opening moves");
}

static void test_moves_turns_and_capture(void)
{
    CfBoard board;
    CfMove move;
    const CfPiece *piece;
    board_init_starting_position(&board);
    check(board_make_move(&board, 4, 1, 4, 3, &move), "E2-E4 legal");
    check(board.side_to_move == CF_COLOR_BLACK, "turn changes to black");
    check(!board_make_move(&board, 3, 1, 3, 3, 0), "white cannot move twice");
    check(board_make_move(&board, 3, 6, 3, 4, 0), "D7-D5 legal");
    check(board_make_move(&board, 4, 3, 3, 4, &move), "E4 captures D5");
    check(move.captured.type == CF_PIECE_PAWN && move.captured.color == CF_COLOR_BLACK, "capture records victim");
    piece = board_piece_at(&board, 3, 4);
    check(piece != 0 && piece->type == CF_PIECE_PAWN && piece->color == CF_COLOR_WHITE, "capturing pawn lands D5");
}

static void test_attack_and_check(void)
{
    CfBoard board;
    board_clear(&board);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    check(board_square_is_attacked(&board, 4, 0, CF_COLOR_BLACK), "rook attacks E1");
    check(board_is_in_check(&board, CF_COLOR_WHITE), "white king is in check");
}

static void test_pin_self_check_filter(void)
{
    CfBoard board;
    CfMoveList list;
    board_clear(&board);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 4, 1, CF_PIECE_ROOK, CF_COLOR_WHITE);
    board_set_piece(&board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board.side_to_move = CF_COLOR_WHITE;
    board_generate_legal_moves(&board, 4, 1, &list);
    check(!has_destination(&list, 5, 1), "pinned rook cannot expose own king");
    check(has_destination(&list, 4, 2), "pinned rook may stay on blocking file");
}

static void test_king_safety(void)
{
    CfBoard board;
    CfMoveList list;
    board_clear(&board);
    board_set_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 5, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    board.side_to_move = CF_COLOR_WHITE;
    board_generate_legal_moves(&board, 4, 0, &list);
    check(!has_destination(&list, 5, 0), "king cannot move onto attacked F1");
}

static unsigned long perft(CfBoard *board, int depth)
{
    CfMoveList list;
    CfMove move;
    unsigned long nodes = 0;
    int file, rank, i;
    if (depth == 0) return 1UL;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].color != board->side_to_move) continue;
            board_generate_legal_moves(board, file, rank, &list);
            for (i = 0; i < list.count; ++i) {
                if (board_make_move(board, file, rank, list.moves[i].to_file, list.moves[i].to_rank, &move)) {
                    nodes += perft(board, depth - 1);
                    board_unmake_move(board, &move);
                }
            }
        }
    }
    return nodes;
}

static void test_opening_perft(void)
{
    CfBoard board;
    board_init_starting_position(&board);
    check(perft(&board, 1) == 20UL, "perft depth 1 is 20");
    check(perft(&board, 2) == 400UL, "perft depth 2 is 400");
    check(perft(&board, 3) == 8902UL, "perft depth 3 is 8902");
}

static void test_unmake(void)
{
    CfBoard board;
    CfMove move;
    board_init_starting_position(&board);
    check(board_make_move(&board, 4, 1, 4, 3, &move), "move before unmake");
    board_unmake_move(&board, &move);
    check(board.side_to_move == CF_COLOR_WHITE, "unmake restores turn");
    check(board_piece_at(&board, 4, 1)->type == CF_PIECE_PAWN, "unmake restores source");
    check(board_piece_at(&board, 4, 3)->type == CF_PIECE_NONE, "unmake restores destination");
}

int main(void)
{
    test_starting_position();
    test_moves_turns_and_capture();
    test_attack_and_check();
    test_pin_self_check_filter();
    test_king_safety();
    test_unmake();
    test_opening_perft();
    if (failures != 0) {
        printf("Build 3 tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 3 rules tests passed.\n");
    return 0;
}
