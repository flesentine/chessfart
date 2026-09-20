#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cpu_internal.h"
#include "practice_undo.h"
#include "replay.h"
#include "version.h"

static volatile int sort_profile_sink;

static void profile_reference_insertion(CfCpuActionList *list)
{
    CfCpuAction item;
    int i;
    int j;
    for (i = 1; i < list->count; ++i) {
        item = list->actions[i];
        j = i - 1;
        while (j >= 0 && list->actions[j].order_score < item.order_score) {
            list->actions[j + 1] = list->actions[j];
            --j;
        }
        list->actions[j + 1] = item;
    }
}

static void run_sort_profile(void)
{
    static CfCpuActionList source;
    static CfCpuActionList work;
    static CfCpuActionList scratch;
    clock_t start;
    clock_t end;
    unsigned long merge_ms;
    unsigned long insertion_ms;
    unsigned long rng = 0xC001D00DUL;
    int repeat;
    int i;

    source.count = 256;
    for (i = 0; i < source.count; ++i) {
        source.actions[i].from_file = (cf_i8)(i & 7);
        source.actions[i].from_rank = (cf_i8)((i >> 3) & 7);
        source.actions[i].to_file = (cf_i8)((i * 3) & 7);
        source.actions[i].to_rank = (cf_i8)((i * 5) & 7);
        source.actions[i].type = (cf_u8)(1 + (i & 1));
        source.actions[i].promotion = (cf_u8)(i % 7);
        source.actions[i].direction = (cf_u8)(i & 7);
        source.actions[i].fart_result = (cf_u8)(i % 5);
        rng = rng * 1664525UL + 1013904223UL;
        source.actions[i].order_score = (cf_i16)((int)(rng % 101UL) - 50);
    }

    start = clock();
    for (repeat = 0; repeat < 200; ++repeat) {
        memcpy(&work, &source, sizeof(source));
        cpu_internal_sort_actions(&work, &scratch);
        sort_profile_sink += work.actions[0].order_score;
    }
    end = clock();
    merge_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 200; ++repeat) {
        memcpy(&work, &source, sizeof(source));
        profile_reference_insertion(&work);
        sort_profile_sink += work.actions[0].order_score;
    }
    end = clock();
    insertion_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("SORT256 merge_ms=%lu insertion_ms=%lu sink=%d\n",
           merge_ms, insertion_ms, sort_profile_sink);
}

static volatile int fart_scan_profile_sink;

static void run_fart_scan_profile(void)
{
    static const CfPieceType promotions[4] = {
        CF_PIECE_QUEEN, CF_PIECE_ROOK, CF_PIECE_BISHOP, CF_PIECE_KNIGHT
    };
    CfBoard board;
    CfGasState gas;
    CfFartScan scan;
    CfFartPreview preview;
    cf_u8 mask;
    clock_t start;
    clock_t end;
    unsigned long prechecked_ms;
    unsigned long batch_ms;
    unsigned long reference_ms;
    int actor_in_check;
    int repeat;
    int file;
    int d;
    int p;

    board_init_starting_position(&board);
    gas_init(&gas);
    board.side_to_move = CF_COLOR_BLACK;
    for (file = 0; file < 8; ++file)
        gas_set(&gas, file, 6, 3U);

    start = clock();
    for (repeat = 0; repeat < 100000; ++repeat) {
        actor_in_check = board_is_in_check(&board, board.side_to_move);
        for (file = 0; file < 8; ++file) {
            gas_scan_farts_prechecked(&board, &gas, file, 6,
                                      actor_in_check, &scan);
            for (d = 0; d < 8; ++d)
                fart_scan_profile_sink +=
                    (int)scan.preview[d] + (int)scan.promotion_mask[d];
        }
    }
    end = clock();
    prechecked_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 100000; ++repeat) {
        for (file = 0; file < 8; ++file) {
            gas_scan_farts(&board, &gas, file, 6, &scan);
            for (d = 0; d < 8; ++d)
                fart_scan_profile_sink +=
                    (int)scan.preview[d] + (int)scan.promotion_mask[d];
        }
    }
    end = clock();
    batch_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 100000; ++repeat) {
        for (file = 0; file < 8; ++file) {
            for (d = 0; d < 8; ++d) {
                preview = gas_preview_fart(&board, &gas, file, 6,
                                           (CfFartDirection)d);
                mask = 0U;
                if (preview == CF_FART_PROMOTION) {
                    for (p = 0; p < 4; ++p)
                        if (gas_fart_promotion_choice_legal(
                                &board, &gas, file, 6,
                                (CfFartDirection)d, promotions[p]))
                            mask = (cf_u8)(mask | (cf_u8)(1U << p));
                }
                fart_scan_profile_sink += (int)preview + (int)mask;
            }
        }
    }
    end = clock();
    reference_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("FART_SCAN prechecked_ms=%lu batch_ms=%lu reference_ms=%lu sink=%d\n",
           prechecked_ms, batch_ms, reference_ms, fart_scan_profile_sink);
}

