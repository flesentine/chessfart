#include <string.h>
#include "cpu_internal.h"

#define CF_CPU_INF 32000

static int in_bounds(int file, int rank)
{
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

static int piece_value(CfPieceType type)
{
    static const int values[7] = {0,100,320,330,500,900,0};
    int index = (int)type;
    if (index < 0 || index > 6) return 0;
    return values[index];
}

static int move_order_score(const CfMove *move)
{
    int score = 0;
    if (move->captured.type != CF_PIECE_NONE)
        score += piece_value(move->captured.type) * 10 -
                 piece_value(move->moved.type);
    if (move->promotion != CF_PIECE_NONE)
        score += 1200 + piece_value(move->promotion);
    if (move->moved.type == CF_PIECE_KING &&
        move->from_file == 4 &&
        (move->to_file == 2 || move->to_file == 6))
        score += 60;
    return score;
}

static int action_order_score(const CfBoard *board, const CfGasState *gas,
                              const CfCpuAction *action)
{
    static const int df[8] = {0,1,1,1,0,-1,-1,-1};
    static const int dr[8] = {1,1,0,-1,-1,-1,0,1};
    const CfPiece *actor;
    const CfPiece *target;
    int score = 0;
    int tf;
    int tr;

    if (action->type != CF_CPU_ACTION_FART) return -CF_CPU_INF;

    tf = action->from_file + df[(int)action->direction];
    tr = action->from_rank + dr[(int)action->direction];
    target = in_bounds(tf, tr) ? &board->squares[tr][tf] : 0;
    actor = &board->squares[action->from_rank][action->from_file];

    if (action->fart_result == CF_FART_PROMOTION) {
        if (target != 0 && target->type != CF_PIECE_NONE && actor != 0) {
            if (target->color == actor->color)
                score += 1500 + piece_value(action->promotion);
            else
                score -= 1500 + piece_value(action->promotion);
        } else {
            score -= 1500;
        }
    } else if (action->fart_result == CF_FART_PUSH) {
        score += 120;
        if (target != 0 && target->type != CF_PIECE_NONE && actor != 0) {
            if (target->color != actor->color)
                score += piece_value(target->type) / 2 + 40;
            else
                score -= piece_value(target->type) / 5;
        }
    } else if (action->fart_result == CF_FART_BLOCKED) {
        score -= 50;
    } else if (action->fart_result == CF_FART_PUFF) {
        score -= 80;
    }

    if (gas->squares[action->from_rank][action->from_file] == 3U) score += 10;
    return score;
}

static void add_move(CfCpuActionList *list, const CfMove *move)
{
    CfCpuAction *a;
    if (list->count >= CF_CPU_MAX_ACTIONS) return;
    a = &list->actions[list->count++];
    a->from_file = (cf_i8)move->from_file;
    a->from_rank = (cf_i8)move->from_rank;
    a->to_file = (cf_i8)move->to_file;
    a->to_rank = (cf_i8)move->to_rank;
    a->type = CF_CPU_ACTION_MOVE;
    a->promotion = (cf_u8)move->promotion;
    a->direction = 0U;
    a->fart_result = CF_FART_INVALID;
    a->order_score = (cf_i16)move_order_score(move);
}

static void add_fart(const CfBoard *board, const CfGasState *gas,
                     CfCpuActionList *list, int file, int rank,
                     CfFartDirection dir, CfFartPreview result,
                     CfPieceType promotion)
{
    CfCpuAction *a;
    if (list->count >= CF_CPU_MAX_ACTIONS) return;
    a = &list->actions[list->count++];
    a->from_file = (cf_i8)file;
    a->from_rank = (cf_i8)rank;
    a->to_file = -1;
    a->to_rank = -1;
    a->type = CF_CPU_ACTION_FART;
    a->promotion = (cf_u8)promotion;
    a->direction = (cf_u8)dir;
    a->fart_result = (cf_u8)result;
    a->order_score = (cf_i16)action_order_score(board, gas, a);
}

void cpu_generate_actions(const CfBoard *board, const CfGasState *gas,
                          CfCpuActionList *list)
{
    static const CfPieceType promotions[4] = {
        CF_PIECE_QUEEN, CF_PIECE_ROOK, CF_PIECE_BISHOP, CF_PIECE_KNIGHT
    };
    CfMoveList moves;
    const CfPiece *piece;
    CfFartScan fart_scan;
    CfFartPreview preview;
    int file;
    int rank;
    int i;
    int d;
    int p;

    if (list == 0) return;
    list->count = 0;
    if (board == 0 || gas == 0) return;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = &board->squares[rank][file];
            if (piece->type == CF_PIECE_NONE ||
                piece->color != board->side_to_move) continue;
            board_generate_legal_moves(board, file, rank, &moves);
            for (i = 0; i < moves.count; ++i) add_move(list, &moves.moves[i]);
            if (gas->squares[rank][file] < CF_GAS_FART_COST) continue;
            gas_scan_farts(board, gas, file, rank, &fart_scan);
            for (d = 0; d < 8; ++d) {
                preview = (CfFartPreview)fart_scan.preview[d];
                if (preview == CF_FART_INVALID) continue;
                if (preview == CF_FART_PROMOTION) {
                    for (p = 0; p < 4; ++p)
                        if ((fart_scan.promotion_mask[d] &
                             (cf_u8)(1U << p)) != 0U)
                            add_fart(board, gas, list, file, rank,
                                     (CfFartDirection)d, preview, promotions[p]);
                } else {
                    add_fart(board, gas, list, file, rank,
                             (CfFartDirection)d, preview, CF_PIECE_NONE);
                }
            }
        }
    }
}

