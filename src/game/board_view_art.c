#include <stdio.h>
#include <string.h>

#include "board_view_build6.h"
#include "board_view_build7.h"
#include "font.h"
#include "ui_assets.h"
#include "vga.h"

#define BOARD_X 10
#define BOARD_Y 24
#define SQUARE_SIZE 20

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
    set_rgb(palette, COL_BG, 2, 4, 10);
    set_rgb(palette, COL_PANEL, 4, 8, 19);
    set_rgb(palette, COL_SQUARE_LIGHT, 54, 44, 25);
    set_rgb(palette, COL_SQUARE_DARK, 7, 30, 27);
    set_rgb(palette, COL_GOLD, flash ? 63 : 61, flash ? 63 : 43, 6);
    set_rgb(palette, COL_TEXT, 63, 61, 50);
    set_rgb(palette, COL_MUTED, 30, 31, 34);
    set_rgb(palette, COL_GREEN, 15, 49, 20);
    set_rgb(palette, COL_GAS, flash ? 63 : 39, 63, flash ? 35 : 7);
    set_rgb(palette, COL_BLACK, 0, 0, 0);
    set_rgb(palette, COL_PANEL_EDGE, 17, 22, 35);
    set_rgb(palette, COL_WHITE_PIECE, 57, 50, 33);
    set_rgb(palette, COL_WHITE_HI, 63, 61, 49);
    set_rgb(palette, COL_BLACK_PIECE, 7, 10, 15);
    set_rgb(palette, COL_BLACK_HI, 28, 33, 40);
    set_rgb(palette, COL_CURSOR, 5, 55, 63);
    set_rgb(palette, COL_SELECTED, 12, 63, 56);
    set_rgb(palette, COL_SHADOW, 1, 2, 4);
    set_rgb(palette, COL_LEGAL, 22, 63, 26);
    set_rgb(palette, COL_CAPTURE, 63, 24, 7);
    set_rgb(palette, COL_CHECK, 63, 8, 8);
    set_rgb(palette, COL_PROMOTE, 55, 25, 63);
    set_rgb(palette, COL_FART, flash ? 63 : 36, 63, 7);
    set_rgb(palette, COL_FART_PUSH, 63, 39, 5);
    set_rgb(palette, COL_SKY, 3, 11, 27);
    set_rgb(palette, COL_CLOUD, 28, 51, 10);
    set_rgb(palette, COL_FLASH, 63, 63, 39);
    set_rgb(palette, COL_COPPER, 47, 24, 7);

    for (i = 28; i < VGA_PALETTE_COLORS; ++i) {
        shade = (i - 28) * 63 / (VGA_PALETTE_COLORS - 29);
        set_rgb(palette, i, shade, shade, shade);
    }
    vga_set_palette(palette);
}

static void draw_outline(int x, int y, int size, cf_u8 color)
{
    vga_fill_rect(x, y, size, 1, color);
    vga_fill_rect(x, y + size - 1, size, 1, color);
    vga_fill_rect(x, y, 1, size, color);
    vga_fill_rect(x + size - 1, y, 1, size, color);
}

static cf_u8 square_color(int file, int rank)
{
    int screen_rank = 7 - rank;
    return ((file + screen_rank) & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT;
}

static void draw_piece_art(int x, int y, const CfPiece *piece,
                           cf_u8 gas, cf_u8 background)
{
    ui_assets_draw_piece(x + 1, y + 1, piece, background);
    vga_fill_rect(x + 3, y + 17, 4, 2, gas >= 1U ? COL_GAS : COL_MUTED);
    vga_fill_rect(x + 8, y + 17, 4, 2, gas >= 2U ? COL_GAS : COL_MUTED);
    vga_fill_rect(x + 13, y + 17, 4, 2, gas >= 3U ? COL_GAS : COL_MUTED);
}

static int move_index_at(const CfMoveList *list, int file, int rank)
{
    int i;
    if (list == 0) return -1;
    for (i = 0; i < list->count; ++i)
        if (list->moves[i].to_file == file && list->moves[i].to_rank == rank)
            return i;
    return -1;
}

static void redraw_pieces_and_hints(const CfBoard *board,
                                    const CfGasState *gas,
                                    const CfMoveList *legal_moves)
{
    int file;
    int rank;
    int x;
    int y;
    int mi;
    const CfPiece *piece;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            piece = board_piece_at(board, file, rank);
            if (piece == 0 || piece->type == CF_PIECE_NONE) continue;
            x = BOARD_X + file * SQUARE_SIZE;
            y = BOARD_Y + (7 - rank) * SQUARE_SIZE;
            draw_piece_art(x, y, piece, gas_at(gas, file, rank),
                           square_color(file, rank));
        }
    }

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            mi = move_index_at(legal_moves, file, rank);
            if (mi < 0) continue;
            x = BOARD_X + file * SQUARE_SIZE;
            y = BOARD_Y + (7 - rank) * SQUARE_SIZE;
            if (legal_moves->moves[mi].captured.type != CF_PIECE_NONE)
                draw_outline(x + 4, y + 4, 12, COL_CAPTURE);
            else
                vga_fill_rect(x + 8, y + 8, 4, 4, COL_LEGAL);
        }
    }
}

