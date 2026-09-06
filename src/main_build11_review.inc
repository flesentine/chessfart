#include <emscripten/emscripten.h>
#include "ui_layout.h"

static int g_review_practice_stress_actions;

static int cf_review_hash_bytes(const void *data, unsigned size)
{
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned long hash = 2166136261UL;
    unsigned i;
    for (i = 0U; i < size; ++i) {
        hash ^= (unsigned long)bytes[i];
        hash *= 16777619UL;
    }
    return (int)hash;
}

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

EMSCRIPTEN_KEEPALIVE int cf_review_castling_rights(void)
{
    return (int)g_board.castling_rights;
}

EMSCRIPTEN_KEEPALIVE int cf_review_en_passant_file(void)
{
    return g_board.en_passant_file;
}

EMSCRIPTEN_KEEPALIVE int cf_review_en_passant_rank(void)
{
    return g_board.en_passant_rank;
}

EMSCRIPTEN_KEEPALIVE int cf_review_halfmove(void)
{
    return (int)g_board.halfmove_clock;
}

EMSCRIPTEN_KEEPALIVE int cf_review_gas_history_count(void)
{
    return g_history.count;
}

EMSCRIPTEN_KEEPALIVE int cf_review_gas_history_state(int index)
{
    if (index < 0 || index >= g_history.count) return -1;
    return (int)g_history.keys[index].state;
}

EMSCRIPTEN_KEEPALIVE int cf_review_gas_history_ep(int index)
{
    if (index < 0 || index >= g_history.count) return -1;
    return (int)g_history.keys[index].en_passant_file;
}

