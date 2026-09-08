#ifndef CF_CPU_H
#define CF_CPU_H

#include "board.h"
#include "gas.h"

#define CF_CPU_MAX_ACTIONS 512

typedef enum CfCpuDifficulty {
    CF_CPU_EASY = 0,
    CF_CPU_MEDIUM,
    CF_CPU_HARD
} CfCpuDifficulty;

typedef enum CfCpuActionType {
    CF_CPU_ACTION_NONE = 0,
    CF_CPU_ACTION_MOVE,
    CF_CPU_ACTION_FART
} CfCpuActionType;

typedef struct CfCpuConfig {
    CfCpuDifficulty difficulty;
    int max_depth;
    unsigned long node_budget;
    unsigned long time_limit_ms;
    int fart_bias;
} CfCpuConfig;

typedef struct CfCpuAction {
    cf_i8 from_file;
    cf_i8 from_rank;
    cf_i8 to_file;
    cf_i8 to_rank;
    cf_u8 type;
    cf_u8 promotion;
    cf_u8 direction;
    cf_u8 fart_result;
    cf_i16 order_score;
} CfCpuAction;

typedef struct CfCpuActionList {
    CfCpuAction actions[CF_CPU_MAX_ACTIONS];
    int count;
} CfCpuActionList;

typedef union CfCpuUndoPayload {
    CfGasMove move;
    CfFartAction fart;
} CfCpuUndoPayload;

typedef struct CfCpuUndo {
    CfCpuActionType type;
    CfCpuUndoPayload action;
} CfCpuUndo;

typedef struct CfCpuStats {
    unsigned long nodes;
    unsigned long cutoffs;
    int depth_completed;
    int score;
    int budget_hit;
} CfCpuStats;

void cpu_config_for_difficulty(CfCpuConfig *config, CfCpuDifficulty difficulty);
void cpu_cycle_difficulty(CfCpuConfig *config);
const char *cpu_difficulty_name(CfCpuDifficulty difficulty);

void cpu_generate_actions(const CfBoard *board, const CfGasState *gas,
                          CfCpuActionList *list);
int cpu_apply_action(CfBoard *board, CfGasState *gas,
                     const CfCpuAction *action, CfCpuUndo *undo);
void cpu_unapply_action(CfBoard *board, CfGasState *gas,
                        const CfCpuUndo *undo);

int cpu_choose_action(CfBoard *board, CfGasState *gas,
                      const CfGasHistory *history,
                      const CfCpuConfig *config,
                      CfCpuAction *chosen, CfCpuStats *stats);

void cpu_format_action(const CfCpuAction *action, char *out, unsigned capacity);

#endif
