#ifndef CF_UX_H
#define CF_UX_H

#include "board.h"
#include "cpu.h"
#include "gas.h"

#define CF_UX_HISTORY_CAPACITY 32
#define CF_UX_HISTORY_LINE 40

typedef struct CfUxHistory {
    char lines[CF_UX_HISTORY_CAPACITY][CF_UX_HISTORY_LINE];
    int start;
    int count;
} CfUxHistory;

void ux_history_init(CfUxHistory *history);
void ux_history_add(CfUxHistory *history, const char *line);
const char *ux_history_get(const CfUxHistory *history, int index);

void ux_format_move_line(const char *actor, const CfGasMove *move,
                         char *out, unsigned capacity);
void ux_format_fart_line(const char *actor, const CfFartAction *fart,
                         char *out, unsigned capacity);
void ux_format_cpu_line(const char *actor, const CfCpuAction *action,
                        char *out, unsigned capacity);

const char *ux_terminal_title(CfGameStatus status, CfPieceColor side_to_move);
int ux_board_hit_test(int x, int y, int *file, int *rank);
int ux_build_attract_push(CfBoard *board, CfGasState *gas,
                          CfFartAction *action);

#endif
