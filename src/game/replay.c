#include <string.h>

#include "replay.h"

#define CF_REPLAY_TYPE_MASK 0x07U
#define CF_REPLAY_COLOR_MASK 0x18U
#define CF_REPLAY_GAS_MASK 0x60U
#define CF_REPLAY_COLOR_SHIFT 3
#define CF_REPLAY_GAS_SHIFT 5

static cf_u8 replay_pack_square(const CfPiece *piece, cf_u8 gas)
{
    unsigned type = 0U;
    unsigned color = 0U;
    if (piece != 0) {
        type = (unsigned)piece->type;
        color = (unsigned)piece->color;
    }
    return (cf_u8)((type & 7U) |
                   ((color & 3U) << CF_REPLAY_COLOR_SHIFT) |
                   (((unsigned)gas & 3U) << CF_REPLAY_GAS_SHIFT));
}

static int replay_valid_piece(unsigned type, unsigned color)
{
    if (type > (unsigned)CF_PIECE_KING) return 0;
    if (color > (unsigned)CF_COLOR_BLACK) return 0;
    if (type == (unsigned)CF_PIECE_NONE)
        return color == (unsigned)CF_COLOR_NONE;
    return color == (unsigned)CF_COLOR_WHITE ||
           color == (unsigned)CF_COLOR_BLACK;
}

static void replay_capture(CfReplaySnapshot *snapshot,
                           const CfBoard *board,
                           const CfGasState *gas,
                           CfGameStatus status,
                           CfMatchMode match_mode,
                           const char *label)
{
    int file;
    int rank;
    int square = 0;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            snapshot->squares[square++] =
                replay_pack_square(board_piece_at(board, file, rank),
                                   gas_at(gas, file, rank));
        }
    }

    snapshot->side_to_move = (cf_u8)board->side_to_move;
    snapshot->castling_rights = (cf_u8)board->castling_rights;
    snapshot->en_passant_file =
        (cf_u8)(board->en_passant_file < 0 ? 0 :
                board->en_passant_file + 1);
    snapshot->en_passant_rank =
        (cf_u8)(board->en_passant_rank < 0 ? 0 :
                board->en_passant_rank + 1);
    {
        unsigned long halfmove = (unsigned long)board->halfmove_clock;
        unsigned long fullmove = (unsigned long)board->fullmove_number;
        snapshot->halfmove_low = (cf_u16)(halfmove & 0xFFFFUL);
        snapshot->halfmove_high = (cf_u16)((halfmove >> 16) & 0xFFFFUL);
        snapshot->fullmove_low = (cf_u16)(fullmove & 0xFFFFUL);
        snapshot->fullmove_high = (cf_u16)((fullmove >> 16) & 0xFFFFUL);
    }
    snapshot->status = (cf_u8)status;
    snapshot->match_mode = (cf_u8)match_mode;

    if (label == 0) label = "";
    strncpy(snapshot->label, label, CF_REPLAY_LABEL - 1U);
    snapshot->label[CF_REPLAY_LABEL - 1U] = '\0';
}

void replay_timeline_init(CfReplayTimeline *timeline)
{
    if (timeline == 0) return;
    memset(timeline, 0, sizeof(*timeline));
}

void replay_timeline_reset(CfReplayTimeline *timeline,
                           const CfBoard *board,
                           const CfGasState *gas,
                           CfGameStatus status,
                           CfMatchMode match_mode,
                           const char *label)
{
    if (timeline == 0) return;
    replay_timeline_init(timeline);
    replay_timeline_record(timeline, board, gas, status, match_mode, label);
}

void replay_timeline_record(CfReplayTimeline *timeline,
                            const CfBoard *board,
                            const CfGasState *gas,
                            CfGameStatus status,
                            CfMatchMode match_mode,
                            const char *label)
{
    int index;
    if (timeline == 0 || board == 0 || gas == 0) return;

    if (timeline->count < CF_REPLAY_CAPACITY) {
        index = (timeline->start + timeline->count) % CF_REPLAY_CAPACITY;
        ++timeline->count;
    } else {
        index = timeline->start;
        timeline->start = (timeline->start + 1) % CF_REPLAY_CAPACITY;
        timeline->truncated = 1;
    }

    replay_capture(&timeline->snapshots[index], board, gas,
                   status, match_mode, label);
    ++timeline->total;
}

const CfReplaySnapshot *replay_timeline_get(const CfReplayTimeline *timeline,
                                            int index)
{
    int physical;
    if (timeline == 0 || index < 0 || index >= timeline->count) return 0;
    physical = (timeline->start + index) % CF_REPLAY_CAPACITY;
    return &timeline->snapshots[physical];
}

