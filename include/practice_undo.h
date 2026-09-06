#ifndef CF_PRACTICE_UNDO_H
#define CF_PRACTICE_UNDO_H

#include "gas.h"

#define CF_PRACTICE_UNDO_CAPACITY 32

typedef enum CfPracticeUndoType {
    CF_PRACTICE_UNDO_NONE = 0,
    CF_PRACTICE_UNDO_MOVE,
    CF_PRACTICE_UNDO_FART
} CfPracticeUndoType;

typedef struct CfPracticeUndoEntry {
    CfPracticeUndoType type;
    int history_count_before;
    int history_dropped_oldest;
    CfGasPositionKey dropped_history_key;
    union {
        CfGasMove move;
        CfFartAction fart;
    } action;
} CfPracticeUndoEntry;

typedef struct CfPracticeUndoJournal {
    CfPracticeUndoEntry entries[CF_PRACTICE_UNDO_CAPACITY];
    int start;
    int count;
} CfPracticeUndoJournal;

void practice_undo_init(CfPracticeUndoJournal *journal);
int practice_undo_can_undo(const CfPracticeUndoJournal *journal);
void practice_undo_record_move(CfPracticeUndoJournal *journal,
                               const CfGasMove *move,
                               const CfGasHistory *history_before_record);
void practice_undo_record_fart(CfPracticeUndoJournal *journal,
                               const CfFartAction *fart,
                               const CfGasHistory *history_before_record);
int practice_undo_apply_last(CfPracticeUndoJournal *journal,
                             CfBoard *board,
                             CfGasState *gas,
                             CfGasHistory *history);

#endif
