#include "cpu_internal.h"

static int in_bounds(int f, int r) { return f >= 0 && f < 8 && r >= 0 && r < 8; }
static int piece_value(CfPieceType t)
{
    switch (t) {
    case CF_PIECE_PAWN: return 100;
    case CF_PIECE_KNIGHT: return 320;
    case CF_PIECE_BISHOP: return 330;
    case CF_PIECE_ROOK: return 500;
    case CF_PIECE_QUEEN: return 900;
    default: return 0;
    }
}

static int square_centrality(int f, int r)
{
    int cf;
    int cr;
    cf = f < 4 ? f : 7 - f;
    cr = r < 4 ? r : 7 - r;
    return cf + cr;
}

static unsigned rights_for_color(CfPieceColor color)
{
    if (color == CF_COLOR_WHITE)
        return CF_CASTLE_WHITE_KING | CF_CASTLE_WHITE_QUEEN;
    if (color == CF_COLOR_BLACK)
        return CF_CASTLE_BLACK_KING | CF_CASTLE_BLACK_QUEEN;
    return 0U;
}

static int bit_count(unsigned value)
{
    int count = 0;
    while (value != 0U) {
        if ((value & 1U) != 0U) ++count;
        value >>= 1;
    }
    return count;
}

static int pressure(const CfBoard *b, const CfGasState *g, int f, int r, CfPieceColor c)
{
    static const int df[8] = {0,1,1,1,0,-1,-1,-1};
    static const int dr[8] = {1,1,0,-1,-1,-1,0,1};
    const CfPiece *t;
    int i, tf, tr, pf, pr, s = 0;
    if (gas_at(g, f, r) < CF_GAS_FART_COST) return 0;
    for (i = 0; i < 8; ++i) {
        tf = f + df[i]; tr = r + dr[i]; pf = tf + df[i]; pr = tr + dr[i];
        if (!in_bounds(tf,tr) || !in_bounds(pf,pr)) continue;
        t = board_piece_at(b, tf, tr);
        if (t == 0 || t->type == CF_PIECE_NONE || b->squares[pr][pf].type != CF_PIECE_NONE) continue;
        s += t->color != c ? 8 + piece_value(t->type) / 80 : 2;
    }
    return s;
}

int cpu_internal_action_bonus(const CfBoard *after, const CfGasState *gas,
                              const CfCpuAction *action,
                              const CfCpuUndo *undo,
                              const CfCpuConfig *config,
                              int actor_was_in_check)
{
    static const int df[8] = {0,1,1,1,0,-1,-1,-1};
    static const int dr[8] = {1,1,0,-1,-1,-1,0,1};
    CfPieceColor actor_color;
    CfPieceColor opponent_color;
    const CfPiece *target;
    int tf;
    int tr;
    int pf;
    int pr;
    int bonus;
    int promotion_gain;
    unsigned lost_rights;
    unsigned lost_actor_rights;

    (void)gas;
    if (after == 0 || action == 0 || undo == 0 || config == 0)
        return 0;
    if (action->type != CF_CPU_ACTION_FART) return 0;

    actor_color = board_other_color(after->side_to_move);
    opponent_color = after->side_to_move;
    bonus = 0;
    target = &undo->fart.previous_target_piece;

    if (action->fart_result == CF_FART_PROMOTION) {
        promotion_gain = piece_value(action->promotion) - piece_value(CF_PIECE_PAWN);
        if (target->type == CF_PIECE_PAWN && target->color == actor_color)
            bonus += config->fart_bias + 45 + promotion_gain / 10;
        else if (target->type == CF_PIECE_PAWN && target->color == opponent_color)
            bonus -= config->fart_bias + 60 + promotion_gain / 6;
        else
            bonus -= 80;
    } else if (action->fart_result == CF_FART_PUSH) {
        tf = action->from_file + df[(int)action->direction];
        tr = action->from_rank + dr[(int)action->direction];
        pf = tf + df[(int)action->direction];
        pr = tr + dr[(int)action->direction];

        if (target->type != CF_PIECE_NONE && target->color == opponent_color) {
            bonus += config->fart_bias + 12 + piece_value(target->type) / 25;
            if (target->type == CF_PIECE_KING) bonus += 55;
            if (in_bounds(tf, tr) && in_bounds(pf, pr))
                bonus += (square_centrality(tf, tr) - square_centrality(pf, pr)) * 3;
        } else if (target->type != CF_PIECE_NONE && target->color == actor_color) {
            bonus += config->fart_bias / 4 - 12;
            if (in_bounds(tf, tr) && in_bounds(pf, pr))
                bonus += (square_centrality(pf, pr) - square_centrality(tf, tr)) * 2;
        } else {
            bonus -= 20;
        }
    } else if (action->fart_result == CF_FART_PUFF) {
        bonus -= 24;
    } else if (action->fart_result == CF_FART_BLOCKED) {
        bonus -= 60;
    }

    if (board_is_in_check(after, after->side_to_move)) bonus += 55;
    if (actor_was_in_check) bonus += 18;

    lost_rights = undo->fart.previous_castling_rights &
                  ~after->castling_rights & rights_for_color(opponent_color);
    bonus += bit_count(lost_rights) * 24;

    lost_actor_rights = undo->fart.previous_castling_rights &
                        ~after->castling_rights & rights_for_color(actor_color);
    bonus -= bit_count(lost_actor_rights) * 20;

    return bonus;
}

int cpu_internal_evaluate(const CfBoard *b, const CfGasState *g)
{
    const CfPiece *p;
    int f, r, sign, v, gv, score = 0;
    for (r = 0; r < 8; ++r) for (f = 0; f < 8; ++f) {
        p = board_piece_at(b, f, r);
        if (p == 0 || p->type == CF_PIECE_NONE) continue;
        sign = p->color == CF_COLOR_WHITE ? 1 : -1;
        v = piece_value(p->type); gv = (int)gas_at(g, f, r);
        score += sign * v + sign * gv * 8;
        if (gv >= 2) score += sign * 10;
        score += sign * pressure(b, g, f, r, p->color);
        if (f >= 2 && f <= 5 && r >= 2 && r <= 5) score += sign * 5;
        if (p->type == CF_PIECE_PAWN) score += sign * (p->color == CF_COLOR_WHITE ? r : 7-r) * 2;
    }
    if (board_is_in_check(b, CF_COLOR_WHITE)) score -= 35;
    if (board_is_in_check(b, CF_COLOR_BLACK)) score += 35;
    if ((b->castling_rights & CF_CASTLE_WHITE_KING) != 0U) score += 5;
    if ((b->castling_rights & CF_CASTLE_WHITE_QUEEN) != 0U) score += 4;
    if ((b->castling_rights & CF_CASTLE_BLACK_KING) != 0U) score -= 5;
    if ((b->castling_rights & CF_CASTLE_BLACK_QUEEN) != 0U) score -= 4;
    return b->side_to_move == CF_COLOR_WHITE ? score : -score;
}
