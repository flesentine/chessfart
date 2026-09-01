#include <stdio.h>
#include <string.h>

#include "board_view_build7.h"
#include "font.h"
#include "ui_assets.h"
#include "vga.h"

#define BOARD_X 18
#define BOARD_Y 27
#define SQUARE_SIZE 18
#define BOARD_PIXELS (SQUARE_SIZE * 8)
#define PANEL_X 170
#define PANEL_Y 24
#define PANEL_W 143
#define PANEL_H 151
#define COMMAND_Y 181

#define COL_BG 0
#define COL_PANEL 1
#define COL_SQUARE_LIGHT 2
#define COL_SQUARE_DARK 3
#define COL_GOLD 4
#define COL_TEXT 5
#define COL_MUTED 6
#define COL_GREEN 7
#define COL_GAS 8
#define COL_BLACK 9
#define COL_PANEL_EDGE 10
#define COL_WHITE_PIECE 11
#define COL_WHITE_HI 12
#define COL_BLACK_PIECE 13
#define COL_BLACK_HI 14
#define COL_CURSOR 15
#define COL_SELECTED 16
#define COL_SHADOW 17
#define COL_LEGAL 18
#define COL_CAPTURE 19
#define COL_CHECK 20
#define COL_PROMOTE 21
#define COL_FART 22
#define COL_FART_PUSH 23
#define COL_SKY 24
#define COL_CLOUD 25
#define COL_FLASH 26
#define COL_COPPER 27
#define COL_PANEL_LINE 28
#define COL_PANEL_SOFT 29

static void set_rgb(cf_u8 *palette, int index, int r, int g, int b)
{
    int base = index * 3;
    palette[base] = (cf_u8)r;
    palette[base + 1] = (cf_u8)g;
    palette[base + 2] = (cf_u8)b;
}

static void load_art_palette(int flash)
{
    cf_u8 palette[VGA_PALETTE_BYTES];
    int i;
    int shade;

    for (i = 0; i < VGA_PALETTE_BYTES; ++i) palette[i] = 0;
    set_rgb(palette, COL_BG, 2, 5, 12);
    set_rgb(palette, COL_PANEL, 4, 10, 23);
    set_rgb(palette, COL_SQUARE_LIGHT, 50, 42, 25);
    set_rgb(palette, COL_SQUARE_DARK, 6, 29, 27);
    set_rgb(palette, COL_GOLD, flash ? 63 : 55, flash ? 63 : 43, 9);
    set_rgb(palette, COL_TEXT, 60, 57, 46);
    set_rgb(palette, COL_MUTED, 26, 28, 32);
    set_rgb(palette, COL_GREEN, 15, 44, 24);
    set_rgb(palette, COL_GAS, flash ? 63 : 38, 56, flash ? 35 : 12);
    set_rgb(palette, COL_BLACK, 0, 0, 0);
    set_rgb(palette, COL_PANEL_EDGE, 14, 19, 31);
    set_rgb(palette, COL_WHITE_PIECE, 55, 50, 38);
    set_rgb(palette, COL_WHITE_HI, 63, 61, 52);
    set_rgb(palette, COL_BLACK_PIECE, 5, 9, 15);
    set_rgb(palette, COL_BLACK_HI, 25, 32, 39);
    set_rgb(palette, COL_CURSOR, 8, 49, 57);
    set_rgb(palette, COL_SELECTED, 10, 57, 50);
    set_rgb(palette, COL_SHADOW, 1, 2, 4);
    set_rgb(palette, COL_LEGAL, 43, 40, 8);
    set_rgb(palette, COL_CAPTURE, 57, 23, 8);
    set_rgb(palette, COL_CHECK, 61, 10, 10);
    set_rgb(palette, COL_PROMOTE, 48, 23, 56);
    set_rgb(palette, COL_FART, flash ? 63 : 34, 55, 11);
    set_rgb(palette, COL_FART_PUSH, 55, 36, 8);
    set_rgb(palette, COL_SKY, 3, 10, 25);
    set_rgb(palette, COL_CLOUD, 27, 47, 13);
    set_rgb(palette, COL_FLASH, 63, 63, 40);
    set_rgb(palette, COL_COPPER, 42, 26, 11);
    set_rgb(palette, COL_PANEL_LINE, 25, 31, 42);
    set_rgb(palette, COL_PANEL_SOFT, 9, 15, 29);

    for (i = 30; i < VGA_PALETTE_COLORS; ++i) {
        shade = (i - 30) * 63 / (VGA_PALETTE_COLORS - 31);
        set_rgb(palette, i, shade, shade, shade);
    }
    vga_set_palette(palette);
}