static volatile int fart_apply_profile_sink;

static void run_fart_apply_profile(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    CfFartPreview preview;
    clock_t start;
    clock_t end;
    unsigned long fast_ms;
    unsigned long public_ms;
    int repeat;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 2, 2, 3U);
    gas_set(&gas, 3, 3, 1U);
    board.side_to_move = CF_COLOR_WHITE;
    preview = gas_preview_fart(&board, &gas, 2, 2, CF_FART_NE);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat) {
        if (!gas_make_fart_prevalidated(&board, &gas, 2, 2, CF_FART_NE,
                                        preview, CF_PIECE_NONE, &action))
            break;
        fart_apply_profile_sink += board.side_to_move;
        gas_unmake_fart(&board, &gas, &action);
    }
    end = clock();
    fast_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat) {
        if (!gas_make_fart(&board, &gas, 2, 2, CF_FART_NE,
                           CF_PIECE_NONE, &action))
            break;
        fart_apply_profile_sink += board.side_to_move;
        gas_unmake_fart(&board, &gas, &action);
    }
    end = clock();
    public_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("FART_APPLY prevalidated_ms=%lu public_ms=%lu sink=%d\n",
           fast_ms, public_ms, fart_apply_profile_sink);
}

typedef void (*ProfileMoveClearFn)(CfMove *);

static void profile_full_move_clear(CfMove *move)
{
    memset(move, 0, sizeof(*move));
}

static void profile_direct_move_clear(CfMove *move)
{
    move->prev_side_to_move = CF_COLOR_NONE;
    move->prev_castling_rights = 0U;
    move->prev_en_passant_file = 0;
    move->prev_en_passant_rank = 0;
    move->prev_halfmove_clock = 0U;
    move->prev_fullmove_number = 0U;
}

static ProfileMoveClearFn volatile profile_move_clear_fn;
static volatile unsigned long move_clear_profile_sink;

static void run_move_clear_profile(void)
{
    CfMove move;
    clock_t start;
    clock_t end;
    unsigned long full_ms;
    unsigned long direct_ms;
    int repeat;

    profile_move_clear_fn = profile_full_move_clear;
    start = clock();
    for (repeat = 0; repeat < 20000000; ++repeat) {
        move.prev_fullmove_number = 1U;
        profile_move_clear_fn(&move);
        move_clear_profile_sink += move.prev_fullmove_number;
    }
    end = clock();
    full_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    profile_move_clear_fn = profile_direct_move_clear;
    start = clock();
    for (repeat = 0; repeat < 20000000; ++repeat) {
        move.prev_fullmove_number = 1U;
        profile_move_clear_fn(&move);
        move_clear_profile_sink += move.prev_fullmove_number;
    }
    end = clock();
    direct_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("MOVE_CLEAR full_ms=%lu direct_ms=%lu full_bytes=%lu tail_fields=6 sink=%lu\n",
           full_ms, direct_ms, (unsigned long)sizeof(move),
           move_clear_profile_sink);
}

static volatile int move_apply_profile_sink;

static void run_move_apply_profile(void)
{
    CfBoard board;
    CfGasState gas;
    CfGasMove move;
    clock_t start;
    clock_t end;
    unsigned long fast_ms;
    unsigned long public_ms;
    int repeat;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_set(&gas, 1, 0, 2U);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat) {
        if (!gas_make_move_prevalidated(&board, &gas,
                                        1, 0, 2, 2,
                                        CF_PIECE_NONE, &move))
            break;
        move_apply_profile_sink += board.side_to_move;
        gas_unmake_move(&board, &gas, &move);
    }
    end = clock();
    fast_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat) {
        if (!gas_make_move_ex(&board, &gas,
                              1, 0, 2, 2,
                              CF_PIECE_NONE, &move))
            break;
        move_apply_profile_sink += board.side_to_move;
        gas_unmake_move(&board, &gas, &move);
    }
    end = clock();
    public_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("MOVE_APPLY prevalidated_ms=%lu public_ms=%lu sink=%d\n",
           fast_ms, public_ms, move_apply_profile_sink);
}

