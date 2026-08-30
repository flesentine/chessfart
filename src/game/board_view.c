#include <stdio.h>
#include <string.h>

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
    COL_PANEL_EDGE = 10,
    COL_WHITE_PIECE = 11,
    COL_WHITE_HI = 12,
    COL_BLACK_PIECE = 13,
    COL_BLACK_HI = 14,
    COL_CURSOR = 15,
    COL_SELECTED = 16,
    COL_SHADOW = 17
};

#define BOARD_X 10
#define BOARD_Y 24
#define SQUARE_SIZE 20

static void set_rgb(cf_u8 *palette, int index, int r, int g, int b)
{
    int base = index * 3;
    palette[base] = (cf_u8)r;
    palette[base + 1] = (cf_u8)g;
    palette[base + 2] = (cf_u8)b;
}

static void load_build2_palette(void)
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
    set_rgb(palette, COL_WHITE_PIECE, 58, 57, 49);
    set_rgb(palette, COL_WHITE_HI, 63, 63, 63);
    set_rgb(palette, COL_BLACK_PIECE, 6, 7, 10);
    set_rgb(palette, COL_BLACK_HI, 24, 27, 32);
    set_rgb(palette, COL_CURSOR, 12, 56, 63);
    set_rgb(palette, COL_SELECTED, 63, 16, 42);
    set_rgb(palette, COL_SHADOW, 2, 2, 3);

    for (i = 18; i < VGA_PALETTE_COLORS; ++i) {
        shade = (i - 18) * 63 / (VGA_PALETTE_COLORS - 19);
        set_rgb(palette, i, shade, shade, shade);
    }

    vga_set_palette(palette);
}

static void draw_outline(int x, int y, int size, cf_u8 color)
{
    vga_fill_rect(x, y, size, 2, color);
    vga_fill_rect(x, y + size - 2, size, 2, color);
    vga_fill_rect(x, y, 2, size, color);
    vga_fill_rect(x + size - 2, y, 2, size, color);
}

static void draw_piece_shape(int x, int y, CfPieceType type, cf_u8 color)
{
    switch (type) {
    case CF_PIECE_PAWN:
        vga_fill_rect(x + 7, y + 3, 6, 5, color);
        vga_fill_rect(x + 6, y + 8, 8, 5, color);
        vga_fill_rect(x + 4, y + 13, 12, 3, color);
        break;
    case CF_PIECE_KNIGHT:
        vga_fill_rect(x + 6, y + 3, 7, 3, color);
        vga_fill_rect(x + 5, y + 5, 9, 4, color);
        vga_fill_rect(x + 7, y + 8, 7, 5, color);
        vga_fill_rect(x + 4, y + 13, 12, 3, color);
        vga_fill_rect(x + 4, y + 5, 3, 4, color);
        break;
    case CF_PIECE_BISHOP:
        vga_fill_rect(x + 8, y + 2, 4, 3, color);
        vga_fill_rect(x + 6, y + 5, 8, 7, color);
        vga_fill_rect(x + 7, y + 12, 6, 2, color);
        vga_fill_rect(x + 4, y + 14, 12, 2, color);
        break;
    case CF_PIECE_ROOK:
        vga_fill_rect(x + 4, y + 3, 3, 4, color);
        vga_fill_rect(x + 9, y + 3, 2, 4, color);
        vga_fill_rect(x + 13, y + 3, 3, 4, color);
        vga_fill_rect(x + 5, y + 6, 10, 8, color);
        vga_fill_rect(x + 3, y + 14, 14, 2, color);
        break;
    case CF_PIECE_QUEEN:
        vga_fill_rect(x + 4, y + 3, 3, 3, color);
        vga_fill_rect(x + 9, y + 2, 2, 4, color);
        vga_fill_rect(x + 14, y + 3, 3, 3, color);
        vga_fill_rect(x + 5, y + 6, 11, 7, color);
        vga_fill_rect(x + 4, y + 13, 13, 3, color);
        break;
    case CF_PIECE_KING:
        vga_fill_rect(x + 9, y + 1, 2, 5, color);
        vga_fill_rect(x + 7, y + 3, 6, 2, color);
        vga_fill_rect(x + 6, y + 6, 8, 7, color);
        vga_fill_rect(x + 4, y + 13, 12, 3, color);
        break;
    default:
        break;
    }
}

