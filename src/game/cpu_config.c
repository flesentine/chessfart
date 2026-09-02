#include "cpu.h"

void cpu_config_for_difficulty(CfCpuConfig *config, CfCpuDifficulty difficulty)
{
    if (config == 0) return;
    config->difficulty = difficulty;
    if (difficulty == CF_CPU_EASY) {
        config->max_depth = 1;
        config->node_budget = 800UL;
        config->time_limit_ms = 100UL;
        config->fart_bias = 8;
    } else if (difficulty == CF_CPU_HARD) {
        config->max_depth = 3;
        config->node_budget = 50000UL;
        config->time_limit_ms = 1500UL;
        config->fart_bias = 38;
    } else {
        config->difficulty = CF_CPU_MEDIUM;
        config->max_depth = 2;
        config->node_budget = 8000UL;
        config->time_limit_ms = 500UL;
        config->fart_bias = 28;
    }
}

void cpu_cycle_difficulty(CfCpuConfig *config)
{
    CfCpuDifficulty next;
    if (config == 0) return;
    next = (CfCpuDifficulty)(((int)config->difficulty + 1) % 3);
    cpu_config_for_difficulty(config, next);
}

const char *cpu_difficulty_name(CfCpuDifficulty difficulty)
{
    switch (difficulty) {
    case CF_CPU_EASY: return "EASY";
    case CF_CPU_HARD: return "HARD";
    default: return "MED";
    }
}
