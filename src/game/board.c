#include <string.h>

#include "board.h"

static CfPiece make_piece(CfPieceType type, CfPieceColor color)
{
    CfPiece piece;
    piece.type = type;
    piece.color = color;
    return piece;
}

void board_init_starting_position(CfBoard *board)
{
    static const CfPieceType back_rank[8] = {
        CF_PIECE_ROOK, CF_PIECE_KNIGHT, CF_PIECE_BISHOP, CF_PIECE_QUEEN,
        CF_PIECE_KING, CF_PIECE_BISHOP, CF_PIECE_KNIGHT, CF_PIECE_ROOK
    };
    int file;

    if (board == 0) {
        return;
    }

    memset(board, 0, sizeof(*board));

    for (file = 0; file < 8; ++file) {
        board->squares[0][file] = make_piece(back_rank[file], CF_COLOR_WHITE);
        board->squares[1][file] = make_piece(CF_PIECE_PAWN, CF_COLOR_WHITE);
        board->squares[6][file] = make_piece(CF_PIECE_PAWN, CF_COLOR_BLACK);
        board->squares[7][file] = make_piece(back_rank[file], CF_COLOR_BLACK);
    }
}

const CfPiece *board_piece_at(const CfBoard *board, int file, int rank)
{
    if (board == 0 || file < 0 || file > 7 || rank < 0 || rank > 7) {
        return 0;
    }
    return &board->squares[rank][file];
}

int board_piece_count(const CfBoard *board)
{
    int file;
    int rank;
    int count = 0;

    if (board == 0) {
        return 0;
    }

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].type != CF_PIECE_NONE) {
                ++count;
            }
        }
    }
    return count;
}

const char *board_piece_type_name(CfPieceType type)
{
    switch (type) {
    case CF_PIECE_PAWN: return "PAWN";
    case CF_PIECE_KNIGHT: return "KNIGHT";
    case CF_PIECE_BISHOP: return "BISHOP";
    case CF_PIECE_ROOK: return "ROOK";
    case CF_PIECE_QUEEN: return "QUEEN";
    case CF_PIECE_KING: return "KING";
    default: return "EMPTY";
    }
}

const char *board_piece_color_name(CfPieceColor color)
{
    switch (color) {
    case CF_COLOR_WHITE: return "WHITE";
    case CF_COLOR_BLACK: return "BLACK";
    default: return "NONE";
    }
}

char board_piece_letter(CfPieceType type)
{
    switch (type) {
    case CF_PIECE_PAWN: return 'P';
    case CF_PIECE_KNIGHT: return 'N';
    case CF_PIECE_BISHOP: return 'B';
    case CF_PIECE_ROOK: return 'R';
    case CF_PIECE_QUEEN: return 'Q';
    case CF_PIECE_KING: return 'K';
    default: return ' ';
    }
}