static void draw_rect_outline(int x, int y, int w, int h, cf_u8 color)
{
    vga_fill_rect(x, y, w, 1, color);
    vga_fill_rect(x, y + h - 1, w, 1, color);
    vga_fill_rect(x, y, 1, h, color);
    vga_fill_rect(x + w - 1, y, 1, h, color);
}

static void draw_bevel_box(int x, int y, int w, int h,
                           cf_u8 fill, cf_u8 edge, cf_u8 accent)
{
    vga_fill_rect(x, y, w, h, edge);
    vga_fill_rect(x + 2, y + 2, w - 4, h - 4, fill);
    vga_fill_rect(x + 2, y + 2, w - 4, 1, accent);
    vga_fill_rect(x + 2, y + h - 3, w - 4, 1, COL_SHADOW);
}

static cf_u8 square_color(int file, int rank)
{
    int screen_rank = 7 - rank;
    return ((file + screen_rank) & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT;
}

static int move_index_at(const CfMoveList *list, int file, int rank)
{
    int i;
    if (list == 0) return -1;
    for (i = 0; i < list->count; ++i) {
        if (list->moves[i].to_file == file && list->moves[i].to_rank == rank)
            return i;
    }
    return -1;
}

static void draw_gas_ticks(int x, int y, cf_u8 gas)
{
    vga_fill_rect(x + 2, y + 16, 3, 1, gas >= 1U ? COL_GAS : COL_MUTED);
    vga_fill_rect(x + 7, y + 16, 3, 1, gas >= 2U ? COL_GAS : COL_MUTED);
    vga_fill_rect(x + 12, y + 16, 3, 1, gas >= 3U ? COL_GAS : COL_MUTED);
}

static void draw_board(const CfBoard *board, const CfGasState *gas,
                       int cursor_file, int cursor_rank,
                       int has_selection, int selected_file, int selected_rank,
                       const CfMoveList *legal_moves)
{
    int file;
    int rank;
    int screen_rank;
    int x;
    int y;
    int mi;
    char label[2];
    const CfPiece *piece;
    cf_u8 color;

    draw_bevel_box(14, 23, 152, 154, COL_BG, COL_COPPER, COL_GOLD);
    vga_fill_rect(BOARD_X, BOARD_Y, BOARD_PIXELS, BOARD_PIXELS, COL_BLACK);

    for (rank = 0; rank < 8; ++rank) {
        screen_rank = 7 - rank;
        for (file = 0; file < 8; ++file) {
            x = BOARD_X + file * SQUARE_SIZE;
            y = BOARD_Y + screen_rank * SQUARE_SIZE;
            color = square_color(file, rank);
            vga_fill_rect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);

            mi = move_index_at(legal_moves, file, rank);
            if (mi >= 0) {
                if (legal_moves->moves[mi].captured.type != CF_PIECE_NONE)
                    draw_rect_outline(x + 3, y + 3, 12, 12, COL_CAPTURE);
                else {
                    vga_fill_rect(x + 7, y + 7, 4, 4, COL_LEGAL);
                    vga_fill_rect(x + 8, y + 6, 2, 6, COL_LEGAL);
                    vga_fill_rect(x + 6, y + 8, 6, 2, COL_LEGAL);
                }
            }

            piece = board_piece_at(board, file, rank);
            if (piece != 0 && piece->type != CF_PIECE_NONE) {
                ui_assets_draw_piece(x, y + 1, piece, color);
                draw_gas_ticks(x, y, gas_at(gas, file, rank));
            }
        }
    }

    if (has_selection) {
        x = BOARD_X + selected_file * SQUARE_SIZE;
        y = BOARD_Y + (7 - selected_rank) * SQUARE_SIZE;
        draw_rect_outline(x + 1, y + 1, 16, 16, COL_SELECTED);
        draw_rect_outline(x + 2, y + 2, 14, 14, COL_SELECTED);
    }

    x = BOARD_X + cursor_file * SQUARE_SIZE;
    y = BOARD_Y + (7 - cursor_rank) * SQUARE_SIZE;
    draw_rect_outline(x, y, SQUARE_SIZE, SQUARE_SIZE, COL_CURSOR);

    label[1] = '\0';
    for (file = 0; file < 8; ++file) {
        label[0] = (char)('A' + file);
        font_draw_text(24 + file * SQUARE_SIZE, 172, label, COL_GOLD, 1);
    }
    for (screen_rank = 0; screen_rank < 8; ++screen_rank) {
        label[0] = (char)('8' - screen_rank);
        font_draw_text(7, 32 + screen_rank * SQUARE_SIZE, label, COL_GOLD, 1);
    }
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
    if (tf < 0 || tf > 7 || tr < 0 || tr > 7) return;

    x = BOARD_X + tf * SQUARE_SIZE;
    y = BOARD_Y + (7 - tr) * SQUARE_SIZE;
    draw_rect_outline(x + 2, y + 2, 14, 14, COL_FART_PUSH);
    if (pf < 0 || pf > 7 || pr < 0 || pr > 7) return;

    x = BOARD_X + pf * SQUARE_SIZE;
    y = BOARD_Y + (7 - pr) * SQUARE_SIZE;
    color = preview == CF_FART_PUSH ? COL_GREEN :
            (preview == CF_FART_PROMOTION ? COL_PROMOTE : COL_CHECK);
    draw_rect_outline(x + 5, y + 5, 8, 8, color);
}