int cpu_internal_has_legal_action(const CfBoard *board,
                                  const CfGasState *gas)
{
    CfMoveList moves;
    const CfPiece *piece;
    CfFartScan fart_scan;
    int file;
    int rank;
    int d;

    if (board == 0 || gas == 0) return 0;
    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = &board->squares[rank][file];
            if (piece->type == CF_PIECE_NONE ||
                piece->color != board->side_to_move) continue;

            board_generate_legal_moves(board, file, rank, &moves);
            if (moves.count > 0) return 1;

            if (gas->squares[rank][file] < CF_GAS_FART_COST) continue;
            gas_scan_farts(board, gas, file, rank, &fart_scan);
            for (d = 0; d < 8; ++d)
                if ((CfFartPreview)fart_scan.preview[d] != CF_FART_INVALID)
                    return 1;
        }
    }
    return 0;
}

static void insertion_sort_actions(CfCpuActionList *list)
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

void cpu_internal_sort_actions(CfCpuActionList *list, CfCpuActionList *scratch)
{
    CfCpuAction *src;
    CfCpuAction *dst;
    CfCpuAction *swap;
    int count;
    int width;
    int left;
    int middle;
    int right;
    int i;
    int j;
    int k;

    if (list == 0 || list->count < 2) return;
    if (scratch == 0 || scratch == list) {
        insertion_sort_actions(list);
        return;
    }

    count = list->count;
    src = list->actions;
    dst = scratch->actions;

    for (width = 1; width < count; width *= 2) {
        for (left = 0; left < count; left += width * 2) {
            middle = left + width;
            right = left + width * 2;
            if (middle > count) middle = count;
            if (right > count) right = count;
            i = left;
            j = middle;
            k = left;

            while (i < middle && j < right) {
                if (src[i].order_score >= src[j].order_score)
                    dst[k++] = src[i++];
                else
                    dst[k++] = src[j++];
            }
            while (i < middle) dst[k++] = src[i++];
            while (j < right) dst[k++] = src[j++];
        }
        swap = src;
        src = dst;
        dst = swap;
    }

    if (src != list->actions)
        for (i = 0; i < count; ++i)
            list->actions[i] = src[i];
}

int cpu_apply_action(CfBoard *board, CfGasState *gas,
                     const CfCpuAction *action, CfCpuUndo *undo)
{
    CfCpuUndo local;
    int ok;
    if (board == 0 || gas == 0 || action == 0) return 0;
    memset(&local, 0, sizeof(local));
    local.type = action->type;
    if (action->type == CF_CPU_ACTION_MOVE)
        ok = gas_make_move_ex(board, gas, action->from_file, action->from_rank,
                              action->to_file, action->to_rank,
                              action->promotion, &local.action.move);
    else if (action->type == CF_CPU_ACTION_FART)
        ok = gas_make_fart(board, gas, action->from_file, action->from_rank,
                           action->direction, action->promotion, &local.action.fart);
    else return 0;
    if (!ok) return 0;
    if (undo != 0) *undo = local;
    return 1;
}

void cpu_unapply_action(CfBoard *board, CfGasState *gas, const CfCpuUndo *undo)
{
    if (board == 0 || gas == 0 || undo == 0) return;
    if (undo->type == CF_CPU_ACTION_MOVE) gas_unmake_move(board, gas, &undo->action.move);
    else if (undo->type == CF_CPU_ACTION_FART) gas_unmake_fart(board, gas, &undo->action.fart);
}