static void fart_delta(CfFartDirection direction, int *df, int *dr)
{
    static const int deltas[8][2] = {
        {0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1}
    };
    int index = (int)direction;
    if (index < 0 || index > 7) { *df = 0; *dr = 0; return; }
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
    draw_outline(x + 2, y + 2, 16, COL_FART_PUSH);
    if (pf < 0 || pf > 7 || pr < 0 || pr > 7) return;
    x = BOARD_X + pf * SQUARE_SIZE;
    y = BOARD_Y + (7 - pr) * SQUARE_SIZE;
    color = preview == CF_FART_PUSH ? COL_GREEN :
            (preview == CF_FART_PROMOTION ? COL_PROMOTE : COL_CHECK);
    draw_outline(x + 5, y + 5, 10, color);
}

static void square_name(char *out, int file, int rank)
{
    if (file < 0 || rank < 0) {
        out[0] = '-'; out[1] = '-'; out[2] = '\0'; return;
    }
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

static void castle_text(char *out, unsigned rights)
{
    int n = 0;
    out[n++] = 'W';
    if ((rights & CF_CASTLE_WHITE_KING) != 0U) out[n++] = 'K';
    if ((rights & CF_CASTLE_WHITE_QUEEN) != 0U) out[n++] = 'Q';
    out[n++] = ' ';
    out[n++] = 'B';
    if ((rights & CF_CASTLE_BLACK_KING) != 0U) out[n++] = 'K';
    if ((rights & CF_CASTLE_BLACK_QUEEN) != 0U) out[n++] = 'Q';
    out[n] = '\0';
}

static void draw_panel_art(const CfBoard *board, const CfGasState *gas,
                           int cursor_file, int cursor_rank,
                           int has_selection, int selected_file, int selected_rank,
                           const CfMoveList *legal_moves, CfGameStatus status,
                           int promotion_pending, CfPieceType promotion_choice,
                           int fart_mode, CfFartDirection fart_direction,
                           CfFartPreview fart_preview, const char *message)
{
    char square[3];
    char ep[3];
    char castles[12];
    char line[28];
    const CfPiece *piece;
    int gas_file = cursor_file;
    int gas_rank = cursor_rank;

    vga_fill_rect(181, 22, 129, 107, COL_PANEL_EDGE);
    vga_fill_rect(183, 24, 125, 103, COL_PANEL);
    vga_fill_rect(183, 24, 125, 2, COL_COPPER);
    vga_fill_rect(183, 125, 125, 2, COL_COPPER);

    font_draw_text(190, 29, "STATUS", COL_GOLD, 1);
    font_draw_text(190, 40, "TURN", COL_GOLD, 1);
    font_draw_text(242, 40, board_piece_color_name(board->side_to_move), COL_TEXT, 1);
    font_draw_text(190, 50, "STATE", COL_GOLD, 1);
    font_draw_text(232, 50, board_game_status_name(status),
                   status == CF_GAME_CHECK || status == CF_GAME_CHECKMATE ? COL_CHECK : COL_GREEN, 1);
    vga_fill_rect(190, 60, 110, 1, COL_MUTED);

    square_name(square, cursor_file, cursor_rank);
    font_draw_text(190, 64, "CURSOR", COL_CURSOR, 1);
    font_draw_text(244, 64, square, COL_CURSOR, 1);
    piece = board_piece_at(board, cursor_file, cursor_rank);
    if (has_selection) {
        piece = board_piece_at(board, selected_file, selected_rank);
        gas_file = selected_file;
        gas_rank = selected_rank;
    }
    font_draw_text(190, 74, "PIECE", COL_GOLD, 1);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        sprintf(line, "%s %s", board_piece_color_name(piece->color),
                board_piece_type_name(piece->type));
        font_draw_text(228, 74, line, COL_TEXT, 1);
        sprintf(line, "%u/3", (unsigned)gas_at(gas, gas_file, gas_rank));
        font_draw_text(244, 84, line, COL_GAS, 1);
    } else {
        font_draw_text(228, 74, "--", COL_MUTED, 1);
        font_draw_text(244, 84, "0/3", COL_MUTED, 1);
    }
    font_draw_text(190, 84, "GAS", COL_GOLD, 1);
    vga_fill_rect(190, 94, 110, 1, COL_MUTED);

    castle_text(castles, board->castling_rights);
    font_draw_text(190, 98, "CASTLE", COL_MUTED, 1);
    font_draw_text(238, 98, castles, COL_MUTED, 1);
    square_name(ep, board->en_passant_file, board->en_passant_rank);
    font_draw_text(190, 108, "EP", COL_MUTED, 1);
    font_draw_text(238, 108, ep, COL_MUTED, 1);

    if (promotion_pending) {
        sprintf(line, "PROMOTE %s", board_piece_type_name(promotion_choice));
    } else if (fart_mode) {
        sprintf(line, "FART %s %s", gas_direction_name(fart_direction),
                gas_fart_preview_name(fart_preview));
    } else if (has_selection) {
        sprintf(line, "%d MOVES  F %s", legal_moves != 0 ? legal_moves->count : 0,
                gas_at(gas, selected_file, selected_rank) >= 2U ? "READY" : "NO");
    } else if (message != 0 && message[0] != '\0') {
        strncpy(line, message, sizeof(line) - 1U);
        line[sizeof(line) - 1U] = '\0';
    } else {
        strcpy(line, "READY");
    }
    font_draw_text(190, 118, line, fart_mode ? COL_FART : COL_GOLD, 1);
}