static void square_name(char *out, int file, int rank)
{
    if (file < 0 || rank < 0) {
        out[0] = '-';
        out[1] = '-';
        out[2] = '\0';
        return;
    }
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

static void draw_gas_pips(int x, int y, cf_u8 gas)
{
    int i;
    for (i = 0; i < 3; ++i) {
        cf_u8 fill = gas > (cf_u8)i ? COL_GAS : COL_PANEL_LINE;
        vga_fill_rect(x + i * 15, y, 9, 7, COL_SHADOW);
        vga_fill_rect(x + 1 + i * 15, y + 1, 7, 5, fill);
        if (gas > (cf_u8)i)
            vga_fill_rect(x + 2 + i * 15, y + 2, 3, 2, COL_GREEN);
    }
}

static void draw_panel_art(const CfBoard *board, const CfGasState *gas,
                           int cursor_file, int cursor_rank,
                           int has_selection, int selected_file, int selected_rank,
                           const CfMoveList *legal_moves, CfGameStatus status,
                           int promotion_pending, CfPieceType promotion_choice,
                           int fart_mode, CfFartDirection fart_direction,
                           CfFartPreview fart_preview, const char *message)
{
    const CfPiece *piece;
    int piece_file;
    int piece_rank;
    char square[3];
    char line[24];
    cf_u8 piece_gas = 0U;

    draw_bevel_box(PANEL_X, PANEL_Y, PANEL_W, PANEL_H,
                   COL_PANEL, COL_PANEL_EDGE, COL_COPPER);

    font_draw_text(180, 31, "TURN", COL_GOLD, 1);
    font_draw_text(223, 31, board_piece_color_name(board->side_to_move), COL_TEXT, 1);
    font_draw_text(180, 42, "STATE", COL_MUTED, 1);
    font_draw_text(223, 42, board_game_status_name(status),
                   status == CF_GAME_CHECK || status == CF_GAME_CHECKMATE ?
                   COL_CHECK : COL_GREEN, 1);
    vga_fill_rect(180, 52, 123, 1, COL_PANEL_LINE);

    piece_file = has_selection ? selected_file : cursor_file;
    piece_rank = has_selection ? selected_rank : cursor_rank;
    piece = board_piece_at(board, piece_file, piece_rank);
    square_name(square, piece_file, piece_rank);

    font_draw_text(180, 58, has_selection ? "SELECTED PIECE" : "CURSOR PIECE",
                   COL_SELECTED, 1);
    vga_fill_rect(181, 68, 22, 22, COL_PANEL_EDGE);
    vga_fill_rect(183, 70, 18, 18, COL_PANEL_SOFT);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        ui_assets_draw_piece(183, 71, piece, COL_PANEL_SOFT);
        font_draw_text(208, 69, board_piece_type_name(piece->type), COL_TEXT, 1);
        font_draw_text(208, 80, square, COL_CURSOR, 1);
        piece_gas = gas_at(gas, piece_file, piece_rank);
    } else {
        font_draw_text(208, 69, "EMPTY", COL_MUTED, 1);
        font_draw_text(208, 80, square, COL_CURSOR, 1);
    }

    vga_fill_rect(180, 94, 123, 1, COL_PANEL_LINE);
    font_draw_text(180, 99, "GAS RESERVE", COL_GOLD, 1);
    draw_gas_pips(181, 110, piece_gas);
    sprintf(line, "%u/3", (unsigned)piece_gas);
    font_draw_text(232, 110, line, piece_gas >= 2U ? COL_GAS : COL_MUTED, 1);
    vga_fill_rect(180, 121, 123, 1, COL_PANEL_LINE);

    if (promotion_pending) {
        sprintf(line, "PROMOTE %s", board_piece_type_name(promotion_choice));
        font_draw_text(180, 127, line, COL_PROMOTE, 1);
    } else if (fart_mode) {
        sprintf(line, "FART %s", gas_direction_name(fart_direction));
        font_draw_text(180, 127, line, COL_FART, 1);
        font_draw_text(180, 138, gas_fart_preview_name(fart_preview),
                       fart_preview == CF_FART_INVALID ? COL_CHECK : COL_GAS, 1);
    } else if (has_selection) {
        sprintf(line, "%d LEGAL MOVES", legal_moves != 0 ? legal_moves->count : 0);
        font_draw_text(180, 127, line, COL_MUTED, 1);
        font_draw_text(180, 138, piece_gas >= 2U ? "FART READY" : "BUILD GAS",
                       piece_gas >= 2U ? COL_GAS : COL_MUTED, 1);
    } else {
        font_draw_text(180, 127, "READY", COL_GREEN, 1);
        if (message != 0 && message[0] != '\0') {
            strncpy(line, message, sizeof(line) - 1U);
            line[sizeof(line) - 1U] = '\0';
            font_draw_text(180, 138, line, COL_MUTED, 1);
        }
    }
}

