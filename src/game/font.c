#include <ctype.h>
#include <string.h>

#include "font.h"
#include "ui_theme.h"
#include "vga.h"

#include "../generated/font_generated.inc"

#define CF_FONT_DRAW_W 5
#define CF_FONT_DRAW_H 7
#define CF_FONT_ADVANCE 6

static int normalized_code(char ch)
{
    int code = (int)(unsigned char)ch;

    if (code >= 'a' && code <= 'z') {
        code = (int)toupper((unsigned char)ch);
    }
    if (code < CF_FONT_FIRST_CHAR ||
        code >= CF_FONT_FIRST_CHAR + CF_FONT_GLYPH_COUNT) {
        code = '?';
    }
    return code;
}

static int font_pixel(int code, int x, int y)
{
    int glyph;
    int atlas_x;
    int atlas_y;
    int pixel;
    cf_u8 packed;

    if (x < 0 || x >= CF_FONT_CELL_W ||
        y < 0 || y >= CF_FONT_CELL_H) {
        return 0;
    }

    glyph = normalized_code((char)code) - CF_FONT_FIRST_CHAR;
    atlas_x = (glyph & 15) * CF_FONT_CELL_W + x;
    atlas_y = (glyph >> 4) * CF_FONT_CELL_H + y;
    pixel = atlas_y * CF_FONT_ATLAS_W + atlas_x;
    packed = cf_font_bitmap[pixel >> 3];
    return (packed & (cf_u8)(1U << (7 - (pixel & 7)))) != 0U;
}

static void draw_char_raw(int x, int y, char ch, cf_u8 color, int scale)
{
    int row;
    int col;
    int sx;
    int sy;
    int code;

    if (scale < 1) {
        scale = 1;
    }
    code = normalized_code(ch);

    for (row = 0; row < CF_FONT_DRAW_H; ++row) {
        for (col = 0; col < CF_FONT_DRAW_W; ++col) {
            if (!font_pixel(code, col, row)) {
                continue;
            }
            for (sy = 0; sy < scale; ++sy) {
                for (sx = 0; sx < scale; ++sx) {
                    vga_put_pixel(x + col * scale + sx,
                                  y + row * scale + sy,
                                  color);
                }
            }
        }
    }
}

static void draw_text_raw(int x, int y, const char *text,
                          cf_u8 color, int scale, int max_width)
{
    int cursor;
    int advance;
    int glyph_width;

    if (text == 0) {
        return;
    }
    if (scale < 1) {
        scale = 1;
    }

    cursor = x;
    advance = CF_FONT_ADVANCE * scale;
    glyph_width = CF_FONT_DRAW_W * scale;

    while (*text != '\0') {
        if (max_width >= 0 && cursor + glyph_width > x + max_width) {
            break;
        }
        draw_char_raw(cursor, y, *text, color, scale);
        cursor += advance;
        ++text;
    }
}

static int accent_text_color(cf_u8 color)
{
    return color == CF_UI_COL_GOLD ||
           color == CF_UI_COL_CURSOR ||
           color == CF_UI_COL_SELECTED ||
           color == CF_UI_COL_GAS;
}

void font_draw_char(int x, int y, char ch, cf_u8 color, int scale)
{
    if (scale < 1) {
        scale = 1;
    }
    if (accent_text_color(color)) {
        draw_char_raw(x + scale, y + scale, ch, CF_UI_COL_SHADOW, scale);
    }
    draw_char_raw(x, y, ch, color, scale);
}

void font_draw_text(int x, int y, const char *text, cf_u8 color, int scale)
{
    if (scale < 1) {
        scale = 1;
    }
    if (accent_text_color(color)) {
        draw_text_raw(x + scale, y + scale, text,
                      CF_UI_COL_SHADOW, scale, -1);
    }
    draw_text_raw(x, y, text, color, scale, -1);
}

void font_draw_heading(int x, int y, const char *text,
                       cf_u8 color, int scale)
{
    if (scale < 1) {
        scale = 1;
    }
    draw_text_raw(x + scale, y + scale, text,
                  CF_UI_COL_SHADOW, scale, -1);
    draw_text_raw(x, y, text, color, scale, -1);
}

void font_draw_text_clipped(int x, int y, const char *text,
                            cf_u8 color, int scale, int max_width)
{
    if (max_width <= 0) {
        return;
    }
    if (scale < 1) {
        scale = 1;
    }
    if (accent_text_color(color) && max_width > scale) {
        draw_text_raw(x + scale, y + scale, text,
                      CF_UI_COL_SHADOW, scale,
                      max_width - scale);
    }
    draw_text_raw(x, y, text, color, scale, max_width);
}

int font_text_width(const char *text, int scale)
{
    int len;

    if (text == 0) {
        return 0;
    }
    if (scale < 1) {
        scale = 1;
    }
    len = (int)strlen(text);
    if (len == 0) {
        return 0;
    }
    return len * CF_FONT_ADVANCE * scale - scale;
}
