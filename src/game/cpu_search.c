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

/*
 * Open Watcom's large model can pull this now-compact array back into
 * DGROUP because it falls below the compiler's large-data threshold.
 * Keep the search workspace explicitly far so shrinking it does not
 * consume the 64 KiB near-data budget.
 */
#ifdef __WATCOMC__
static CfCpuActionList __far g_lists[CPU_SEARCH_PLY];
#else
static CfCpuActionList g_lists[CPU_SEARCH_PLY];
#endif

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

static int shifted_bound(int bonus, int bound)
{
    long shifted;
    shifted = (long)bonus - (long)bound;
    if (shifted < -(long)CPU_INF) return -CPU_INF;
    if (shifted > (long)CPU_INF) return CPU_INF;
    return (int)shifted;
}

static int negamax(CfBoard *b, CfGasState *g, int depth,
                   int alpha, int beta, int ply, SearchContext *c)
{
    CfCpuActionList *list;
    CfCpuUndo undo;
    int best = -CPU_INF;
    int score;
    int action_bonus;
    int child_alpha;
    int child_beta;
    int actor_was_in_check;
    int opponent_king_located = 0;
    int opponent_king_known = 0;
    int opponent_king_file = -1;
    int opponent_king_rank = -1;
    int i;

    if (budget_expired(c)) {
        c->aborted = 1;
        c->stats->budget_hit = 1;
        return 0;
    }
    ++c->stats->nodes;
    if (b->halfmove_clock >= 100U || board_is_insufficient_material(b)) return 0;
    if (ply >= CPU_SEARCH_PLY) return cpu_internal_evaluate(b, g);

    if (depth <= 0) {
        actor_was_in_check = -1;
        if (!cpu_internal_has_legal_action(
                b, g, &actor_was_in_check)) {
            if (actor_was_in_check < 0)
                actor_was_in_check = board_is_in_check(
                    b, b->side_to_move);
            if (actor_was_in_check)
                return -CPU_MATE + ply;
            return 0;
        }
        return cpu_internal_evaluate(b, g);
    }

    list = &g_lists[ply];
    cpu_internal_generate_actions(b, g, list, &actor_was_in_check);
    if (list->count == 0) {
        if (actor_was_in_check < 0)
            actor_was_in_check = board_is_in_check(b, b->side_to_move);
        if (actor_was_in_check) return -CPU_MATE + ply;
        return 0;
    }
    cpu_internal_sort_actions(list,
                              ply + 1 < CPU_SEARCH_PLY ?
                              &g_lists[ply + 1] : 0);
    /*
     * If check state is still unknown, this node has no Fart actions.
     * Generation resolves it before adding any Fart action, and ordinary
     * chess actions never consume the value.
     */
    for (i = 0; i < list->count; ++i) {
        /*
         * Child negamax checks the full time+node budget immediately on
         * entry. Keep only the hard node-cap guard here so recursive action
         * loops do not call clock() twice per child.
         */
        if (c->config.node_budget != 0UL &&
            c->stats->nodes >= c->config.node_budget) {
            c->aborted = 1;
            c->stats->budget_hit = 1;
            break;
        }
        if (list->actions[i].type == CF_CPU_ACTION_FART &&
            !opponent_king_located) {
            opponent_king_known = cpu_internal_find_first_king(
                b, board_other_color(b->side_to_move),
                &opponent_king_file, &opponent_king_rank);
            opponent_king_located = 1;
        }
        if (!cpu_apply_action(b, g, &list->actions[i], &undo)) continue;
        if (list->actions[i].type == CF_CPU_ACTION_FART)
            action_bonus = cpu_internal_action_bonus_prelocated(
                b, g, &list->actions[i], &undo, &c->config,
                actor_was_in_check, opponent_king_known,
                opponent_king_file, opponent_king_rank);
        else
            action_bonus = 0;
        child_alpha = shifted_bound(action_bonus, beta);
        child_beta = shifted_bound(action_bonus, alpha);
        score = -negamax(b, g, depth-1, child_alpha, child_beta, ply+1, c) +
                action_bonus;
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
    int best_score = -CPU_INF;
    int iteration_score;
    int score;
    int action_bonus;
    int actor_was_in_check;
    int opponent_king_located = 0;
    int opponent_king_known = 0;
    int opponent_king_file = -1;
    int opponent_king_rank = -1;
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

    root = &g_lists[0];
    cpu_internal_generate_actions(b, g, root, &actor_was_in_check);
    if (root->count == 0) return 0;
    if (board_is_insufficient_material(b)) return 0;
    if (history != 0 &&
        gas_history_repetition_count(history, b, g) >= 3)
        return 0;
    if (b->halfmove_clock >= 100U) return 0;
    cpu_internal_sort_actions(root, &g_lists[1]);
    best_action = root->actions[0];

    c.config = *config;
    c.stats = stats;
    c.start_clock = clock();
    c.aborted = 0;
    /*
     * As in negamax, an unknown actor check state means there are no Fart
     * actions in this root list, so no action bonus can consume the value.
     */

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
            if (root->actions[i].type == CF_CPU_ACTION_FART &&
                !opponent_king_located) {
                opponent_king_known = cpu_internal_find_first_king(
                    b, board_other_color(b->side_to_move),
                    &opponent_king_file, &opponent_king_rank);
                opponent_king_located = 1;
            }
            if (!cpu_apply_action(b, g, &root->actions[i], &undo)) continue;
            if (root->actions[i].type == CF_CPU_ACTION_FART)
                action_bonus = cpu_internal_action_bonus_prelocated(
                    b, g, &root->actions[i], &undo, &c.config,
                    actor_was_in_check, opponent_king_known,
                    opponent_king_file, opponent_king_rank);
            else
                action_bonus = 0;
            score = -negamax(b, g, depth-1, -CPU_INF, CPU_INF, 1, &c) +
                    action_bonus;
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
