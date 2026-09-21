#ifndef CF_CPU_INTERNAL_H
#define CF_CPU_INTERNAL_H
#include "cpu.h"
void cpu_internal_generate_actions(const CfBoard *board,
                                   const CfGasState *gas,
                                   CfCpuActionList *list,
                                   int *actor_in_check);
void cpu_internal_sort_actions(CfCpuActionList *list, CfCpuActionList *scratch);
int cpu_internal_has_legal_action(const CfBoard *board,
                                  const CfGasState *gas,
                                  int *actor_in_check);
int cpu_internal_find_first_king(const CfBoard *board, CfPieceColor color,
                                 int *king_file, int *king_rank);
int cpu_internal_evaluate(const CfBoard *board, const CfGasState *gas);
int cpu_internal_action_bonus(const CfBoard *after, const CfGasState *gas,
                              const CfCpuAction *action,
                              const CfCpuUndo *undo,
                              const CfCpuConfig *config,
                              int actor_was_in_check);
int cpu_internal_action_bonus_prelocated(
                              const CfBoard *after, const CfGasState *gas,
                              const CfCpuAction *action,
                              const CfCpuUndo *undo,
                              const CfCpuConfig *config,
                              int actor_was_in_check,
                              int opponent_king_known,
                              int opponent_king_file,
                              int opponent_king_rank);
#ifdef CF_PROFILE_RUNTIME
typedef struct CfCpuProfileCounters {
    unsigned long full_budget_checks;
    unsigned long node_guard_checks;
    unsigned long action_gen_calls;
    unsigned long sort_calls;
    unsigned long legal_probe_calls;
    unsigned long eval_calls;
    unsigned long move_attempts;
    unsigned long fart_attempts;
    unsigned long fart_bonus_calls;
} CfCpuProfileCounters;
void cpu_internal_profile_reset(void);
const CfCpuProfileCounters *cpu_internal_profile_counters(void);
#endif
#endif