static volatile unsigned long undo_write_profile_sink;

static int profile_legacy_gas_move_prevalidated(
    CfBoard *board, CfGasState *gas,
    int from_file, int from_rank, int to_file, int to_rank,
    CfPieceType promotion, CfGasMove *made_move)
{
    CfGasMove local;
    if (!gas_make_move_prevalidated(board, gas,
                                    from_file, from_rank,
                                    to_file, to_rank,
                                    promotion, &local))
        return 0;
    if (made_move != 0) *made_move = local;
    return 1;
}

static int profile_legacy_gas_fart_prevalidated(
    CfBoard *board, CfGasState *gas,
    int file, int rank, CfFartDirection direction,
    CfFartPreview preview, CfPieceType promotion,
    CfFartAction *action)
{
    CfFartAction local;
    if (!gas_make_fart_prevalidated(board, gas, file, rank,
                                    direction, preview,
                                    promotion, &local))
        return 0;
    if (action != 0) *action = local;
    return 1;
}

static int profile_legacy_cpu_apply_action(
    CfBoard *board, CfGasState *gas,
    const CfCpuAction *action, CfCpuUndo *undo)
{
    CfCpuUndo local;
    int ok;

    if (action->type == CF_CPU_ACTION_MOVE)
        ok = profile_legacy_gas_move_prevalidated(
            board, gas, action->from_file, action->from_rank,
            action->to_file, action->to_rank,
            (CfPieceType)action->promotion, &local.action.move);
    else if (action->type == CF_CPU_ACTION_FART)
        ok = profile_legacy_gas_fart_prevalidated(
            board, gas, action->from_file, action->from_rank,
            (CfFartDirection)action->direction,
            (CfFartPreview)action->fart_result,
            (CfPieceType)action->promotion, &local.action.fart);
    else
        return 0;

    if (!ok) return 0;
    local.type = action->type;
    if (undo != 0) *undo = local;
    return 1;
}

static void run_undo_write_profile(void)
{
    CfBoard board;
    CfGasState gas;
    CfCpuAction move_action;
    CfCpuAction fart_action;
    CfCpuUndo undo;
    clock_t start;
    clock_t end;
    unsigned long move_direct_ms;
    unsigned long move_legacy_ms;
    unsigned long fart_direct_ms;
    unsigned long fart_legacy_ms;
    int repeat;

    board_init_starting_position(&board);
    gas_init(&gas);
    memset(&move_action, 0, sizeof(move_action));
    move_action.type = CF_CPU_ACTION_MOVE;
    move_action.from_file = 1;
    move_action.from_rank = 0;
    move_action.to_file = 2;
    move_action.to_rank = 2;

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat) {
        if (!cpu_apply_action(&board, &gas, &move_action, &undo)) break;
        undo_write_profile_sink +=
            (unsigned long)undo.action.move.chess_move.to_file;
        cpu_unapply_action(&board, &gas, &undo);
    }
    end = clock();
    move_direct_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat) {
        if (!profile_legacy_cpu_apply_action(
                &board, &gas, &move_action, &undo)) break;
        undo_write_profile_sink +=
            (unsigned long)undo.action.move.chess_move.to_file;
        cpu_unapply_action(&board, &gas, &undo);
    }
    end = clock();
    move_legacy_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 2, 2, 3U);
    board.side_to_move = CF_COLOR_WHITE;
    memset(&fart_action, 0, sizeof(fart_action));
    fart_action.type = CF_CPU_ACTION_FART;
    fart_action.from_file = 2;
    fart_action.from_rank = 2;
    fart_action.direction = CF_FART_NE;
    fart_action.fart_result = CF_FART_PUSH;

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat) {
        if (!cpu_apply_action(&board, &gas, &fart_action, &undo)) break;
        undo_write_profile_sink +=
            (unsigned long)undo.action.fart.destination_file;
        cpu_unapply_action(&board, &gas, &undo);
    }
    end = clock();
    fart_direct_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat) {
        if (!profile_legacy_cpu_apply_action(
                &board, &gas, &fart_action, &undo)) break;
        undo_write_profile_sink +=
            (unsigned long)undo.action.fart.destination_file;
        cpu_unapply_action(&board, &gas, &undo);
    }
    end = clock();
    fart_legacy_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("UNDO_WRITE move_direct_ms=%lu move_legacy_ms=%lu "
           "fart_direct_ms=%lu fart_legacy_ms=%lu bytes=%lu sink=%lu\n",
           move_direct_ms, move_legacy_ms,
           fart_direct_ms, fart_legacy_ms,
           (unsigned long)sizeof(CfCpuUndo),
           undo_write_profile_sink);
}

