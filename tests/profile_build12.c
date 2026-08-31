#include <stdio.h>
#include <time.h>

#include "cpu.h"
#include "version.h"

static void run_profile(const char *name, CfCpuDifficulty difficulty)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    clock_t start;
    clock_t end;
    unsigned long elapsed_ms;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);
    cpu_config_for_difficulty(&config, difficulty);
    config.time_limit_ms = 0UL;
    start = clock();
    if (!cpu_choose_action(&board, &gas, &history, &config, &action, &stats)) {
        printf("%s ERROR no-action\n", name);
        return;
    }
    end = clock();
    elapsed_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);
    printf("%s depth=%d nodes=%lu cutoffs=%lu score=%d budget=%d elapsed_ms=%lu\n",
           name, stats.depth_completed, stats.nodes, stats.cutoffs,
           stats.score, stats.budget_hit, elapsed_ms);
}

static void run_fart_profile(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfCpuConfig config;
    CfCpuAction action;
    CfCpuStats stats;
    clock_t start;
    clock_t end;
    unsigned long elapsed_ms;
    int file;

    board_init_starting_position(&board);
    gas_init(&gas);
    for (file = 0; file < 8; ++file) {
        gas_set(&gas, file, 1, 3U);
        gas_set(&gas, file, 6, 3U);
    }
    board.side_to_move = CF_COLOR_BLACK;
    gas_history_init(&history, &board, &gas);
    cpu_config_for_difficulty(&config, CF_CPU_MEDIUM);
    config.time_limit_ms = 0UL;
    start = clock();
    if (!cpu_choose_action(&board, &gas, &history, &config, &action, &stats)) {
        printf("FART_HEAVY ERROR no-action\n");
        return;
    }
    end = clock();
    elapsed_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);
    printf("FART_HEAVY depth=%d nodes=%lu cutoffs=%lu score=%d budget=%d elapsed_ms=%lu type=%d\n",
           stats.depth_completed, stats.nodes, stats.cutoffs, stats.score,
           stats.budget_hit, elapsed_ms, (int)action.type);
}

int main(void)
{
    printf("Chess Fart %s host profile\n", CF_VERSION_STRING);
    printf("sizeof_board=%lu sizeof_gas=%lu sizeof_history=%lu sizeof_actions=%lu\n",
           (unsigned long)sizeof(CfBoard), (unsigned long)sizeof(CfGasState),
           (unsigned long)sizeof(CfGasHistory), (unsigned long)sizeof(CfCpuActionList));
    run_profile("EASY_START", CF_CPU_EASY);
    run_profile("MED_START", CF_CPU_MEDIUM);
    run_profile("HARD_START", CF_CPU_HARD);
    run_fart_profile();
    return 0;
}
