#ifndef CF_BOARD_H
#define CF_BOARD_H

typedef enum CfPieceType {
    CF_PIECE_NONE = 0,
    CF_PIECE_PAWN,
    CF_PIECE_KNIGHT,
    CF_PIECE_BISHOP,
    CF_PIECE_ROOK,
    CF_PIECE_QUEEN,
    CF_PIECE_KING
} CfPieceType;

typedef enum CfPieceColor {
    CF_COLOR_NONE = 0,
    CF_COLOR_WHITE,
    CF_COLOR_BLACK
} CfPieceColor;

typedef struct CfPiece {
    CfPieceType type;
    CfPieceColor color;
} CfPiece;

typedef struct CfBoard {
    CfPiece squares[8][8];
} CfBoard;

void board_init_starting_position(CfBoard *board);
const CfPiece *board_piece_at(const CfBoard *board, int file, int rank);
int board_piece_count(const CfBoard *board);
const char *board_piece_type_name(CfPieceType type);
const char *board_piece_color_name(CfPieceColor color);
char board_piece_letter(CfPieceType type);

#endif
