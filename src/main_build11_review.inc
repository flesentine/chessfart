#include <emscripten/emscripten.h>
#include "ui_layout.h"

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

EMSCRIPTEN_KEEPALIVE int cf_review_ui_synced(void)
{
    if (!g_ux_cache.valid) return 0;
    if (g_ux_cache.fart_mode ||
        g_ux_cache.promotion_pending ||
        g_ux_cache.fart_promotion_pending) return 0;
    return g_ux_cache.board.side_to_move == g_board.side_to_move &&
           g_ux_cache.board.fullmove_number == g_board.fullmove_number;
}

EMSCRIPTEN_KEEPALIVE int cf_review_board_x(void)
{
    return CF_UI_BOARD_X;
}

EMSCRIPTEN_KEEPALIVE int cf_review_board_y(void)
{
    return CF_UI_BOARD_Y;
}

EMSCRIPTEN_KEEPALIVE int cf_review_square_size(void)
{
    return CF_UI_SQUARE_SIZE;
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

/*
 * Build 13 visual-review fixtures. These exist only in CF_WEB_REVIEW builds
 * so Chromium can capture rare but important UI states deterministically.
 * They never ship in the DOS/release gameplay path.
 *
 * fixture 0: charged H4 pawn, north Fart push preview H5 -> H6
 * fixture 1: White to move in CHECK
 * fixture 2: White to move in CHECKMATE
 * fixture 3: charged A8 knight aiming north off-board (INVALID)
 * fixture 4: Black to move in STALEMATE
 */
EMSCRIPTEN_KEEPALIVE int cf_review_render_fixture(int fixture)
{
    CfMoveList moves;
    CfGameStatus status;
    CfFartPreview preview;

    moves.count = 0;
    board_clear(&g_board);
    gas_init(&g_gas);

    if (fixture == 0) {
        board_set_piece(&g_board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 7, 3, CF_PIECE_PAWN, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 4, CF_PIECE_PAWN, CF_COLOR_BLACK);
        g_board.side_to_move = CF_COLOR_WHITE;
        gas_set(&g_gas, 7, 3, 2U);
        gas_history_init(&g_history, &g_board, &g_gas);
        ux_history_init(&g_ux_history);
        ux_history_add(&g_ux_history, "WHITE PAWN TO H4");
        status = gas_game_status(&g_board, &g_gas, &g_history);
        preview = gas_preview_fart(&g_board, &g_gas, 7, 3, CF_FART_N);
        if (status != CF_GAME_ONGOING || preview != CF_FART_PUSH) return 0;
        render(7, 3, 1, 7, 3, &moves, status,
               0, CF_PIECE_QUEEN, 1, CF_FART_N, preview,
               0, CF_PIECE_QUEEN, "PUSH H5-H6");
        return 1;
    }

    if (fixture == 1) {
        board_set_piece(&g_board, 4, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 4, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
        g_board.side_to_move = CF_COLOR_WHITE;
        gas_history_init(&g_history, &g_board, &g_gas);
        status = gas_game_status(&g_board, &g_gas, &g_history);
        if (status != CF_GAME_CHECK) return 0;
        render(4, 0, 0, 0, 0, &moves, status,
               0, CF_PIECE_QUEEN, 0, CF_FART_N, CF_FART_INVALID,
               0, CF_PIECE_QUEEN, "CHECK");
        return 1;
    }

    if (fixture == 2) {
        board_set_piece(&g_board, 7, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 5, 2, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 6, 1, CF_PIECE_QUEEN, CF_COLOR_BLACK);
        g_board.side_to_move = CF_COLOR_WHITE;
        gas_history_init(&g_history, &g_board, &g_gas);
        status = gas_game_status(&g_board, &g_gas, &g_history);
        if (status != CF_GAME_CHECKMATE) return 0;
        render(7, 0, 0, 0, 0, &moves, status,
               0, CF_PIECE_QUEEN, 0, CF_FART_N, CF_FART_INVALID,
               0, CF_PIECE_QUEEN, "CHECKMATE");
        return 1;
    }

    if (fixture == 3) {
        board_set_piece(&g_board, 7, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&g_board, 0, 7, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
        board_set_piece(&g_board, 1, 1, CF_PIECE_PAWN, CF_COLOR_WHITE);
        g_board.side_to_move = CF_COLOR_WHITE;
        gas_set(&g_gas, 0, 7, 2U);
        gas_history_init(&g_history, &g_board, &g_gas);
        ux_history_init(&g_ux_history);
        ux_history_add(&g_ux_history, "WHITE KNIGHT TO A8");
        status = gas_game_status(&g_board, &g_gas, &g_history);
        preview = gas_preview_fart(&g_board, &g_gas, 0, 7, CF_FART_N);
        if (status != CF_GAME_ONGOING || preview != CF_FART_INVALID) return 0;
        render(0, 7, 1, 0, 7, &moves, status,
               0, CF_PIECE_QUEEN, 1, CF_FART_N, preview,
               0, CF_PIECE_QUEEN, "INVALID");
        return 1;
    }

    if (fixture == 4) {
        board_set_piece(&g_board, 2, 5, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&g_board, 2, 6, CF_PIECE_QUEEN, CF_COLOR_WHITE);
        board_set_piece(&g_board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        g_board.side_to_move = CF_COLOR_BLACK;
        gas_history_init(&g_history, &g_board, &g_gas);
        ux_history_init(&g_ux_history);
        ux_history_add(&g_ux_history, "WHITE QUEEN TO C7");
        status = gas_game_status(&g_board, &g_gas, &g_history);
        if (status != CF_GAME_STALEMATE) return 0;
        render(0, 7, 0, 0, 0, &moves, status,
               0, CF_PIECE_QUEEN, 0, CF_FART_N, CF_FART_INVALID,
               0, CF_PIECE_QUEEN, "STALEMATE");
        return 1;
    }

    return 0;
}

EMSCRIPTEN_KEEPALIVE int cf_review_render_ui_fixture(int fixture)
{
    if (fixture == 0) {
        ux_draw_title(0, 0);
        ux_draw_help_page(1);
        vga_present();
        return 1;
    }
    if (fixture == 1) {
        ux_draw_title(0, 0);
        ux_draw_credits_screen();
        vga_present();
        return 1;
    }
    if (fixture == 2) {
        ux_draw_title(3, 1);
        vga_present();
        return 1;
    }
    return 0;
}
