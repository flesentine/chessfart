#include "ui_assets.h"
#include "vga.h"

#define COL_GAS 8
#define COL_WHITE_PIECE 11
#define COL_WHITE_HI 12
#define COL_BLACK_PIECE 13
#define COL_BLACK_HI 14
#define COL_SHADOW 17
#define COL_CLOUD 25

/*
 * Production VGA sprites.
 *
 * The first art pass embedded large per-pixel arrays generated from the
 * concept atlas.  They were hard to edit and produced inconsistent shapes.
 * These masks keep the same 18x16 runtime footprint but describe each chess
 * silhouette in a compact, hand-tunable form.  White and black share geometry
 * and get their identity from the VGA palette, which keeps the two sets
 * visually consistent and dramatically reduces the asset code.
 */

static const char *shape_pawn[16] = {
    "................", ".......##.......", "......####......", "......####......",
    ".......##.......", "......####......", ".....######.....", "......####......",
    "......####......", "......####......", ".....######.....", "....########....",
    "...##########...", "..############..", "..############..", "................"
};

static const char *shape_knight[16] = {
    "..........##....", ".......#####....", ".....#######....", "....###.####....",
    "...###..####....", "..###...####....", "..##########....", "...#########....",
    "....########....", ".....#######....", ".....#######....", "....########....",
    "...##########...", "..############..", "..############..", "................"
};

static const char *shape_bishop[16] = {
    ".......##.......", "......####......", ".....######.....", ".....###.##.....",
    ".....##.###.....", ".....######.....", "......####......", ".....######.....",
    ".....######.....", "....########....", "....########....", "...##########...",
    "...##########...", "..############..", "..############..", "................"
};

static const char *shape_rook[16] = {
    "..##..##..##....", "..############..", "...##########...", "...##########...",
    "....########....", "....########....", "....########....", "....########....",
    "....########....", "....########....", "...##########...", "...##########...",
    "..############..", ".##############.", ".##############.", "................"
};

static const char *shape_queen[16] = {
    "..##...##...##..", "..###..##..###..", "...############.", "...##########...",
    "....########....", "....########....", ".....######.....", ".....######.....",
    "....########....", "....########....", "...##########...", "...##########...",
    "..############..", ".##############.", ".##############.", "................"
};

static const char *shape_king[16] = {
    ".......##.......", ".....######.....", ".......##.......", ".....######.....",
    "....########....", ".....######.....", "......####......", ".....######.....",
    ".....######.....", "....########....", "....########....", "...##########...",
    "...##########...", "..############..", "..############..", "................"
};

static const char *puff1[16] = {
    "................", "................", "................", "................",
    "................", ".......##.......", "......####......", ".....######.....",
    "......####......", ".......##.......", "................", "................",
    "................", "................", "................", "................"
};

static const char *puff2[16] = {
    "................", "................", "................", ".......##.......",
    "....##.####.....", "...#########....", "..###########...", "..###########...",
    "...#########....", "....#######.....", "......###.......", "................",
    "................", "................", "................", "................"
};

static const char *puff3[16] = {
    "................", ".....###........", "..###.#####.....", ".###########....",
    ".############...", "#############...", "##############..", ".#############..",
    "..###########...", "...#########....", ".....######.....", ".......###......",
    "................", "................", "................", "................"
};

static const char *puff4[16] = {
    "..###...........", ".#####..####.....", "########.#####..", "##############..",
    "###############.", ".##############..", "..############..", "...###########..",
    ".....########...", ".......######...", ".........####...", "...........##...",
    "................", "................", "................", "................"
};

static const char *puff5[16] = {
    "................", "...........##...", "..........###...", "................",
    "....##..........", "...####..........", "................", "........##......",
    ".......###......", "................", "..##............", ".###.............",
    ".........##.....", "................", "................", "................"
};

static const char **piece_shape(CfPieceType type)
{
    switch (type) {
    case CF_PIECE_PAWN: return shape_pawn;
    case CF_PIECE_KNIGHT: return shape_knight;
    case CF_PIECE_BISHOP: return shape_bishop;
    case CF_PIECE_ROOK: return shape_rook;
    case CF_PIECE_QUEEN: return shape_queen;
    case CF_PIECE_KING: return shape_king;
    default: return 0;
    }
}

static int mask_pixel(const char **mask, int x, int y)
{
    if (mask == 0 || x < 0 || x > 15 || y < 0 || y > 15) return 0;
    return mask[y][x] == '#';
}

static int exposed_edge(const char **mask, int x, int y)
{
    return !mask_pixel(mask, x - 1, y) || !mask_pixel(mask, x, y - 1);
}

void ui_assets_draw_piece(int x, int y, const CfPiece *piece, cf_u8 background)
{
    const char **shape;
    cf_u8 body;
    cf_u8 hi;
    int px;
    int py;

    if (piece == 0 || piece->type == CF_PIECE_NONE) return;
    shape = piece_shape(piece->type);
    if (shape == 0) return;

    body = piece->color == CF_COLOR_WHITE ? COL_WHITE_PIECE : COL_BLACK_PIECE;
    hi = piece->color == CF_COLOR_WHITE ? COL_WHITE_HI : COL_BLACK_HI;

    vga_fill_rect(x, y, CF_UI_PIECE_W, CF_UI_PIECE_H, background);

    /* One-pixel dark keyline makes both sets readable on either board color. */
    for (py = 0; py < 16; ++py) {
        for (px = 0; px < 16; ++px) {
            if (mask_pixel(shape, px, py)) continue;
            if (mask_pixel(shape, px - 1, py) || mask_pixel(shape, px + 1, py) ||
                mask_pixel(shape, px, py - 1) || mask_pixel(shape, px, py + 1)) {
                vga_put_pixel(x + 1 + px, y + py, COL_SHADOW);
            }
        }
    }

    /* A light north/west rim gives the silhouettes a restrained VGA bevel. */
    for (py = 0; py < 16; ++py) {
        for (px = 0; px < 16; ++px) {
            if (!mask_pixel(shape, px, py)) continue;
            vga_put_pixel(x + 1 + px, y + py,
                          exposed_edge(shape, px, py) ? hi : body);
        }
    }
}

void ui_assets_draw_puff(int x, int y, int frame)
{
    const char **frames[5];
    const char **shape;
    int px;
    int py;

    frames[0] = puff1;
    frames[1] = puff2;
    frames[2] = puff3;
    frames[3] = puff4;
    frames[4] = puff5;

    if (frame < 0) frame = 0;
    if (frame > 4) frame = 4;
    shape = frames[frame];

    for (py = 0; py < CF_UI_PUFF_H; ++py) {
        for (px = 0; px < CF_UI_PUFF_W; ++px) {
            if (!mask_pixel(shape, px, py)) continue;
            vga_put_pixel(x + px, y + py,
                          exposed_edge(shape, px, py) ? COL_GAS : COL_CLOUD);
        }
    }
}
