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
int cpu_internal_evaluate(const CfBoard *b, const CfGasState *g)
{
    const CfPiece *p;
    int f, r, sign, v, gv, score = 0;
    for (r = 0; r < 8; ++r) for (f = 0; f < 8; ++f) {
        p = board_piece_at(b, f, r);
        if (p == 0 || p->type == CF_PIECE_NONE) continue;
        sign = p->color == CF_COLOR_WHITE ? 1 : -1;
        v = piece_value(p->type); gv = (int)gas_at(g, f, r);
        score += sign * v + sign * gv * 10;
        if (gv >= 2) score += sign * 14;
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
