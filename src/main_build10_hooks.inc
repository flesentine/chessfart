static int cpu_terminal(CfGameStatus status)
{
    return status == CF_GAME_CHECKMATE || status == CF_GAME_STALEMATE ||
           status == CF_GAME_DRAW_FIFTY_MOVE ||
           status == CF_GAME_DRAW_THREEFOLD ||
           status == CF_GAME_DRAW_INSUFFICIENT;
}

static int cpu_execute_action(CfBoard *board, CfGasState *gas,
                              const CfCpuAction *action)
{
    CfGasMove move;
    CfFartAction fart;
    if (action->type == CF_CPU_ACTION_MOVE)
        return audio_game_make_move_ex(board, gas,
                                       action->from_file, action->from_rank,
                                       action->to_file, action->to_rank,
                                       action->promotion, &move);
    if (action->type == CF_CPU_ACTION_FART)
        return audio_game_make_fart(board, gas,
                                    action->from_file, action->from_rank,
                                    action->direction, action->promotion, &fart);
    return 0;
}

static int cpu_take_turn(CfBoard *board, CfGasState *gas,
                         CfGasHistory *history, int record_final)
{
    CfCpuAction action;
    CfGameStatus status;
    if (board == 0 || gas == 0 || history == 0) return 0;
    if (board->side_to_move != CF_COLOR_BLACK) return 0;

    g_cpu_search_history = *history;
    gas_history_record(&g_cpu_search_history, board, gas);
    status = gas_game_status(board, gas, &g_cpu_search_history);
    if (cpu_terminal(status)) return 0;
    if (!cpu_choose_action(board, gas, &g_cpu_search_history,
                           &g_cpu_config, &action, &g_cpu_stats)) return 0;

    g_cpu_history_backup = *history;
    gas_history_record(history, board, gas);
    if (!cpu_execute_action(board, gas, &action)) {
        *history = g_cpu_history_backup;
        return 0;
    }
    if (record_final) gas_history_record(history, board, gas);
    cpu_format_action(&action, g_cpu_message, sizeof(g_cpu_message));
    g_cpu_message_pending = 1;
    return 1;
}

int cpu_human_make_move(CfBoard *board, CfGasState *gas,
                        int ff, int fr, int tf, int tr, CfGasMove *made)
{
    CfGasMove local;
    if (!audio_game_make_move(board, gas, ff, fr, tf, tr, &local)) return 0;
    (void)cpu_take_turn(board, gas, &g_history, 0);
    if (made != 0) *made = local;
    return 1;
}

int cpu_human_make_move_ex(CfBoard *board, CfGasState *gas,
                           int ff, int fr, int tf, int tr,
                           CfPieceType promotion, CfGasMove *made)
{
    CfGasMove local;
    if (!audio_game_make_move_ex(board, gas, ff, fr, tf, tr,
                                 promotion, &local)) return 0;
    (void)cpu_take_turn(board, gas, &g_history, 0);
    if (made != 0) *made = local;
    return 1;
}

int cpu_human_make_fart(CfBoard *board, CfGasState *gas, int file, int rank,
                        CfFartDirection direction, CfPieceType promotion,
                        CfFartAction *action)
{
    CfFartAction local;
    if (!audio_game_make_fart(board, gas, file, rank,
                              direction, promotion, &local)) return 0;
    (void)cpu_take_turn(board, gas, &g_history, 0);
    if (action != 0) *action = local;
    return 1;
}

CfPersistenceResult cpu_load_game(const char *path, CfBoard *board,
                                  CfGasState *gas, CfGasHistory *history)
{
    CfPersistenceResult result;
    result = persistence_load_game(path, board, gas, history);
    if (result == CF_PERSIST_OK && board->side_to_move == CF_COLOR_BLACK)
        (void)cpu_take_turn(board, gas, history, 1);
    return result;
}