static volatile int legal_probe_profile_sink;

static int profile_reference_has_legal_action(const CfBoard *board,
                                              const CfGasState *gas)
{
    CfMoveList moves;
    CfFartScan scan;
    int file;
    int rank;
    int d;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].type == CF_PIECE_NONE ||
                board->squares[rank][file].color != board->side_to_move)
                continue;
            board_generate_legal_moves(board, file, rank, &moves);
            if (moves.count > 0) return 1;
            if (gas->squares[rank][file] < CF_GAS_FART_COST) continue;
            gas_scan_farts(board, gas, file, rank, &scan);
            for (d = 0; d < 8; ++d)
                if ((CfFartPreview)scan.preview[d] != CF_FART_INVALID)
                    return 1;
        }
    }
    return 0;
}

static void run_legal_probe_profile(void)
{
    CfBoard board;
    CfGasState gas;
    clock_t start;
    clock_t end;
    unsigned long fast_ms;
    unsigned long reference_ms;
    int repeat;

    board_init_starting_position(&board);
    gas_init(&gas);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat)
        legal_probe_profile_sink +=
            cpu_internal_has_legal_action(&board, &gas, 0);
    end = clock();
    fast_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat)
        legal_probe_profile_sink +=
            profile_reference_has_legal_action(&board, &gas);
    end = clock();
    reference_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("LEGAL_PROBE fast_ms=%lu reference_ms=%lu sink=%d\n",
           fast_ms, reference_ms, legal_probe_profile_sink);
}

static volatile int leaf_terminal_profile_sink;

static int profile_legacy_leaf_has_legal_action(const CfBoard *board,
                                                const CfGasState *gas)
{
    const CfPiece *piece;
    CfFartScan fart_scan;
    int actor_in_check = -1;
    int file;
    int rank;
    int d;

    if (board_has_legal_move(board)) return 1;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = &board->squares[rank][file];
            if (piece->type == CF_PIECE_NONE ||
                piece->color != board->side_to_move ||
                gas->squares[rank][file] < CF_GAS_FART_COST)
                continue;
            if (actor_in_check < 0)
                actor_in_check =
                    board_is_in_check(board, board->side_to_move);
            gas_scan_farts_prechecked(board, gas, file, rank,
                                      actor_in_check, &fart_scan);
            for (d = 0; d < 8; ++d)
                if ((CfFartPreview)fart_scan.preview[d] != CF_FART_INVALID)
                    return 1;
        }
    }
    return 0;
}

static void run_leaf_terminal_profile(void)
{
    CfBoard board;
    CfGasState gas;
    clock_t start;
    clock_t end;
    unsigned long fast_ms;
    unsigned long legacy_ms;
    int actor_in_check;
    int repeat;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 2, 1, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 1, 1, CF_PIECE_QUEEN, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 2, CF_PIECE_ROOK, CF_COLOR_BLACK);
    gas_set(&gas, 0, 0, 3U);
    board.side_to_move = CF_COLOR_WHITE;

    start = clock();
    for (repeat = 0; repeat < 300000; ++repeat) {
        actor_in_check = -1;
        if (!cpu_internal_has_legal_action(
                &board, &gas, &actor_in_check)) {
            if (actor_in_check < 0)
                actor_in_check =
                    board_is_in_check(&board, board.side_to_move);
            leaf_terminal_profile_sink += actor_in_check;
        }
    }
    end = clock();
    fast_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 300000; ++repeat) {
        if (!profile_legacy_leaf_has_legal_action(&board, &gas))
            leaf_terminal_profile_sink +=
                board_is_in_check(&board, board.side_to_move);
    }
    end = clock();
    legacy_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("LEAF_TERMINAL fast_ms=%lu legacy_ms=%lu sink=%d\n",
           fast_ms, legacy_ms, leaf_terminal_profile_sink);
}

