#ifndef CF_BOARD_H
#define CF_BOARD_H

#include "cf_types.h"

#define CF_BOARD_SIZE 8
#define CF_MAX_MOVES 64
#define CF_MAX_HISTORY 128

#define CF_CASTLE_WHITE_KING  0x01U
#define CF_CASTLE_WHITE_QUEEN 0x02U
#define CF_CASTLE_BLACK_KING  0x04U
#define CF_CASTLE_BLACK_QUEEN 0x08U
#define CF_CASTLE_ALL          0x0FU

#define CF_MOVE_EN_PASSANT    0x01U
#define CF_MOVE_CASTLE_KING   0x02U
#define CF_MOVE_CASTLE_QUEEN  0x04U
#define CF_MOVE_PROMOTION     0x08U
#define CF_MOVE_PAWN_DOUBLE   0x10U

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

typedef enum CfGameStatus {
    CF_GAME_ONGOING = 0,
    CF_GAME_CHECK,
    CF_GAME_CHECKMATE,
    CF_GAME_STALEMATE,
    CF_GAME_DRAW_FIFTY_MOVE,
    CF_GAME_DRAW_THREEFOLD,
    CF_GAME_DRAW_INSUFFICIENT
} CfGameStatus;

typedef struct CfPiece {
    CfPieceType type;
    CfPieceColor color;
} CfPiece;

typedef struct CfMove {
    int from_file;
    int from_rank;
    int to_file;
    int to_rank;
    int captured_file;
    int captured_rank;
    CfPiece moved;
    CfPiece captured;
    CfPieceType promotion;
    unsigned flags;

    CfPieceColor prev_side_to_move;
    unsigned prev_castling_rights;
    int prev_en_passant_file;
    int prev_en_passant_rank;
    unsigned prev_halfmove_clock;
    unsigned prev_fullmove_number;
} CfMove;

typedef struct CfMoveList {
    CfMove moves[CF_MAX_MOVES];
    int count;
} CfMoveList;

typedef struct CfBoard {
    CfPiece squares[CF_BOARD_SIZE][CF_BOARD_SIZE];
    CfPieceColor side_to_move;
    unsigned castling_rights;
    int en_passant_file;
    int en_passant_rank;
    unsigned halfmove_clock;
    unsigned fullmove_number;
} CfBoard;

typedef struct CfPositionKey {
    cf_u8 packed_squares[32];
    cf_u8 state;
    cf_u8 en_passant_file;
} CfPositionKey;

typedef struct CfHistory {
    CfPositionKey keys[CF_MAX_HISTORY];
    int count;
} CfHistory;

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
int board_make_move_ex(CfBoard *board, int from_file, int from_rank, int to_file, int to_rank,
                       CfPieceType promotion, CfMove *made_move);
void board_unmake_move(CfBoard *board, const CfMove *move);
int board_count_legal_moves(const CfBoard *board, CfPieceColor color);

void board_history_init(CfHistory *history, const CfBoard *board);
void board_history_record_move(CfHistory *history, const CfBoard *board, const CfMove *move);
int board_history_repetition_count(const CfHistory *history, const CfBoard *board);
int board_is_insufficient_material(const CfBoard *board);
CfGameStatus board_game_status(const CfBoard *board, const CfHistory *history);
const char *board_game_status_name(CfGameStatus status);

#endif