static void draw_piece(int screen_x, int screen_y, const CfPiece *piece)
{
    cf_u8 body;
    cf_u8 highlight;
    cf_u8 label;
    char text[2];

    if (piece == 0 || piece->type == CF_PIECE_NONE) {
        return;
    }

    if (piece->color == CF_COLOR_WHITE) {
        body = COL_WHITE_PIECE;
        highlight = COL_WHITE_HI;
        label = COL_BLACK;
    } else {
        body = COL_BLACK_PIECE;
        highlight = COL_BLACK_HI;
        label = COL_TEXT;
    }

    draw_piece_shape(screen_x + 1, screen_y + 1, piece->type, COL_SHADOW);
    draw_piece_shape(screen_x, screen_y, piece->type, body);
    vga_fill_rect(screen_x + 7, screen_y + 4, 2, 2, highlight);

    text[0] = board_piece_letter(piece->type);
    text[1] = '\0';
    font_draw_text(screen_x + 8, screen_y + 8, text, label, 1);
}

static void draw_board(const CfBoard *board,
                       int cursor_file,
                       int cursor_rank,
                       int has_selection,
                       int selected_file,
                       int selected_rank)
{
    int file;
    int rank;
    int screen_rank;
    int x;
    int y;
    cf_u8 color;
    const CfPiece *piece;

    vga_fill_rect(BOARD_X - 2, BOARD_Y - 2, 164, 164, COL_GOLD);
    vga_fill_rect(BOARD_X, BOARD_Y, 160, 160, COL_BLACK);

    for (rank = 0; rank < 8; ++rank) {
        screen_rank = 7 - rank;
        for (file = 0; file < 8; ++file) {
            x = BOARD_X + file * SQUARE_SIZE;
            y = BOARD_Y + screen_rank * SQUARE_SIZE;
            color = ((file + screen_rank) & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT;
            vga_fill_rect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);
            piece = board_piece_at(board, file, rank);
            draw_piece(x, y, piece);
        }
    }

    if (has_selection) {
        x = BOARD_X + selected_file * SQUARE_SIZE;
        y = BOARD_Y + (7 - selected_rank) * SQUARE_SIZE;
        draw_outline(x + 2, y + 2, SQUARE_SIZE - 4, COL_SELECTED);
    }

    x = BOARD_X + cursor_file * SQUARE_SIZE;
    y = BOARD_Y + (7 - cursor_rank) * SQUARE_SIZE;
    draw_outline(x, y, SQUARE_SIZE, COL_CURSOR);
}

static void square_name(char *out, int file, int rank)
{
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

static void draw_panel(const CfBoard *board,
                       int cursor_file,
                       int cursor_rank,
                       int has_selection,
                       int selected_file,
                       int selected_rank)
{
    char square[3];
    char line[24];
    const CfPiece *piece;

    vga_fill_rect(181, 22, 129, 164, COL_PANEL_EDGE);
    vga_fill_rect(183, 24, 125, 160, COL_PANEL);

    font_draw_text(190, 31, "BUILD 2", COL_GOLD, 1);
    font_draw_text(190, 43, "PIECES CURSOR", COL_TEXT, 1);

    square_name(square, cursor_file, cursor_rank);
    sprintf(line, "CURSOR %s", square);
    font_draw_text(190, 61, line, COL_CURSOR, 1);

    piece = board_piece_at(board, cursor_file, cursor_rank);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        font_draw_text(190, 73, board_piece_color_name(piece->color), COL_TEXT, 1);
        font_draw_text(190, 83, board_piece_type_name(piece->type), COL_TEXT, 1);
    } else {
        font_draw_text(190, 73, "EMPTY SQUARE", COL_MUTED, 1);
    }

    if (has_selection) {
        square_name(square, selected_file, selected_rank);
        sprintf(line, "SELECT %s", square);
        font_draw_text(190, 103, line, COL_SELECTED, 1);
        piece = board_piece_at(board, selected_file, selected_rank);
        if (piece != 0 && piece->type != CF_PIECE_NONE) {
            font_draw_text(190, 113, board_piece_type_name(piece->type), COL_MUTED, 1);
        }
    } else {
        font_draw_text(190, 103, "SELECT NONE", COL_MUTED, 1);
    }

    font_draw_text(190, 137, "ARROWS MOVE", COL_TEXT, 1);
    font_draw_text(190, 149, "ENTER SELECT", COL_TEXT, 1);
    font_draw_text(190, 169, "ESC QUITS", COL_GOLD, 1);
}

void board_view_render_build2(const CfBoard *board,
                              int cursor_file,
                              int cursor_rank,
                              int has_selection,
                              int selected_file,
                              int selected_rank)
{
    load_build2_palette();
    vga_clear(COL_BG);

    font_draw_text(10, 7, "CHESS FART", COL_TEXT, 1);
    font_draw_text(75, 7, "- CHECK MATE VENTILATE", COL_GOLD, 1);

    draw_board(board, cursor_file, cursor_rank,
               has_selection, selected_file, selected_rank);
    draw_panel(board, cursor_file, cursor_rank,
               has_selection, selected_file, selected_rank);

    font_draw_text(10, 190, "BUILD 2 - 32 PIECES ONLINE", COL_MUTED, 1);
}
