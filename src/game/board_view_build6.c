#include <stdio.h>

#include "board_view_build5.h"
#include "board_view_build6.h"
#include "font.h"
#include "vga.h"

#define BOARD_X 10
#define BOARD_Y 24
#define SQUARE_SIZE 20
#define COL_BG 0
#define COL_PANEL 1
#define COL_GOLD 4
#define COL_TEXT 5
#define COL_GREEN 7
#define COL_CHECK 20
#define COL_PROMOTE 21
#define COL_FART 22
#define COL_FART_PUSH 23

static void draw_outline(int x, int y, int size, cf_u8 color)
{
    vga_fill_rect(x, y, size, 2, color);
    vga_fill_rect(x, y + size - 2, size, 2, color);
    vga_fill_rect(x, y, 2, size, color);
    vga_fill_rect(x + size - 2, y, 2, size, color);
}

static int in_bounds(int file, int rank)
{
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

static void fart_delta(CfFartDirection direction, int *df, int *dr)
{
    static const int deltas[8][2] = {
        {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}
    };
    int index = (int)direction;
    if (index < 0 || index > 7) {
        *df = 0;
        *dr = 0;
        return;
    }
    *df = deltas[index][0];
    *dr = deltas[index][1];
}

static CfFartPreview build5_preview(CfFartPreview preview)
{
    if (preview == CF_FART_PUFF || preview == CF_FART_INVALID)
        return preview;
    return CF_FART_PUSH_BUILD6;
}

static void draw_push_geometry(int file, int rank,
                               CfFartDirection direction,
                               CfFartPreview preview)
{
    int df;
    int dr;
    int tf;
    int tr;
    int pf;
    int pr;
    int x;
    int y;
    cf_u8 color;

    if (preview == CF_FART_INVALID || preview == CF_FART_PUFF) return;
    fart_delta(direction, &df, &dr);
    tf = file + df;
    tr = rank + dr;
    pf = tf + df;
    pr = tr + dr;
    if (!in_bounds(tf, tr)) return;

    x = BOARD_X + tf * SQUARE_SIZE;
    y = BOARD_Y + (7 - tr) * SQUARE_SIZE;
    draw_outline(x + 3, y + 3, 14, COL_FART_PUSH);

    if (!in_bounds(pf, pr)) return;
    x = BOARD_X + pf * SQUARE_SIZE;
    y = BOARD_Y + (7 - pr) * SQUARE_SIZE;
    if (preview == CF_FART_PUSH) color = COL_GREEN;
    else if (preview == CF_FART_PROMOTION) color = COL_PROMOTE;
    else color = COL_CHECK;
    draw_outline(x + 5, y + 5, 10, color);
}

void board_view_render_build6(const CfBoard *board,
                              const CfGasState *gas,
                              int cursor_file,
                              int cursor_rank,
                              int has_selection,
                              int selected_file,
                              int selected_rank,
                              const CfMoveList *legal_moves,
                              CfGameStatus status,
                              int promotion_pending,
                              CfPieceType promotion_choice,
                              int fart_mode,
                              CfFartDirection fart_direction,
                              CfFartPreview fart_preview,
                              int fart_promotion_pending,
                              CfPieceType fart_promotion_choice,
                              const char *message)
{
    char line[24];
    CfFartPreview base_preview;

    base_preview = build5_preview(fart_preview);
    board_view_render_build5(board, gas, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             legal_moves, status, promotion_pending,
                             promotion_choice, fart_mode, fart_direction,
                             base_preview, message);

    vga_fill_rect(188, 27, 116, 10, COL_PANEL);
    font_draw_text(190, 29, "BUILD 6", COL_GOLD, 1);

    if (has_selection && fart_mode)
        draw_push_geometry(selected_file, selected_rank,
                           fart_direction, fart_preview);

    if (fart_mode || fart_promotion_pending) {
        vga_fill_rect(188, 94, 116, 38, COL_PANEL);
        sprintf(line, "FART %s", gas_direction_name(fart_direction));
        font_draw_text(190, 96, line, COL_FART, 1);
        if (fart_promotion_pending) {
            sprintf(line, "PROMOTE %s",
                    board_piece_type_name(fart_promotion_choice));
            font_draw_text(190, 107, line, COL_PROMOTE, 1);
        } else {
            sprintf(line, "%s READY", gas_fart_preview_name(fart_preview));
            font_draw_text(190, 107, line,
                           fart_preview == CF_FART_INVALID ? COL_CHECK :
                           (fart_preview == CF_FART_PROMOTION ? COL_PROMOTE :
                           (fart_preview == CF_FART_PUSH ? COL_GREEN : COL_FART_PUSH)), 1);
        }
        if (message != 0 && message[0] != '\0')
            font_draw_text(190, 119, message, COL_GOLD, 1);
    }

    vga_fill_rect(8, 188, 304, 10, COL_BG);
    font_draw_text(10, 190, "BUILD 6 - FART PHYSICS ONLINE", COL_TEXT, 1);
}
