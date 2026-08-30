#ifndef CF_BOARD_H
#define CF_BOARD_H

#define CF_BOARD_SIZE 8
#define CF_MAX_MOVES 64

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

typedef struct CfMove {
    int from_file;
    int from_rank;
    int to_file;
    int to_rank;
    CfPiece moved;
    CfPiece captured;
} CfMove;

typedef struct CfMoveList {
    CfMove moves[CF_MAX_MOVES];
    int count;
} CfMoveList;

typedef struct CfBoard {
    CfPiece squares[CF_BOARD_SIZE][CF_BOARD_SIZE];
    CfPieceColor side_to_move;
} CfBoard;

void board_clear(CfBoard *board);
void board_init_starting_position(CfBoard *board);
const CfPiece *board_piece_at(const CfBoard *board, int file, int rank);
void board_set_piece(CfBoard *board, int file, int rank, CfPieceType type, CfPieceColor color);
int board_piece_count(const CfBoard *board);
const char *board_piece_type_name(CfPieceType type);
const char *board_piece_color_name(CfPieceColor color);
char board_piece_letter(CfPieceType type);
CfPieceColor board_other_color(CfPieceColor color);

int board_square_is_attacked(const CfBoard *board, int file, int rank, CfPieceColor by_color);
int board_is_in_check(const CfBoard *board, CfPieceColor color);
void board_generate_legal_moves(const CfBoard *board, int file, int rank, CfMoveList *list);
int board_move_is_legal(const CfBoard *board, int from_file, int from_rank, int to_file, int to_rank);
int board_make_move(CfBoard *board, int from_file, int from_rank, int to_file, int to_rank, CfMove *made_move);
void board_unmake_move(CfBoard *board, const CfMove *move);
int board_count_legal_moves(const CfBoard *board, CfPieceColor color);

#endif
