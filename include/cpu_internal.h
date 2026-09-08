#ifndef CF_CPU_INTERNAL_H
#define CF_CPU_INTERNAL_H
#include "cpu.h"
void cpu_internal_generate_actions(const CfBoard *board,
                                   const CfGasState *gas,
                                   CfCpuActionList *list,
                                   int *actor_in_check);
void cpu_internal_sort_actions(CfCpuActionList *list, CfCpuActionList *scratch);
int cpu_internal_has_legal_action(const CfBoard *board,
                                  const CfGasState *gas);
int cpu_internal_evaluate(const CfBoard *board, const CfGasState *gas);
int cpu_internal_action_bonus(const CfBoard *after, const CfGasState *gas,
                              const CfCpuAction *action,
                              const CfCpuUndo *undo,
                              const CfCpuConfig *config,
                              int actor_was_in_check);
#endif
