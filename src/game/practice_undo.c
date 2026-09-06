#include <string.h>

#include "practice_undo.h"

static CfPracticeUndoEntry *practice_undo_push(CfPracticeUndoJournal *journal,
                                               const CfGasHistory *history)
{
    CfPracticeUndoEntry *entry;
    int index;

    if (journal == 0 || history == 0 ||
        history->count < 0 || history->count > CF_GAS_HISTORY)
        return 0;

    if (journal->count < CF_PRACTICE_UNDO_CAPACITY) {
        index = (journal->start + journal->count) %
                CF_PRACTICE_UNDO_CAPACITY;
        ++journal->count;
    } else {
        index = journal->start;
        journal->start = (journal->start + 1) %
                         CF_PRACTICE_UNDO_CAPACITY;
    }

    entry = &journal->entries[index];
    memset(entry, 0, sizeof(*entry));
    entry->history_count_before = history->count;
    if (history->count == CF_GAS_HISTORY) {
        entry->history_dropped_oldest = 1;
        entry->dropped_history_key = history->keys[0];
    }
    return entry;
}

void practice_undo_init(CfPracticeUndoJournal *journal)
{
    if (journal == 0) return;
    memset(journal, 0, sizeof(*journal));
}

int practice_undo_can_undo(const CfPracticeUndoJournal *journal)
{
    return journal != 0 && journal->count > 0;
}

void practice_undo_record_move(CfPracticeUndoJournal *journal,
                               const CfGasMove *move,
                               const CfGasHistory *history_before_record)
{
    CfPracticeUndoEntry *entry;
    if (move == 0) return;
    entry = practice_undo_push(journal, history_before_record);
    if (entry == 0) return;
    entry->type = CF_PRACTICE_UNDO_MOVE;
    entry->action.move = *move;
}

void practice_undo_record_fart(CfPracticeUndoJournal *journal,
                               const CfFartAction *fart,
                               const CfGasHistory *history_before_record)
{
    CfPracticeUndoEntry *entry;
    if (fart == 0) return;
    entry = practice_undo_push(journal, history_before_record);
    if (entry == 0) return;
    entry->type = CF_PRACTICE_UNDO_FART;
    entry->action.fart = *fart;
}

static int practice_undo_history_shape_matches(
    const CfPracticeUndoEntry *entry,
    const CfGasHistory *history)
{
    int expected;
    if (entry == 0 || history == 0) return 0;
    expected = entry->history_count_before < CF_GAS_HISTORY ?
               entry->history_count_before + 1 : CF_GAS_HISTORY;
    return history->count == expected;
}

static void practice_undo_restore_history(
    const CfPracticeUndoEntry *entry,
    CfGasHistory *history)
{
    int i;
    if (entry->history_dropped_oldest) {
        for (i = CF_GAS_HISTORY - 1; i > 0; --i)
            history->keys[i] = history->keys[i - 1];
        history->keys[0] = entry->dropped_history_key;
        history->count = CF_GAS_HISTORY;
    } else {
        history->count = entry->history_count_before;
    }
}

int practice_undo_apply_last(CfPracticeUndoJournal *journal,
                             CfBoard *board,
                             CfGasState *gas,
                             CfGasHistory *history)
{
    CfPracticeUndoEntry *entry;
    int index;

    if (journal == 0 || board == 0 || gas == 0 || history == 0 ||
        journal->count <= 0) return 0;

    index = (journal->start + journal->count - 1) %
            CF_PRACTICE_UNDO_CAPACITY;
    entry = &journal->entries[index];
    if (!practice_undo_history_shape_matches(entry, history)) return 0;

    if (entry->type == CF_PRACTICE_UNDO_MOVE)
        gas_unmake_move(board, gas, &entry->action.move);
    else if (entry->type == CF_PRACTICE_UNDO_FART)
        gas_unmake_fart(board, gas, &entry->action.fart);
    else
        return 0;

    practice_undo_restore_history(entry, history);
    --journal->count;
    if (journal->count == 0) journal->start = 0;
    return 1;
}
