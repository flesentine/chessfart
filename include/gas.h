#ifndef CF_GAS_H
#define CF_GAS_H

#include "board.h"
#include "cf_types.h"

#define CF_GAS_MAX 3U
#define CF_GAS_FART_COST 2U
#define CF_GAS_HISTORY 128

typedef struct CfGasState {
    cf_u8 squares[CF_BOARD_SIZE][CF_BOARD_SIZE];
} CfGasState;

typedef struct CfGasMove {
    CfMove chess_move;
    CfGasState previous_gas;
} CfGasMove;

typedef enum CfFartDirection {
    CF_FART_N = 0,
    CF_FART_NE,
    CF_FART_E,
    CF_FART_SE,
    CF_FART_S,
    CF_FART_SW,
    CF_FART_W,
    CF_FART_NW
} CfFartDirection;

typedef enum CfFartPreview {
    CF_FART_INVALID = 0,
    CF_FART_PUFF,
    CF_FART_PUSH_BUILD6
} CfFartPreview;

typedef struct CfFartAction {
    int actor_file;
    int actor_rank;
    CfFartDirection direction;
    cf_u8 previous_gas;
    CfPieceColor previous_side;
    int previous_ep_file;
    int previous_ep_rank;
    unsigned previous_halfmove;
    unsigned previous_fullmove;
} CfFartAction;

typedef struct CfGasPositionKey {
    cf_u8 squares[64];
    cf_u8 state;
    cf_u8 en_passant_file;
} CfGasPositionKey;

typedef struct CfGasHistory {
    CfGasPositionKey keys[CF_GAS_HISTORY];
    int count;
} CfGasHistory;

void gas_init(CfGasState *gas);
cf_u8 gas_at(const CfGasState *gas, int file, int rank);
void gas_set(CfGasState *gas, int file, int rank, cf_u8 value);

int gas_make_move(CfBoard *board, CfGasState *gas,
                  int from_file, int from_rank, int to_file, int to_rank,
                  CfGasMove *made_move);
int gas_make_move_ex(CfBoard *board, CfGasState *gas,
                     int from_file, int from_rank, int to_file, int to_rank,
                     CfPieceType promotion, CfGasMove *made_move);
void gas_unmake_move(CfBoard *board, CfGasState *gas, const CfGasMove *move);

int gas_piece_can_fart(const CfBoard *board, const CfGasState *gas, int file, int rank);
CfFartPreview gas_preview_fart(const CfBoard *board, const CfGasState *gas,
                               int file, int rank, CfFartDirection direction);
int gas_make_puff(CfBoard *board, CfGasState *gas,
                  int file, int rank, CfFartDirection direction,
                  CfFartAction *action);
void gas_unmake_puff(CfBoard *board, CfGasState *gas, const CfFartAction *action);
const char *gas_direction_name(CfFartDirection direction);

void gas_history_init(CfGasHistory *history, const CfBoard *board, const CfGasState *gas);
void gas_history_record(CfGasHistory *history, const CfBoard *board, const CfGasState *gas);
int gas_history_repetition_count(const CfGasHistory *history,
                                 const CfBoard *board, const CfGasState *gas);
CfGameStatus gas_game_status(const CfBoard *board, const CfGasState *gas,
                             const CfGasHistory *history);

#endif
