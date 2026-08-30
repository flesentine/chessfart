#include "board_view.h"
#include "cf_types.h"
#include "font.h"
#include "vga.h"

enum {
    COL_BG = 0,
    COL_PANEL = 1,
    COL_SQUARE_LIGHT = 2,
    COL_SQUARE_DARK = 3,
    COL_GOLD = 4,
    COL_TEXT = 5,
    COL_MUTED = 6,
    COL_GREEN = 7,
    COL_GAS = 8,
    COL_BLACK = 9,
    COL_PANEL_EDGE = 10
};

static void set_rgb(cf_u8 *palette, int index, int r, int g, int b)
{
    int base = index * 3;
    palette[base] = (cf_u8)r;
    palette[base + 1] = (cf_u8)g;
    palette[base + 2] = (cf_u8)b;
}

static void load_build1_palette(void)
{
    cf_u8 palette[VGA_PALETTE_BYTES];
    int i;
    int shade;

    for (i = 0; i < VGA_PALETTE_BYTES; ++i) {
        palette[i] = 0;
    }

    set_rgb(palette, COL_BG, 4, 5, 8);
    set_rgb(palette, COL_PANEL, 9, 11, 16);
    set_rgb(palette, COL_SQUARE_LIGHT, 49, 43, 31);
    set_rgb(palette, COL_SQUARE_DARK, 18, 23, 19);
    set_rgb(palette, COL_GOLD, 63, 49, 12);
    set_rgb(palette, COL_TEXT, 63, 63, 55);
    set_rgb(palette, COL_MUTED, 31, 34, 31);
    set_rgb(palette, COL_GREEN, 18, 44, 14);
    set_rgb(palette, COL_GAS, 41, 63, 15);
    set_rgb(palette, COL_BLACK, 0, 0, 0);
    set_rgb(palette, COL_PANEL_EDGE, 20, 24, 30);

    for (i = 16; i < VGA_PALETTE_COLORS; ++i) {
        shade = (i - 16) * 63 / (VGA_PALETTE_COLORS - 17);
        set_rgb(palette, i, shade, shade, shade);
    }

    vga_set_palette(palette);
}

static void draw_board(void)
{
    const int board_x = 10;
    const int board_y = 24;
    const int square = 20;
    int file;
    int rank;
    cf_u8 color;

    vga_fill_rect(board_x - 2, board_y - 2, 164, 164, COL_GOLD);
    vga_fill_rect(board_x, board_y, 160, 160, COL_BLACK);

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            color = ((file + rank) & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT;
            vga_fill_rect(board_x + file * square,
                          board_y + rank * square,
                          square, square, color);
        }
    }
}

static void draw_panel(void)
{
    int i;
    vga_fill_rect(181, 22, 129, 164, COL_PANEL_EDGE);
    vga_fill_rect(183, 24, 125, 160, COL_PANEL);

    font_draw_text(190, 31, "BUILD 1", COL_GOLD, 1);
    font_draw_text(190, 46, "VGA BOOT", COL_TEXT, 1);
    font_draw_text(190, 63, "MODE 13H", COL_MUTED, 1);
    font_draw_text(190, 73, "320X200", COL_MUTED, 1);
    font_draw_text(190, 83, "256 COLOR", COL_MUTED, 1);

    font_draw_text(190, 105, "GAS CORE", COL_GREEN, 1);
    vga_fill_rect(190, 117, 92, 8, COL_BLACK);
    for (i = 0; i < 3; ++i) {
        vga_fill_rect(192 + i * 29, 119, 25, 4, i < 2 ? COL_GAS : COL_MUTED);
    }

    font_draw_text(190, 140, "BOARD OK", COL_TEXT, 1);
    font_draw_text(190, 151, "PALETTE OK", COL_TEXT, 1);
    font_draw_text(190, 169, "ESC QUITS", COL_GOLD, 1);
}

void board_view_render_build1(void)
{
    const char *title = "CHESS FART";

    load_build1_palette();
    vga_clear(COL_BG);

    font_draw_text(10, 7, title, COL_TEXT, 1);
    font_draw_text(75, 7, "- CHECK MATE VENTILATE", COL_GOLD, 1);

    draw_board();
    draw_panel();

    font_draw_text(10, 190, "BUILD 1 DOS VGA BOOT", COL_MUTED, 1);
}
