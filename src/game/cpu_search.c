#include <string.h>
#include <time.h>
#include "cpu_internal.h"

#define CPU_INF 32000
#define CPU_MATE 30000
#define CPU_SEARCH_PLY 5

typedef struct SearchContext {
    CfCpuConfig config;
    CfCpuStats *stats;
    clock_t start_clock;
    int aborted;
} SearchContext;

static CfCpuActionList g_lists[CPU_SEARCH_PLY];

static int time_expired(SearchContext *c)
{
    clock_t now;
    unsigned long elapsed;
    if (c->config.time_limit_ms == 0UL) return 0;
    now = clock();
    if (now < c->start_clock) return 0;
    elapsed = (unsigned long)(now - c->start_clock) * 1000UL /
              (unsigned long)CLOCKS_PER_SEC;
    return elapsed >= c->config.time_limit_ms;
}

static int budget_expired(SearchContext *c)
{
    if (c->config.node_budget != 0UL &&
        c->stats->nodes >= c->config.node_budget) return 1;
    return time_expired(c);
}

static int negamax(CfBoard *b, CfGasState *g, int depth,
                   int alpha, int beta, int ply, SearchContext *c)
{
    CfCpuActionList *list;
    CfCpuUndo undo;
    int best = -CPU_INF;
    int score;
    int i;

    if (budget_expired(c)) {
        c->aborted = 1;
        c->stats->budget_hit = 1;
        return 0;
    }
    ++c->stats->nodes;
    if (b->halfmove_clock >= 100U || board_is_insufficient_material(b)) return 0;
    if (ply >= CPU_SEARCH_PLY) return cpu_internal_evaluate(b, g);

    list = &g_lists[ply];
    cpu_generate_actions(b, g, list);
    if (list->count == 0) {
        if (board_is_in_check(b, b->side_to_move)) return -CPU_MATE + ply;
        return 0;
    }
    if (depth <= 0) return cpu_internal_evaluate(b, g);
    cpu_internal_sort_actions(list);

    for (i = 0; i < list->count; ++i) {
        if (budget_expired(c)) {
            c->aborted = 1;
            c->stats->budget_hit = 1;
            break;
        }
        if (!cpu_apply_action(b, g, &list->actions[i], &undo)) continue;
        score = -negamax(b, g, depth-1, -beta, -alpha, ply+1, c);
        cpu_unapply_action(b, g, &undo);
        if (c->aborted) break;
        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) {
            ++c->stats->cutoffs;
            break;
        }
    }
    return best == -CPU_INF ? cpu_internal_evaluate(b, g) : best;
}

int cpu_choose_action(CfBoard *b, CfGasState *g,
                      const CfGasHistory *history,
                      const CfCpuConfig *config,
                      CfCpuAction *chosen, CfCpuStats *stats)
{
    CfCpuConfig local_config;
    CfCpuStats local_stats;
    SearchContext c;
    CfCpuActionList *root;
    CfCpuUndo undo;
    CfCpuAction best_action;
    CfCpuAction iteration_best;
    CfGameStatus status;
    int best_score = -CPU_INF;
    int iteration_score;
    int score;
    int depth;
    int i;

    if (b == 0 || g == 0 || chosen == 0) return 0;
    if (config == 0) {
        cpu_config_for_difficulty(&local_config, CF_CPU_MEDIUM);
        config = &local_config;
    }
    memset(&local_stats, 0, sizeof(local_stats));
    if (stats == 0) stats = &local_stats;
    else memset(stats, 0, sizeof(*stats));

    status = gas_game_status(b, g, history);
    if (status == CF_GAME_CHECKMATE || status == CF_GAME_STALEMATE ||
        status == CF_GAME_DRAW_FIFTY_MOVE || status == CF_GAME_DRAW_THREEFOLD ||
        status == CF_GAME_DRAW_INSUFFICIENT) return 0;

    root = &g_lists[0];
    cpu_generate_actions(b, g, root);
    if (root->count == 0) return 0;
    cpu_internal_sort_actions(root);
    best_action = root->actions[0];

    c.config = *config;
    c.stats = stats;
    c.start_clock = clock();
    c.aborted = 0;

    for (depth = 1; depth <= config->max_depth; ++depth) {
        int completed = 1;
        iteration_best = best_action;
        iteration_score = -CPU_INF;
        c.aborted = 0;
        for (i = 0; i < root->count; ++i) {
            if (budget_expired(&c)) {
                c.aborted = 1;
                stats->budget_hit = 1;
                completed = 0;
                break;
            }
            if (!cpu_apply_action(b, g, &root->actions[i], &undo)) continue;
            score = -negamax(b, g, depth-1, -CPU_INF, CPU_INF, 1, &c);
            cpu_unapply_action(b, g, &undo);
            if (c.aborted) {
                completed = 0;
                break;
            }
            if (score > iteration_score) {
                iteration_score = score;
                iteration_best = root->actions[i];
            }
        }
        if (!completed) break;
        best_action = iteration_best;
        best_score = iteration_score;
        stats->depth_completed = depth;
        stats->score = best_score;
    }

    *chosen = best_action;
    if (stats->depth_completed == 0) stats->score = best_score;
    return 1;
}