static void draw_header(void)
{
    vga_fill_rect(0, 0, 320, 21, COL_BG);
    vga_fill_rect(0, 20, 320, 1, COL_COPPER);
    font_draw_text(12, 4, "CHESS FART", COL_GOLD, 2);
    font_draw_text(139, 7, "CHECK. MATE. VENTILATE.", COL_TEXT, 1);
}

static void draw_command_bar(void)
{
    static const char *labels[8] = {
        "F FART", "S SAVE", "L LOAD", "H HELP",
        "M LOG", "C CRED", "D CPU", "ESC"
    };
    int i;
    int x;

    vga_fill_rect(0, COMMAND_Y, 320, 19, COL_BG);
    vga_fill_rect(0, COMMAND_Y, 320, 1, COL_COPPER);
    for (i = 0; i < 8; ++i) {
        x = i * 40;
        vga_fill_rect(x + 1, COMMAND_Y + 2, 38, 15, COL_PANEL_EDGE);
        vga_fill_rect(x + 2, COMMAND_Y + 3, 36, 13, COL_PANEL_SOFT);
        font_draw_text(x + 3, COMMAND_Y + 6, labels[i],
                       i == 0 ? COL_GAS : COL_TEXT, 1);
    }
}

static void draw_fx(const CfPresentationFx *fx)
{
    int df;
    int dr;
    int ax;
    int ay;
    int px;
    int py;
    int jitter;
    int frame;

    if (fx == 0 || !fx->active) return;
    fart_delta(fx->action.direction, &df, &dr);
    ax = BOARD_X + fx->action.actor_file * SQUARE_SIZE + 9;
    ay = BOARD_Y + (7 - fx->action.actor_rank) * SQUARE_SIZE + 9;
    frame = fx->frame;
    if (frame < 0) frame = 0;
    if (frame > 4) frame = 4;
    jitter = (frame & 1) ? 1 : -1;
    px = ax + df * (5 + frame * 3) - 8;
    py = ay - dr * (5 + frame * 3) - 8;
    ui_assets_draw_puff(px, py, frame);

    if (fx->action.result == CF_FART_PUSH ||
        fx->action.result == CF_FART_PROMOTION) {
        if (fx->action.destination_file >= 0 && fx->action.destination_rank >= 0) {
            px = BOARD_X + fx->action.destination_file * SQUARE_SIZE;
            py = BOARD_Y + (7 - fx->action.destination_rank) * SQUARE_SIZE;
            draw_rect_outline(px + 2 + jitter, py + 2, 14, 14,
                              frame == 2 ? COL_FLASH : COL_FART_PUSH);
        }
    }
    draw_rect_outline(14 + jitter, 23, 152, 154,
                      frame == 2 ? COL_FLASH : COL_COPPER);
}