static void draw_frame_polish(void)
{
    int i;
    char file_label[2];
    char rank_label[2];

    vga_fill_rect(7, 21, 166, 2, COL_COPPER);
    vga_fill_rect(7, 185, 166, 2, COL_COPPER);
    vga_fill_rect(7, 21, 2, 166, COL_COPPER);
    vga_fill_rect(171, 21, 2, 166, COL_COPPER);
    vga_fill_rect(9, 23, 162, 1, COL_GOLD);

    file_label[1] = '\0';
    rank_label[1] = '\0';
    for (i = 0; i < 8; ++i) {
        file_label[0] = (char)('A' + i);
        rank_label[0] = (char)('8' - i);
        font_draw_text(17 + i * 20, 181, file_label, COL_GOLD, 1);
        font_draw_text(2, 31 + i * 20, rank_label, COL_GOLD, 1);
    }
}

static void draw_header_footer(void)
{
    vga_fill_rect(0, 0, 320, 20, COL_BG);
    font_draw_text(10, 5, "CHESS FART", COL_GOLD, 2);
    font_draw_text(136, 7, "CHECK. MATE. VENTILATE.", COL_TEXT, 1);
    vga_fill_rect(0, 188, 320, 12, COL_BG);
    font_draw_text(8, 190, "F FART  S SAVE  L LOAD  H HELP  M LOG  C CREDITS", COL_MUTED, 1);
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
    ax = BOARD_X + fx->action.actor_file * SQUARE_SIZE + 10;
    ay = BOARD_Y + (7 - fx->action.actor_rank) * SQUARE_SIZE + 10;
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
            draw_outline(px + 2 + jitter, py + 2, 16,
                         frame == 2 ? COL_FLASH : COL_FART_PUSH);
        }
    }
    draw_outline(7 + jitter, 21, 166, frame == 2 ? COL_FLASH : COL_COPPER);
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

    board_view_render_build6(board, gas, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             legal_moves, status, promotion_pending,
                             promotion_choice, fart_mode, fart_direction,
                             fart_preview, fart_promotion_pending,
                             fart_promotion_choice, message);
    load_art_palette(flash);
    redraw_pieces_and_hints(board, gas, legal_moves);
    if (has_selection && fart_mode)
        draw_push_geometry(selected_file, selected_rank,
                           fart_direction, fart_preview);
    draw_frame_polish();
    draw_panel_art(board, gas, cursor_file, cursor_rank,
                   has_selection, selected_file, selected_rank,
                   legal_moves, status, promotion_pending,
                   promotion_choice, fart_mode, fart_direction,
                   fart_preview, message);
    draw_header_footer();
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
    int i;
    int phase = frame & 1;
    int x;

    (void)menu_index;
    load_art_palette(0);
    vga_clear(COL_BG);
    vga_fill_rect(0, 0, 320, 22, COL_SKY);
    vga_fill_rect(0, 178, 320, 22, COL_SKY);
    vga_fill_rect(18, 24, 284, 68, COL_PANEL_EDGE);
    vga_fill_rect(20, 26, 280, 64, COL_PANEL);
    vga_fill_rect(20, 26, 280, 2, COL_COPPER);
    vga_fill_rect(20, 88, 280, 2, COL_COPPER);

    ui_assets_draw_puff(28 + phase, 39, 2);
    ui_assets_draw_puff(274 - phase, 39, 2);
    font_draw_text(67, 36, "CHESS FART", COL_GOLD, 3);
    font_draw_text(87, 72, "CHECK. MATE. VENTILATE.", COL_TEXT, 1);

    for (i = 0; i < 16; ++i) {
        x = i * 20;
        vga_fill_rect(x, 180, 20, 18,
                      (i & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT);
    }
    font_draw_text(102, 184, "VGA / 386 EDITION", COL_GOLD, 1);
}