static volatile int root_preflight_profile_sink;

static void run_root_preflight_profile(void)
{
    static CfCpuActionList list;
    CfBoard board;
    CfGasState gas;
    CfGasHistory history;
    clock_t start;
    clock_t end;
    unsigned long direct_ms;
    unsigned long legacy_ms;
    int repeat;

    board_init_starting_position(&board);
    gas_init(&gas);
    gas_history_init(&history, &board, &gas);

    start = clock();
    for (repeat = 0; repeat < 2000; ++repeat) {
        cpu_generate_actions(&board, &gas, &list);
        root_preflight_profile_sink += list.count;
        if (list.count > 0 &&
            !board_is_insufficient_material(&board) &&
            gas_history_repetition_count(&history, &board, &gas) < 3 &&
            board.halfmove_clock < 100U)
            ++root_preflight_profile_sink;
    }
    end = clock();
    direct_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 2000; ++repeat) {
        root_preflight_profile_sink +=
            (int)gas_game_status(&board, &gas, &history);
        cpu_generate_actions(&board, &gas, &list);
        root_preflight_profile_sink += list.count;
    }
    end = clock();
    legacy_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("ROOT_PREFLIGHT direct_ms=%lu legacy_ms=%lu sink=%d\n",
           direct_ms, legacy_ms, root_preflight_profile_sink);
}

static volatile int check_lookup_profile_sink;

static void run_check_lookup_profile(void)
{
    CfBoard board;
    clock_t start;
    clock_t end;
    unsigned long scanned_ms;
    unsigned long cached_ms;
    int repeat;

    board_init_starting_position(&board);
    board.side_to_move = CF_COLOR_BLACK;

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat)
        check_lookup_profile_sink +=
            board_is_in_check(&board, CF_COLOR_BLACK);
    end = clock();
    scanned_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat)
        check_lookup_profile_sink +=
            board_square_is_attacked(&board, 4, 7, CF_COLOR_WHITE);
    end = clock();
    cached_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("KING_CHECK scanned_ms=%lu cached_ms=%lu sink=%d\n",
           scanned_ms, cached_ms, check_lookup_profile_sink);
}

static volatile int eval_check_profile_sink;

static void run_eval_check_profile(void)
{
    CfBoard board;
    clock_t start;
    clock_t end;
    unsigned long scanned_pair_ms;
    unsigned long folded_pair_ms;
    int repeat;

    board_init_starting_position(&board);

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat) {
        eval_check_profile_sink +=
            board_is_in_check(&board, CF_COLOR_WHITE);
        eval_check_profile_sink +=
            board_is_in_check(&board, CF_COLOR_BLACK);
    }
    end = clock();
    scanned_pair_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat) {
        eval_check_profile_sink +=
            board_square_is_attacked(&board, 4, 0, CF_COLOR_BLACK);
        eval_check_profile_sink +=
            board_square_is_attacked(&board, 4, 7, CF_COLOR_WHITE);
    }
    end = clock();
    folded_pair_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("EVAL_CHECK scanned_pair_ms=%lu folded_pair_ms=%lu sink=%d\n",
           scanned_pair_ms, folded_pair_ms, eval_check_profile_sink);
}

static volatile int movegen_king_profile_sink;

static void run_movegen_king_profile(void)
{
    CfBoard board;
    CfMoveList moves;
    clock_t start;
    clock_t end;
    unsigned long public_ms;
    unsigned long prelocated_ms;
    int repeat;
    int file;
    int rank;

    board_init_starting_position(&board);
    board.side_to_move = CF_COLOR_BLACK;

    start = clock();
    for (repeat = 0; repeat < 100000; ++repeat) {
        for (rank = 0; rank < 8; ++rank) {
            for (file = 0; file < 8; ++file) {
                if (board.squares[rank][file].color != CF_COLOR_BLACK)
                    continue;
                board_generate_legal_moves(&board, file, rank, &moves);
                movegen_king_profile_sink += moves.count;
            }
        }
    }
    end = clock();
    public_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 100000; ++repeat) {
        for (rank = 0; rank < 8; ++rank) {
            for (file = 0; file < 8; ++file) {
                if (board.squares[rank][file].color != CF_COLOR_BLACK)
                    continue;
                board_generate_legal_moves_prelocated(
                    &board, file, rank, 4, 7, &moves);
                movegen_king_profile_sink += moves.count;
            }
        }
    }
    end = clock();
    prelocated_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("MOVEGEN_KING public_ms=%lu prelocated_ms=%lu sink=%d\n",
           public_ms, prelocated_ms, movegen_king_profile_sink);
}