void board_view_render_build7_fx(const CfBoard *board,
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
                                 const char *message,
                                 const CfPresentationFx *fx)
{
    int flash = fx != 0 && fx->active && fx->frame == 2;

    (void)fart_promotion_pending;
    (void)fart_promotion_choice;
    load_art_palette(flash);
    vga_clear(COL_BG);
    draw_header();
    draw_board(board, gas, cursor_file, cursor_rank,
               has_selection, selected_file, selected_rank, legal_moves);
    if (has_selection && fart_mode) {
        draw_push_geometry(selected_file, selected_rank,
                           fart_direction, fart_preview);
    }
    draw_panel_art(board, gas, cursor_file, cursor_rank,
                   has_selection, selected_file, selected_rank,
                   legal_moves, status, promotion_pending,
                   promotion_choice, fart_mode, fart_direction,
                   fart_preview, message);
    draw_command_bar();
    draw_fx(fx);
}

void board_view_render_build7(const CfBoard *board,
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
    board_view_render_build7_fx(board, gas, cursor_file, cursor_rank,
                                has_selection, selected_file, selected_rank,
                                legal_moves, status, promotion_pending,
                                promotion_choice, fart_mode, fart_direction,
                                fart_preview, fart_promotion_pending,
                                fart_promotion_choice, message, 0);
}

void board_view_render_title7(int menu_index, int frame)
{
    int phase = frame & 1;

    (void)menu_index;
    load_art_palette(0);
    vga_clear(COL_BG);
    draw_rect_outline(4, 4, 312, 192, COL_PANEL_EDGE);
    draw_rect_outline(6, 6, 308, 188, COL_COPPER);
    font_draw_text(100, 19, "CHESS FART", COL_GOLD, 2);
    font_draw_text(88, 42, "CHECK. MATE. VENTILATE.", COL_SELECTED, 1);
    vga_fill_rect(52, 60, 216, 1, COL_PANEL_LINE);
    vga_fill_rect(52, 63, 216, 1, COL_COPPER);
    ui_assets_draw_puff(63 + phase, 31, 1);
    ui_assets_draw_puff(241 - phase, 31, 1);
    font_draw_text(111, 76, "386 VGA EDITION", COL_MUTED, 1);
}
