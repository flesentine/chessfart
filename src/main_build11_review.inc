#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE int cf_review_status(void)
{
    return (int)gas_game_status(&g_board, &g_gas, &g_history);
}

EMSCRIPTEN_KEEPALIVE int cf_review_side(void)
{
    return (int)g_board.side_to_move;
}

EMSCRIPTEN_KEEPALIVE int cf_review_fullmove(void)
{
    return (int)g_board.fullmove_number;
}

EMSCRIPTEN_KEEPALIVE void cf_review_set_cpu_difficulty(int difficulty)
{
    if (difficulty < (int)CF_CPU_EASY) difficulty = (int)CF_CPU_EASY;
    if (difficulty > (int)CF_CPU_HARD) difficulty = (int)CF_CPU_HARD;
    cpu_config_for_difficulty(&g_cpu_config, (CfCpuDifficulty)difficulty);
}

EMSCRIPTEN_KEEPALIVE int cf_review_cpu_difficulty(void)
{
    return (int)g_cpu_config.difficulty;
}

EMSCRIPTEN_KEEPALIVE int cf_review_piece(int file, int rank)
{
    const CfPiece *piece;
    int gas;
    piece = board_piece_at(&g_board, file, rank);
    if (piece == 0 || piece->type == CF_PIECE_NONE) return 0;
    gas = (int)gas_at(&g_gas, file, rank);
    return ((int)piece->type & 15) |
           (((int)piece->color & 15) << 4) |
           ((gas & 15) << 8);
}

EMSCRIPTEN_KEEPALIVE int cf_review_move_count(int file, int rank)
{
    CfMoveList list;
    board_generate_legal_moves(&g_board, file, rank, &list);
    return list.count;
}

EMSCRIPTEN_KEEPALIVE int cf_review_move_code(int file, int rank, int index)
{
    CfMoveList list;
    const CfMove *move;
    board_generate_legal_moves(&g_board, file, rank, &list);
    if (index < 0 || index >= list.count) return -1;
    move = &list.moves[index];
    return (move->to_file & 7) |
           ((move->to_rank & 7) << 3) |
           (((int)move->captured.type & 15) << 6) |
           (((int)move->flags & 255) << 10) |
           (((int)move->promotion & 15) << 18);
}

EMSCRIPTEN_KEEPALIVE int cf_review_fart_preview(int file, int rank, int direction)
{
    return (int)gas_preview_fart(&g_board, &g_gas,
                                 file, rank, (CfFartDirection)direction);
}

EMSCRIPTEN_KEEPALIVE int cf_review_history_count(void)
{
    return g_ux_history.count;
}

EMSCRIPTEN_KEEPALIVE const char *cf_review_history_line(int index)
{
    const char *line;
    line = ux_history_get(&g_ux_history, index);
    return line != 0 ? line : "";
}