static volatile int action_bonus_king_profile_sink;

static void run_action_bonus_king_profile(void)
{
    CfBoard board;
    CfGasState gas;
    CfCpuActionList list;
    CfCpuConfig config;
    CfCpuUndo undo;
    clock_t start;
    clock_t end;
    unsigned long scanned_ms;
    unsigned long prelocated_ms;
    int king_file;
    int king_rank;
    int king_known;
    int action_index;
    int repeat;
    int i;

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&gas, 2, 2, 3U);
    board.side_to_move = CF_COLOR_WHITE;
    cpu_generate_actions(&board, &gas, &list);

    action_index = -1;
    for (i = 0; i < list.count; ++i) {
        if (list.actions[i].type == CF_CPU_ACTION_FART &&
            list.actions[i].direction == CF_FART_NE &&
            list.actions[i].fart_result == CF_FART_PUSH) {
            action_index = i;
            break;
        }
    }
    if (action_index < 0) {
        printf("ACTION_BONUS_KING ERROR no-action\n");
        return;
    }

    king_file = -1;
    king_rank = -1;
    king_known = cpu_internal_find_first_king(
        &board, CF_COLOR_BLACK, &king_file, &king_rank);
    cpu_config_for_difficulty(&config, CF_CPU_MEDIUM);
    if (!cpu_apply_action(&board, &gas, &list.actions[action_index], &undo)) {
        printf("ACTION_BONUS_KING ERROR apply\n");
        return;
    }

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat)
        action_bonus_king_profile_sink += cpu_internal_action_bonus(
            &board, &gas, &list.actions[action_index], &undo, &config, 0);
    end = clock();
    scanned_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 500000; ++repeat)
        action_bonus_king_profile_sink +=
            cpu_internal_action_bonus_prelocated(
                &board, &gas, &list.actions[action_index], &undo, &config, 0,
                king_known, king_file, king_rank);
    end = clock();
    prelocated_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("ACTION_BONUS_KING scanned_ms=%lu prelocated_ms=%lu sink=%d\n",
           scanned_ms, prelocated_ms, action_bonus_king_profile_sink);
    cpu_unapply_action(&board, &gas, &undo);
}

static volatile int status_fart_profile_sink;

static int profile_legacy_status_has_fart(const CfBoard *board,
                                          const CfGasState *gas)
{
    CfFartScan scan;
    int file;
    int rank;
    int direction;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (gas->squares[rank][file] < CF_GAS_FART_COST ||
                board->squares[rank][file].type == CF_PIECE_NONE ||
                board->squares[rank][file].color != board->side_to_move)
                continue;
            gas_scan_farts(board, gas, file, rank, &scan);
            for (direction = 0; direction < 8; ++direction)
                if ((CfFartPreview)scan.preview[direction] != CF_FART_INVALID)
                    return 1;
        }
    }
    return 0;
}

static CfGameStatus profile_legacy_gas_status(const CfBoard *board,
                                              const CfGasState *gas)
{
    int has_move;
    int check;
    int fart;

    has_move = board_has_legal_move(board);
    check = board_is_in_check(board, board->side_to_move);
    fart = profile_legacy_status_has_fart(board, gas);
    if (!has_move && !fart)
        return check ? CF_GAME_CHECKMATE : CF_GAME_STALEMATE;
    if (board_is_insufficient_material(board))
        return CF_GAME_DRAW_INSUFFICIENT;
    if (board->halfmove_clock >= 100U)
        return CF_GAME_DRAW_FIFTY_MOVE;
    if (check) return CF_GAME_CHECK;
    return CF_GAME_ONGOING;
}

