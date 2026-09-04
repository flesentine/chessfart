static int ux_terminal(CfGameStatus status)
{
    return status == CF_GAME_CHECKMATE || status == CF_GAME_STALEMATE ||
           status == CF_GAME_DRAW_FIFTY_MOVE ||
           status == CF_GAME_DRAW_THREEFOLD ||
           status == CF_GAME_DRAW_INSUFFICIENT;
}

static void ux_log_move(const char *actor, const CfGasMove *move)
{
    char line[CF_UX_HISTORY_LINE];
    ux_format_move_line(actor, move, line, sizeof(line));
    ux_history_add(&g_ux_history, line);
}

static void ux_log_fart(const char *actor, const CfFartAction *fart)
{
    char line[CF_UX_HISTORY_LINE];
    ux_format_fart_line(actor, fart, line, sizeof(line));
    ux_history_add(&g_ux_history, line);
}

static int ux_execute_cpu_action(CfBoard *board, CfGasState *gas,
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

static int ux_cpu_take_turn(CfBoard *board, CfGasState *gas,
                            CfGasHistory *history, int record_final)
{
    CfCpuAction action;
    CfGameStatus status;
    char line[CF_UX_HISTORY_LINE];
    if (board == 0 || gas == 0 || history == 0) return 0;
    if (!ux_match_mode_uses_cpu(g_match_mode)) return 0;
    if (board->side_to_move != CF_COLOR_BLACK) return 0;
    g_cpu_search_history = *history;
    gas_history_record(&g_cpu_search_history, board, gas);
    status = gas_game_status(board, gas, &g_cpu_search_history);
    if (ux_terminal(status)) return 0;
    if (!cpu_choose_action(board, gas, &g_cpu_search_history,
                           &g_cpu_config, &action, &g_cpu_stats)) return 0;
    g_cpu_history_backup = *history;
    gas_history_record(history, board, gas);
    if (!ux_execute_cpu_action(board, gas, &action)) {
        *history = g_cpu_history_backup;
        return 0;
    }
    if (record_final) gas_history_record(history, board, gas);
    cpu_format_action(&action, g_cpu_message, sizeof(g_cpu_message));
    g_cpu_message_pending = 1;
    ux_format_cpu_line("CPU", &action, line, sizeof(line));
    ux_history_add(&g_ux_history, line);
    return 1;
}

int ux_human_make_move(CfBoard *board, CfGasState *gas,
                       int ff, int fr, int tf, int tr, CfGasMove *made)
{
    CfGasMove local;
    if (!audio_game_make_move(board, gas, ff, fr, tf, tr, &local)) return 0;
    ux_log_move(ux_match_actor_name(g_match_mode,
                                    local.chess_move.moved.color), &local);
    (void)ux_cpu_take_turn(board, gas, &g_history, 0);
    if (made != 0) *made = local;
    return 1;
}

int ux_human_make_move_ex(CfBoard *board, CfGasState *gas,
                          int ff, int fr, int tf, int tr,
                          CfPieceType promotion, CfGasMove *made)
{
    CfGasMove local;
    if (!audio_game_make_move_ex(board, gas, ff, fr, tf, tr,
                                 promotion, &local)) return 0;
    ux_log_move(ux_match_actor_name(g_match_mode,
                                    local.chess_move.moved.color), &local);
    (void)ux_cpu_take_turn(board, gas, &g_history, 0);
    if (made != 0) *made = local;
    return 1;
}

int ux_human_make_fart(CfBoard *board, CfGasState *gas, int file, int rank,
                       CfFartDirection direction, CfPieceType promotion,
                       CfFartAction *action)
{
    CfFartAction local;
    if (!audio_game_make_fart(board, gas, file, rank,
                              direction, promotion, &local)) return 0;
    ux_log_fart(ux_match_actor_name(g_match_mode, local.previous_side), &local);
    (void)ux_cpu_take_turn(board, gas, &g_history, 0);
    if (action != 0) *action = local;
    return 1;
}

CfPersistenceResult ux_save_game(const char *path, const CfBoard *board,
                                 const CfGasState *gas,
                                 const CfGasHistory *history)
{
    CfPersistenceResult result;
    result = persistence_save_game(path, board, gas, history);
    if (result == CF_PERSIST_OK) ux_history_add(&g_ux_history, "SYSTEM SAVE OK");
    return result;
}

CfPersistenceResult ux_load_game(const char *path, CfBoard *board,
                                 CfGasState *gas, CfGasHistory *history)
{
    CfPersistenceResult result;
    result = persistence_load_game(path, board, gas, history);
    if (result == CF_PERSIST_OK) {
        ux_history_init(&g_ux_history);
        ux_history_add(&g_ux_history, "SYSTEM LOAD OK");
        if (board->side_to_move == CF_COLOR_BLACK)
            (void)ux_cpu_take_turn(board, gas, history, 1);
    }
    return result;
}

static CfInputKey5 ux_mouse_direction(int dx, int dy)
{
    if (dx > 0 && dy > 0) return CF5_KEY_UP_RIGHT;
    if (dx > 0 && dy < 0) return CF5_KEY_DOWN_RIGHT;
    if (dx < 0 && dy > 0) return CF5_KEY_UP_LEFT;
    if (dx < 0 && dy < 0) return CF5_KEY_DOWN_LEFT;
    if (dx > 0) return CF5_KEY_RIGHT;
    if (dx < 0) return CF5_KEY_LEFT;
    if (dy > 0) return CF5_KEY_UP;
    if (dy < 0) return CF5_KEY_DOWN;
    return CF5_KEY_NONE;
}

static CfInputKey5 ux_begin_mouse_target(int file, int rank, int fart_after_select)
{
    int dx;
    int dy;
    if (!g_ux_cache.valid) return CF5_KEY_NONE;
    g_ux_mouse_target_file = file;
    g_ux_mouse_target_rank = rank;
    g_ux_mouse_target_active = 1;
    g_ux_mouse_fart_pending = fart_after_select ? 1 : 0;
    dx = file - g_ux_cache.cursor_file;
    dy = rank - g_ux_cache.cursor_rank;
    if (dx != 0) return dx > 0 ? CF5_KEY_RIGHT : CF5_KEY_LEFT;
    if (dy != 0) return dy > 0 ? CF5_KEY_UP : CF5_KEY_DOWN;
    g_ux_mouse_target_active = 0;
    if (g_ux_mouse_fart_pending == 1) g_ux_mouse_fart_pending = 2;
    return CF5_KEY_ENTER;
}

CfInputKey5 ux_poll_key(void)
{
    CfInputKey5 key;
    CfMouseState mouse;
    unsigned pressed;
    int file;
    int rank;
    int dx;
    int dy;
    if (g_ux_mouse_confirm_pending) {
        g_ux_mouse_confirm_pending = 0;
        return CF5_KEY_ENTER;
    }
    if (g_ux_mouse_fart_pending == 2) {
        g_ux_mouse_fart_pending = 0;
        return CF5_KEY_FART;
    }
    if (g_ux_mouse_target_active && g_ux_cache.valid) {
        dx = g_ux_mouse_target_file - g_ux_cache.cursor_file;
        dy = g_ux_mouse_target_rank - g_ux_cache.cursor_rank;
        if (dx != 0) return dx > 0 ? CF5_KEY_RIGHT : CF5_KEY_LEFT;
        if (dy != 0) return dy > 0 ? CF5_KEY_UP : CF5_KEY_DOWN;
        g_ux_mouse_target_active = 0;
        if (g_ux_mouse_fart_pending == 1) g_ux_mouse_fart_pending = 2;
        return CF5_KEY_ENTER;
    }
    if (mouse_poll(&mouse) && mouse.present) {
        pressed = mouse.buttons & ~g_ux_mouse_buttons;
        g_ux_mouse_buttons = mouse.buttons;
        if ((pressed & 2U) != 0U &&
            ux_board_hit_test(mouse.x, mouse.y, &file, &rank) &&
            g_ux_cache.valid && !g_ux_cache.fart_mode) {
            return ux_begin_mouse_target(file, rank, 1);
        }
        if ((pressed & 1U) != 0U &&
            ux_board_hit_test(mouse.x, mouse.y, &file, &rank) &&
            g_ux_cache.valid) {
            if (g_ux_cache.fart_mode) {
                dx = file - g_ux_cache.selected_file;
                dy = rank - g_ux_cache.selected_rank;
                if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1 &&
                    !(dx == 0 && dy == 0)) {
                    key = ux_mouse_direction(dx, dy);
                    if (key != CF5_KEY_NONE) {
                        g_ux_mouse_confirm_pending = 1;
                        return key;
                    }
                }
            } else {
                return ux_begin_mouse_target(file, rank, 0);
            }
        }
    }
    key = input5_poll_key();
    if (key == CF5_KEY_HELP) {
        ux_show_help_modal();
        return CF5_KEY_NONE;
    }
    if (key == CF5_KEY_HISTORY) {
        ux_show_history_modal();
        return CF5_KEY_NONE;
    }
    if (key == CF5_KEY_CREDITS) {
        ux_show_credits_modal();
        return CF5_KEY_NONE;
    }
    return key;
}
