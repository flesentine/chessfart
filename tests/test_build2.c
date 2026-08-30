#include <stdio.h>

#include "board.h"

static int expect_piece(const CfBoard *board, int file, int rank,
                        CfPieceType type, CfPieceColor color)
{
    const CfPiece *piece = board_piece_at(board, file, rank);
    if (piece == 0 || piece->type != type || piece->color != color) {
        return 0;
    }
    return 1;
}

int main(void)
{
    CfBoard board;

    board_init_starting_position(&board);

    if (board_piece_count(&board) != 32) {
        fprintf(stderr, "expected 32 starting pieces\n");
        return 1;
    }
    if (!expect_piece(&board, 0, 0, CF_PIECE_ROOK, CF_COLOR_WHITE) ||
        !expect_piece(&board, 3, 0, CF_PIECE_QUEEN, CF_COLOR_WHITE) ||
        !expect_piece(&board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE) ||
        !expect_piece(&board, 4, 1, CF_PIECE_PAWN, CF_COLOR_WHITE) ||
        !expect_piece(&board, 4, 6, CF_PIECE_PAWN, CF_COLOR_BLACK) ||
        !expect_piece(&board, 4, 7, CF_PIECE_KING, CF_COLOR_BLACK) ||
        !expect_piece(&board, 7, 7, CF_PIECE_ROOK, CF_COLOR_BLACK)) {
        fprintf(stderr, "starting position mismatch\n");
        return 1;
    }
    if (board_piece_at(&board, -1, 0) != 0 ||
        board_piece_at(&board, 8, 0) != 0 ||
        board_piece_at(&board, 0, 8) != 0) {
        fprintf(stderr, "out-of-range board access should return null\n");
        return 1;
    }
    if (board_piece_letter(CF_PIECE_KNIGHT) != 'N' ||
        board_piece_letter(CF_PIECE_KING) != 'K') {
        fprintf(stderr, "piece labels mismatch\n");
        return 1;
    }

    printf("Build 2 board tests passed.\n");
    return 0;
}
