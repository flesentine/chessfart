#include "ui_assets.h"
#include "ui_theme.h"
#include "vga.h"

#include "../generated/ui_assets_generated.inc"

/*
 * Build 13 assets are authored as indexed PNGs and converted ahead of time.
 * Piece pixels are packed two 4-bit semantic classes per byte; puff masks are
 * packed one bit per pixel. Runtime palette mapping remains here so source art
 * is independent of raw VGA palette numbers.
 */

static const cf_u8 *piece_sprite(const CfPiece *piece)
{
    if (piece == 0 || piece->type == CF_PIECE_NONE) return 0;
    if (piece->color == CF_COLOR_WHITE) {
        switch (piece->type) {
        case CF_PIECE_KING: return cf_asset_white_king;
        case CF_PIECE_QUEEN: return cf_asset_white_queen;
        case CF_PIECE_ROOK: return cf_asset_white_rook;
        case CF_PIECE_BISHOP: return cf_asset_white_bishop;
        case CF_PIECE_KNIGHT: return cf_asset_white_knight;
        case CF_PIECE_PAWN: return cf_asset_white_pawn;
        default: return 0;
        }
    }
    switch (piece->type) {
    case CF_PIECE_KING: return cf_asset_black_king;
    case CF_PIECE_QUEEN: return cf_asset_black_queen;
    case CF_PIECE_ROOK: return cf_asset_black_rook;
    case CF_PIECE_BISHOP: return cf_asset_black_bishop;
    case CF_PIECE_KNIGHT: return cf_asset_black_knight;
    case CF_PIECE_PAWN: return cf_asset_black_pawn;
    default: return 0;
    }
}

static cf_u8 piece_color(const CfPiece *piece, int code)
{
    if (code <= 1) return CF_UI_COL_SHADOW;
    if (piece->color == CF_COLOR_WHITE) {
        if (code == 2) return CF_UI_COL_GOLD;
        if (code == 3) return CF_UI_COL_WHITE_PIECE;
        return CF_UI_COL_WHITE_HI;
    }
    if (code == 2) return CF_UI_COL_BLACK_PIECE;
    if (code == 3) return CF_UI_COL_BLACK_HI;
    return CF_UI_COL_CURSOR;
}

static int piece_pixel(const cf_u8 *sprite, int x, int y)
{
    int pixel;
    cf_u8 packed;

    if (sprite == 0 || x < 0 || x > 15 || y < 0 || y > 15) return 0;
    pixel = y * 16 + x;
    packed = sprite[pixel >> 1];
    if ((pixel & 1) == 0) return (int)((packed >> 4) & 15U);
    return (int)(packed & 15U);
}

static int mask_pixel(const cf_u8 *mask, int x, int y)
{
    int pixel;
    cf_u8 packed;

    if (mask == 0 || x < 0 || x >= CF_UI_PUFF_W ||
        y < 0 || y >= CF_UI_PUFF_H) return 0;
    pixel = y * CF_UI_PUFF_W + x;
    packed = mask[pixel >> 3];
    return (packed & (cf_u8)(1U << (7 - (pixel & 7)))) != 0U;
}

static int exposed_edge(const cf_u8 *mask, int x, int y)
{
    return !mask_pixel(mask, x - 1, y) || !mask_pixel(mask, x, y - 1);
}

void ui_assets_draw_piece(int x, int y, const CfPiece *piece, cf_u8 background)
{
    const cf_u8 *sprite;
    int px;
    int py;
    int code;

    sprite = piece_sprite(piece);
    if (sprite == 0) return;

    vga_fill_rect(x, y, CF_UI_PIECE_W, CF_UI_PIECE_H, background);
    for (py = 0; py < 16; ++py) {
        for (px = 0; px < 16; ++px) {
            code = piece_pixel(sprite, px, py);
            if (code <= 0) continue;
            vga_put_pixel(x + 1 + px, y + py, piece_color(piece, code));
        }
    }
}

void ui_assets_draw_puff(int x, int y, int frame)
{
    const cf_u8 *frames[5];
    const cf_u8 *shape;
    int px;
    int py;

    frames[0] = cf_asset_puff1;
    frames[1] = cf_asset_puff2;
    frames[2] = cf_asset_puff3;
    frames[3] = cf_asset_puff4;
    frames[4] = cf_asset_puff5;

    if (frame < 0) frame = 0;
    if (frame > 4) frame = 4;
    shape = frames[frame];

    for (py = 0; py < CF_UI_PUFF_H; ++py) {
        for (px = 0; px < CF_UI_PUFF_W; ++px) {
            if (!mask_pixel(shape, px, py)) continue;
            vga_put_pixel(x + px, y + py,
                          exposed_edge(shape, px, py)
                              ? CF_UI_COL_GAS : CF_UI_COL_CLOUD);
        }
    }
}
