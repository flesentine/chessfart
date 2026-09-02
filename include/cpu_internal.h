#ifndef CF_CPU_INTERNAL_H
#define CF_CPU_INTERNAL_H
#include "cpu.h"
void cpu_internal_sort_actions(CfCpuActionList *list);
int cpu_internal_evaluate(const CfBoard *board, const CfGasState *gas);
int cpu_internal_action_bonus(const CfBoard *after, const CfGasState *gas,
                              const CfCpuAction *action,
                              const CfCpuUndo *undo,
                              const CfCpuConfig *config,
                              int actor_was_in_check);
#endif