EMSCRIPTEN_KEEPALIVE int cf_review_gas_history_square(int index, int square)
{
    if (index < 0 || index >= g_history.count ||
        square < 0 || square >= 64) return -1;
    return (int)g_history.keys[index].squares[square];
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

EMSCRIPTEN_KEEPALIVE void cf_review_set_match_mode(int mode)
{
    CfMatchMode requested = (CfMatchMode)mode;
    if (ux_match_mode_valid(requested)) g_match_mode = requested;
}

EMSCRIPTEN_KEEPALIVE int cf_review_match_mode(void)
{
    return (int)g_match_mode;
}

EMSCRIPTEN_KEEPALIVE int cf_review_ui_theme(void)
{
    return (int)ui_theme_get();
}

EMSCRIPTEN_KEEPALIVE int cf_review_set_ui_theme(int theme)
{
    if (!ui_theme_set((CfUiTheme)theme)) return 0;
    if (g_ux_cache.valid)
        ux_redraw_cached();
    else {
        ux_draw_title(CF_UI_TITLE_ITEM_PLAY_CPU, 0);
        vga_present();
    }
    return 1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_board_hash(void)
{
    return cf_review_hash_bytes(&g_board, sizeof(g_board));
}

EMSCRIPTEN_KEEPALIVE int cf_review_gas_hash(void)
{
    return cf_review_hash_bytes(&g_gas, sizeof(g_gas));
}

EMSCRIPTEN_KEEPALIVE int cf_review_practice_mode(void)
{
    return g_practice_mode;
}

EMSCRIPTEN_KEEPALIVE int cf_review_practice_undo_count(void)
{
    return g_practice_undo.count;
}

EMSCRIPTEN_KEEPALIVE int cf_review_practice_presentation_undo_count(void)
{
    return g_practice_presentation_undo.count;
}

EMSCRIPTEN_KEEPALIVE int cf_review_practice_undo_start(void)
{
    return g_practice_undo.start;
}

EMSCRIPTEN_KEEPALIVE int cf_review_practice_presentation_undo_start(void)
{
    return g_practice_presentation_undo.start;
}

EMSCRIPTEN_KEEPALIVE int cf_review_ux_history_start(void)
{
    return g_ux_history.start;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_start(void)
{
    return g_replay_timeline == 0 ? -1 : g_replay_timeline->start;
}

EMSCRIPTEN_KEEPALIVE int cf_review_gas_history_hash(void)
{
    return cf_review_hash_bytes(&g_history, sizeof(g_history));
}

EMSCRIPTEN_KEEPALIVE int cf_review_ux_history_hash(void)
{
    return cf_review_hash_bytes(&g_ux_history, sizeof(g_ux_history));
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_hash(void)
{
    if (g_replay_timeline == 0) return 0;
    return cf_review_hash_bytes(g_replay_timeline,
                                sizeof(*g_replay_timeline));
}

EMSCRIPTEN_KEEPALIVE int cf_review_prepare_practice_stress(void)
{
    CfGameStatus status;
    if (g_replay_timeline == 0) return 0;

    board_clear(&g_board);
    gas_init(&g_gas);
    board_set_piece(&g_board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&g_board, 3, 3, CF_PIECE_PAWN, CF_COLOR_WHITE);
    g_board.side_to_move = CF_COLOR_WHITE;
    g_board.castling_rights = 0U;
    g_board.en_passant_file = -1;
    g_board.en_passant_rank = -1;
    g_board.halfmove_clock = 0U;
    g_board.fullmove_number = 1U;

    g_match_mode = CF_MATCH_LOCAL;
    g_practice_mode = 1;
    practice_undo_init(&g_practice_undo);
    ux_practice_presentation_init();
    ux_history_init(&g_ux_history);
    gas_history_init(&g_history, &g_board, &g_gas);
    status = gas_game_status(&g_board, &g_gas, &g_history);
    replay_timeline_reset(g_replay_timeline, &g_board, &g_gas,
                          status, g_match_mode, "STRESS START");
    g_cpu_message[0] = '\0';
    g_cpu_message_pending = 0;
    g_review_practice_stress_actions = 0;
    return 1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_run_practice_stress_actions(int count)
{
    CfGasMove move;
    int i;
    int ok;
    if (!g_practice_mode || count < 0) return 0;

    for (i = 0; i < count; ++i) {
        switch (g_review_practice_stress_actions & 3) {
        case 0:
            ok = ux_human_make_move(&g_board, &g_gas,
                                     0, 0, 0, 1, &move);
            break;
        case 1:
            ok = ux_human_make_move(&g_board, &g_gas,
                                     7, 7, 7, 6, &move);
            break;
        case 2:
            ok = ux_human_make_move(&g_board, &g_gas,
                                     0, 1, 0, 0, &move);
            break;
        default:
            ok = ux_human_make_move(&g_board, &g_gas,
                                     7, 6, 7, 7, &move);
            break;
        }
        if (!ok) return 0;
        gas_history_record(&g_history, &g_board, &g_gas);
        ++g_review_practice_stress_actions;
    }
    return 1;
}

EMSCRIPTEN_KEEPALIVE int cf_review_practice_undo_direct(void)
{
    return ux_practice_undo_game(&g_board, &g_gas, &g_history);
}

EMSCRIPTEN_KEEPALIVE int cf_review_enable_practice_from_current(void)
{
    CfGameStatus status;
    if (g_match_mode != CF_MATCH_LOCAL || g_replay_timeline == 0)
        return 0;
    g_practice_mode = 1;
    practice_undo_init(&g_practice_undo);
    ux_practice_presentation_init();
    ux_history_init(&g_ux_history);
    status = gas_game_status(&g_board, &g_gas, &g_history);
    replay_timeline_reset(g_replay_timeline, &g_board, &g_gas,
                          status, g_match_mode, "PRACTICE FIXTURE");
    g_cpu_message[0] = '\0';
    g_cpu_message_pending = 0;
    return 1;
}

EMSCRIPTEN_KEEPALIVE void cf_review_set_practice_mode_for_fixture(int enabled)
{
    g_practice_mode = enabled ? 1 : 0;
    practice_undo_init(&g_practice_undo);
    ux_practice_presentation_init();
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_count(void)
{
    return g_replay_timeline == 0 ? 0 : g_replay_timeline->count;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_total(void)
{
    return g_replay_timeline == 0 ? 0 : (int)g_replay_timeline->total;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_truncated(void)
{
    return g_replay_timeline == 0 ? 0 : g_replay_timeline->truncated;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_viewer_active(void)
{
    return g_replay_viewer_active;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_viewer_index(void)
{
    return g_replay_viewer_index;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_side(int index)
{
    const CfReplaySnapshot *snapshot;
    snapshot = replay_timeline_get(g_replay_timeline, index);
    return snapshot == 0 ? -1 : (int)snapshot->side_to_move;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_fullmove(int index)
{
    const CfReplaySnapshot *snapshot;
    CfBoard board;
    CfGasState gas;
    snapshot = replay_timeline_get(g_replay_timeline, index);
    if (snapshot == 0) return -1;
    if (!replay_snapshot_restore(snapshot, &board, &gas, 0, 0)) return -1;
    return (int)board.fullmove_number;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_status(int index)
{
    const CfReplaySnapshot *snapshot;
    snapshot = replay_timeline_get(g_replay_timeline, index);
    return snapshot == 0 ? -1 : (int)snapshot->status;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_mode(int index)
{
    const CfReplaySnapshot *snapshot;
    snapshot = replay_timeline_get(g_replay_timeline, index);
    return snapshot == 0 ? -1 : (int)snapshot->match_mode;
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_piece(int index, int file, int rank)
{
    const CfReplaySnapshot *snapshot;
    CfBoard board;
    CfGasState gas;
    const CfPiece *piece;
    int square_gas;

    snapshot = replay_timeline_get(g_replay_timeline, index);
    if (snapshot == 0 || file < 0 || file >= 8 || rank < 0 || rank >= 8)
        return -1;
    if (!replay_snapshot_restore(snapshot, &board, &gas, 0, 0)) return -1;
    piece = board_piece_at(&board, file, rank);
    if (piece == 0 || piece->type == CF_PIECE_NONE) return 0;
    square_gas = (int)gas_at(&gas, file, rank);
    return ((int)piece->type & 15) |
           (((int)piece->color & 15) << 4) |
           ((square_gas & 15) << 8);
}

EMSCRIPTEN_KEEPALIVE int cf_review_replay_last_matches_current(void)
{
    const CfReplaySnapshot *snapshot;
    CfGameStatus status;
    if (g_replay_timeline == 0 || g_replay_timeline->count <= 0) return 0;
    snapshot = replay_timeline_get(g_replay_timeline,
                                   g_replay_timeline->count - 1);
    status = gas_game_status(&g_board, &g_gas, &g_history);
    return replay_snapshot_matches(snapshot, &g_board, &g_gas,
                                   status, g_match_mode);
}

EMSCRIPTEN_KEEPALIVE int cf_review_history_has_local_pair(void)
{
    const char *line;
    int saw_white = 0;
    int i;
    for (i = 0; i < g_ux_history.count; ++i) {
        line = ux_history_get(&g_ux_history, i);
        if (line == 0) continue;
        if (!saw_white && strncmp(line, "WHITE ", 6) == 0)
            saw_white = 1;
        else if (saw_white && strncmp(line, "BLACK ", 6) == 0)
            return 1;
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE int cf_review_fart_mode(void)
{
    return g_ux_cache.valid && g_ux_cache.fart_mode;
}

EMSCRIPTEN_KEEPALIVE int cf_review_promotion_pending(void)
{
    return g_ux_cache.valid && g_ux_cache.promotion_pending;
}

EMSCRIPTEN_KEEPALIVE int cf_review_promotion_choice(void)
{
    return g_ux_cache.valid ? (int)g_ux_cache.promotion_choice : 0;
}

EMSCRIPTEN_KEEPALIVE int cf_review_fart_promotion_pending(void)
{
    return g_ux_cache.valid && g_ux_cache.fart_promotion_pending;
}

EMSCRIPTEN_KEEPALIVE int cf_review_fart_promotion_choice(void)
{
    return g_ux_cache.valid ? (int)g_ux_cache.fart_promotion_choice : 0;
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

EMSCRIPTEN_KEEPALIVE int cf_review_title_menu_x(void)
{
    return CF_UI_TITLE_MENU_X;
}

EMSCRIPTEN_KEEPALIVE int cf_review_title_menu_width(void)
{
    return CF_UI_TITLE_MENU_W;
}

EMSCRIPTEN_KEEPALIVE int cf_review_title_menu_hit_y(void)
{
    return CF_UI_TITLE_MENU_HIT_Y;
}

EMSCRIPTEN_KEEPALIVE int cf_review_title_menu_item_step(void)
{
    return CF_UI_TITLE_MENU_ITEM_STEP;
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

EMSCRIPTEN_KEEPALIVE int cf_review_gas(int file, int rank)
{
    return (int)gas_at(&g_gas, file, rank);
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
 * Build 14.4 interactive local fixtures.
 *
 * These write a normal version-2 LOCAL save through the production
 * persistence layer. Chromium then presses L and performs every move/Fart
 * through the real input loop. The fixture writer itself never executes an
 * action.
 *
 * 0: Black Fart push from D5 south, pushing White pawn D4 -> D3
 * 1: White pawn B7 ready for ordinary promotion on B8
 * 2: Black pawn B2 ready for ordinary promotion on B1
 * 3: White Fart-push promotion: charged D6 actor, White pawn D7 -> D8
 * 4: Black Fart-push promotion: charged D3 actor, Black pawn D2 -> D1
 * 5: White mate in one: QG6-G7#
 * 6: Black mate in one: QG3-G2#
 * 7: Practice draw fixture: kings shuttle A1/A2 and H8/H7
 */
EMSCRIPTEN_KEEPALIVE int cf_review_write_local_fixture(int fixture)
{
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    CfGameStatus status;

    board_clear(&board);
    gas_init(&gas);

    if (fixture == 0) {
        board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 3, 4, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
        board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_WHITE);
        board.side_to_move = CF_COLOR_BLACK;
        gas_set(&gas, 3, 4, 3U);
        gas_set(&gas, 3, 3, 1U);
    } else if (fixture == 1) {
        board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 1, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
        board.side_to_move = CF_COLOR_WHITE;
    } else if (fixture == 2) {
        board_set_piece(&board, 7, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 1, 1, CF_PIECE_PAWN, CF_COLOR_BLACK);
        board.side_to_move = CF_COLOR_BLACK;
    } else if (fixture == 3) {
        board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 3, 5, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
        board_set_piece(&board, 3, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
        board.side_to_move = CF_COLOR_WHITE;
        gas_set(&gas, 3, 5, 3U);
        gas_set(&gas, 3, 6, 2U);
    } else if (fixture == 4) {
        board_set_piece(&board, 7, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 0, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 3, 2, CF_PIECE_KNIGHT, CF_COLOR_BLACK);
        board_set_piece(&board, 3, 1, CF_PIECE_PAWN, CF_COLOR_BLACK);
        board.side_to_move = CF_COLOR_BLACK;
        gas_set(&gas, 3, 2, 3U);
        gas_set(&gas, 3, 1, 2U);
    } else if (fixture == 5) {
        board_set_piece(&board, 5, 5, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 6, 5, CF_PIECE_QUEEN, CF_COLOR_WHITE);
        board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board.side_to_move = CF_COLOR_WHITE;
    } else if (fixture == 6) {
        board_set_piece(&board, 7, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 5, 2, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 6, 2, CF_PIECE_QUEEN, CF_COLOR_BLACK);
        board.side_to_move = CF_COLOR_BLACK;
    } else if (fixture == 7) {
        board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
        board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
        board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_WHITE);
        board.side_to_move = CF_COLOR_WHITE;
        board.castling_rights = 0U;
    } else {
        return 0;
    }

    gas_history_init(&history, &board, &gas);
    status = gas_game_status(&board, &gas, &history);
    if (status != CF_GAME_ONGOING) return 0;

    return persistence_save_game_mode(BUILD9_SAVE_PATH, &board, &gas,
                                      &history, CF_MATCH_LOCAL) ==
           CF_PERSIST_OK;
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
        ux_draw_title(CF_UI_TITLE_ITEM_CREDITS, 1);
        vga_present();
        return 1;
    }
    if (fixture == 3) {
        ux_draw_title(CF_UI_TITLE_ITEM_PLAY_LOCAL, 1);
        vga_present();
        return 1;
    }
    return 0;
}