int replay_snapshot_restore(const CfReplaySnapshot *snapshot,
                            CfBoard *board,
                            CfGasState *gas,
                            CfGameStatus *status,
                            CfMatchMode *match_mode)
{
    CfBoard decoded_board;
    CfGasState decoded_gas;
    int file;
    int rank;
    int square = 0;
    unsigned packed;
    unsigned type;
    unsigned color;
    unsigned square_gas;

    if (snapshot == 0 || board == 0 || gas == 0) return 0;
    if (snapshot->side_to_move != (cf_u8)CF_COLOR_WHITE &&
        snapshot->side_to_move != (cf_u8)CF_COLOR_BLACK) return 0;
    if ((snapshot->castling_rights & (cf_u8)~CF_CASTLE_ALL) != 0U)
        return 0;
    if (snapshot->en_passant_file > 8U ||
        snapshot->en_passant_rank > 8U) return 0;
    if ((snapshot->en_passant_file == 0U) !=
        (snapshot->en_passant_rank == 0U)) return 0;
    if (snapshot->status > (cf_u8)CF_GAME_DRAW_INSUFFICIENT) return 0;
    if (snapshot->match_mode != (cf_u8)CF_MATCH_CPU &&
        snapshot->match_mode != (cf_u8)CF_MATCH_LOCAL) return 0;

    board_clear(&decoded_board);
    gas_init(&decoded_gas);

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            packed = (unsigned)snapshot->squares[square++];
            if ((packed & 0x80U) != 0U) return 0;
            type = packed & CF_REPLAY_TYPE_MASK;
            color = (packed & CF_REPLAY_COLOR_MASK) >>
                    CF_REPLAY_COLOR_SHIFT;
            square_gas = (packed & CF_REPLAY_GAS_MASK) >>
                         CF_REPLAY_GAS_SHIFT;
            if (!replay_valid_piece(type, color)) return 0;
            board_set_piece(&decoded_board, file, rank,
                            (CfPieceType)type, (CfPieceColor)color);
            gas_set(&decoded_gas, file, rank, (cf_u8)square_gas);
        }
    }

    decoded_board.side_to_move = (CfPieceColor)snapshot->side_to_move;
    decoded_board.castling_rights = (unsigned)snapshot->castling_rights;
    decoded_board.en_passant_file = snapshot->en_passant_file == 0U ?
                                    -1 : (int)snapshot->en_passant_file - 1;
    decoded_board.en_passant_rank = snapshot->en_passant_rank == 0U ?
                                    -1 : (int)snapshot->en_passant_rank - 1;
    {
        unsigned long halfmove =
            ((unsigned long)snapshot->halfmove_high << 16) |
            (unsigned long)snapshot->halfmove_low;
        unsigned long fullmove =
            ((unsigned long)snapshot->fullmove_high << 16) |
            (unsigned long)snapshot->fullmove_low;
        if (sizeof(unsigned) < 4U &&
            (snapshot->halfmove_high != 0U ||
             snapshot->fullmove_high != 0U)) return 0;
        decoded_board.halfmove_clock = (unsigned)halfmove;
        decoded_board.fullmove_number = (unsigned)fullmove;
    }

    *board = decoded_board;
    *gas = decoded_gas;
    if (status != 0) *status = (CfGameStatus)snapshot->status;
    if (match_mode != 0)
        *match_mode = (CfMatchMode)snapshot->match_mode;
    return 1;
}

int replay_snapshot_matches(const CfReplaySnapshot *snapshot,
                            const CfBoard *board,
                            const CfGasState *gas,
                            CfGameStatus status,
                            CfMatchMode match_mode)
{
    CfBoard restored_board;
    CfGasState restored_gas;
    CfGameStatus restored_status;
    CfMatchMode restored_mode;
    int file;
    int rank;
    const CfPiece *a;
    const CfPiece *b;

    if (snapshot == 0 || board == 0 || gas == 0) return 0;
    if (!replay_snapshot_restore(snapshot, &restored_board, &restored_gas,
                                 &restored_status, &restored_mode)) return 0;
    if (restored_status != status || restored_mode != match_mode) return 0;
    if (restored_board.side_to_move != board->side_to_move ||
        restored_board.castling_rights != board->castling_rights ||
        restored_board.en_passant_file != board->en_passant_file ||
        restored_board.en_passant_rank != board->en_passant_rank ||
        restored_board.halfmove_clock != board->halfmove_clock ||
        restored_board.fullmove_number != board->fullmove_number) return 0;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            a = board_piece_at(&restored_board, file, rank);
            b = board_piece_at(board, file, rank);
            if (a == 0 || b == 0 ||
                a->type != b->type || a->color != b->color ||
                gas_at(&restored_gas, file, rank) != gas_at(gas, file, rank))
                return 0;
        }
    }
    return 1;
}
