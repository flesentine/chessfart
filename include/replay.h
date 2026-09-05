#ifndef CF_REPLAY_H
#define CF_REPLAY_H

#include "board.h"
#include "gas.h"
#include "match_mode.h"

#define CF_REPLAY_CAPACITY 256
#define CF_REPLAY_LABEL 40

typedef struct CfReplaySnapshot {
    cf_u8 squares[64];
    cf_u8 side_to_move;
    cf_u8 castling_rights;
    cf_u8 en_passant_file;
    cf_u8 en_passant_rank;
    cf_u16 halfmove_low;
    cf_u16 halfmove_high;
    cf_u16 fullmove_low;
    cf_u16 fullmove_high;
    cf_u8 status;
    cf_u8 match_mode;
    char label[CF_REPLAY_LABEL];
} CfReplaySnapshot;

typedef struct CfReplayTimeline {
    CfReplaySnapshot snapshots[CF_REPLAY_CAPACITY];
    int start;
    int count;
    unsigned long total;
    int truncated;
} CfReplayTimeline;

void replay_timeline_init(CfReplayTimeline *timeline);
void replay_timeline_reset(CfReplayTimeline *timeline,
                           const CfBoard *board,
                           const CfGasState *gas,
                           CfGameStatus status,
                           CfMatchMode match_mode,
                           const char *label);
void replay_timeline_record(CfReplayTimeline *timeline,
                            const CfBoard *board,
                            const CfGasState *gas,
                            CfGameStatus status,
                            CfMatchMode match_mode,
                            const char *label);
const CfReplaySnapshot *replay_timeline_get(const CfReplayTimeline *timeline,
                                            int index);
int replay_snapshot_restore(const CfReplaySnapshot *snapshot,
                            CfBoard *board,
                            CfGasState *gas,
                            CfGameStatus *status,
                            CfMatchMode *match_mode);
int replay_snapshot_matches(const CfReplaySnapshot *snapshot,
                            const CfBoard *board,
                            const CfGasState *gas,
                            CfGameStatus status,
                            CfMatchMode match_mode);

#endif