static void run_status_fart_profile(void)
{
    CfBoard board;
    CfGasState gas;
    clock_t start;
    clock_t end;
    unsigned long fast_ms;
    unsigned long legacy_ms;
    unsigned long fart_only_fast_ms;
    unsigned long fart_only_legacy_ms;
    int repeat;
    int file;

    board_init_starting_position(&board);
    gas_init(&gas);
    for (file = 0; file < 8; ++file)
        gas_set(&gas, file, 1, 3U);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat)
        status_fart_profile_sink += (int)gas_game_status(&board, &gas, 0);
    end = clock();
    fast_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat)
        status_fart_profile_sink +=
            (int)profile_legacy_gas_status(&board, &gas);
    end = clock();
    legacy_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    board_clear(&board);
    gas_init(&gas);
    board_set_piece(&board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&board, 2, 1, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&board, 1, 2, CF_PIECE_QUEEN, CF_COLOR_BLACK);
    board_set_piece(&board, 7, 6, CF_PIECE_PAWN, CF_COLOR_WHITE);
    board_set_piece(&board, 7, 7, CF_PIECE_ROOK, CF_COLOR_BLACK);
    gas_set(&gas, 7, 6, 3U);
    board.side_to_move = CF_COLOR_WHITE;

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat)
        status_fart_profile_sink += (int)gas_game_status(&board, &gas, 0);
    end = clock();
    fart_only_fast_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    start = clock();
    for (repeat = 0; repeat < 200000; ++repeat)
        status_fart_profile_sink +=
            (int)profile_legacy_gas_status(&board, &gas);
    end = clock();
    fart_only_legacy_ms =
        (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);

    printf("STATUS_FART ongoing_fast_ms=%lu ongoing_legacy_ms=%lu "
           "fart_only_fast_ms=%lu fart_only_legacy_ms=%lu sink=%d\n",
           fast_ms, legacy_ms, fart_only_fast_ms, fart_only_legacy_ms,
           status_fart_profile_sink);
}

static unsigned long profile_perft(CfBoard *board, int depth)
{
    CfMoveList list;
    CfMove made;
    unsigned long nodes = 0UL;
    int file;
    int rank;
    int i;

    if (depth == 0) return 1UL;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            if (board->squares[rank][file].color != board->side_to_move)
                continue;
            board_generate_legal_moves(board, file, rank, &list);
            for (i = 0; i < list.count; ++i) {
                if (!board_make_move_ex(board, file, rank,
                                        list.moves[i].to_file,
                                        list.moves[i].to_rank,
                                        list.moves[i].promotion, &made))
                    continue;
                nodes += profile_perft(board, depth - 1);
                board_unmake_move(board, &made);
            }
        }
    }
    return nodes;
}

static void run_perft_profile(void)
{
    CfBoard board;
    clock_t start;
    clock_t end;
    unsigned long nodes;
    unsigned long elapsed_ms;

    board_init_starting_position(&board);
    start = clock();
    nodes = profile_perft(&board, 4);
    end = clock();
    elapsed_ms = (unsigned long)(((end - start) * 1000L) / CLOCKS_PER_SEC);
    printf("PERFT4 nodes=%lu elapsed_ms=%lu\n", nodes, elapsed_ms);
}

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
    printf("sizeof_cpu_action=%lu search_action_lists=%lu\n",
           (unsigned long)sizeof(CfCpuAction),
           (unsigned long)sizeof(CfCpuActionList) * 5UL);
    printf("sizeof_gas_move=%lu sizeof_cpu_undo=%lu sizeof_practice_undo=%lu\n",
           (unsigned long)sizeof(CfGasMove),
           (unsigned long)sizeof(CfCpuUndo),
           (unsigned long)sizeof(CfPracticeUndoJournal));
    printf("sizeof_replay_snapshot=%lu sizeof_replay_timeline=%lu transient_import_peak=%lu\n",
           (unsigned long)sizeof(CfReplaySnapshot),
           (unsigned long)sizeof(CfReplayTimeline),
           (unsigned long)sizeof(CfReplayTimeline) * 2UL);
    run_sort_profile();
    run_fart_scan_profile();
    run_fart_apply_profile();
    run_move_clear_profile();
    run_move_apply_profile();
    run_undo_write_profile();
    run_legal_probe_profile();
    run_leaf_terminal_profile();
    run_root_preflight_profile();
    run_check_lookup_profile();
    run_eval_check_profile();
    run_movegen_king_profile();
    run_action_bonus_king_profile();
    run_status_fart_profile();
    run_perft_profile();
    run_profile("EASY_START", CF_CPU_EASY);
    run_profile("MED_START", CF_CPU_MEDIUM);
    run_profile("HARD_START", CF_CPU_HARD);
    run_fart_profile();
    return 0;
}
