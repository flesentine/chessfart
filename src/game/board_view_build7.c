#include <stdio.h>
#include <string.h>

#include "board_view_build6.h"
#include "board_view_build7.h"
#include "font.h"
#include "ui_theme.h"
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

static void load_build7_palette(int flash)
{
    cf_u8 palette[VGA_PALETTE_BYTES];
    ui_theme_build_palette(palette, flash);
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

static void draw_piece_shape7(int x, int y, CfPieceType type,
                              cf_u8 body, cf_u8 hi, cf_u8 cut)
{
    switch (type) {
    case CF_PIECE_PAWN:
        vga_fill_rect(x + 6, y + 2, 6, 2, body);
        vga_fill_rect(x + 5, y + 4, 8, 4, body);
        vga_fill_rect(x + 7, y + 3, 3, 2, hi);
        vga_fill_rect(x + 6, y + 8, 6, 5, body);
        vga_fill_rect(x + 4, y + 13, 10, 2, body);
        break;
    case CF_PIECE_KNIGHT:
        vga_fill_rect(x + 5, y + 2, 8, 2, body);
        vga_fill_rect(x + 4, y + 4, 10, 3, body);
        vga_fill_rect(x + 6, y + 7, 8, 3, body);
        vga_fill_rect(x + 8, y + 10, 6, 3, body);
        vga_fill_rect(x + 4, y + 13, 11, 2, body);
        vga_fill_rect(x + 4, y + 4, 2, 5, body);
        vga_fill_rect(x + 7, y + 4, 2, 2, cut);
        vga_put_pixel(x + 11, y + 4, hi);
        break;
    case CF_PIECE_BISHOP:
        vga_fill_rect(x + 8, y + 1, 2, 2, hi);
        vga_fill_rect(x + 6, y + 3, 6, 3, body);
        vga_fill_rect(x + 5, y + 6, 8, 5, body);
        vga_fill_rect(x + 7, y + 11, 4, 2, body);
        vga_fill_rect(x + 4, y + 13, 10, 2, body);
        vga_fill_rect(x + 9, y + 4, 1, 5, cut);
        break;
    case CF_PIECE_ROOK:
        vga_fill_rect(x + 3, y + 2, 3, 4, body);
        vga_fill_rect(x + 8, y + 2, 3, 4, body);
        vga_fill_rect(x + 13, y + 2, 3, 4, body);
        vga_fill_rect(x + 4, y + 5, 11, 2, body);
        vga_fill_rect(x + 5, y + 7, 9, 6, body);
        vga_fill_rect(x + 3, y + 13, 13, 2, body);
        vga_fill_rect(x + 6, y + 7, 2, 5, hi);
        break;
    case CF_PIECE_QUEEN:
        vga_fill_rect(x + 3, y + 2, 3, 3, body);
        vga_fill_rect(x + 8, y + 1, 3, 4, body);
        vga_fill_rect(x + 14, y + 2, 3, 3, body);
        vga_fill_rect(x + 5, y + 5, 10, 2, body);
        vga_fill_rect(x + 6, y + 7, 8, 6, body);
        vga_fill_rect(x + 4, y + 13, 12, 2, body);
        vga_put_pixel(x + 9, y + 2, hi);
        break;
    case CF_PIECE_KING:
        vga_fill_rect(x + 8, y, 2, 5, body);
        vga_fill_rect(x + 6, y + 2, 6, 2, body);
        vga_fill_rect(x + 5, y + 5, 8, 3, body);
        vga_fill_rect(x + 6, y + 8, 6, 5, body);
        vga_fill_rect(x + 4, y + 13, 10, 2, body);
        vga_fill_rect(x + 7, y + 6, 2, 2, hi);
        break;
    default:
        break;
    }
}

static void draw_piece7(int x, int y, const CfPiece *piece,
                        cf_u8 gas, cf_u8 background)
{
    cf_u8 body;
    cf_u8 hi;

    if (piece == 0 || piece->type == CF_PIECE_NONE) return;
    body = piece->color == CF_COLOR_WHITE ? COL_WHITE_PIECE : COL_BLACK_PIECE;
    hi = piece->color == CF_COLOR_WHITE ? COL_WHITE_HI : COL_BLACK_HI;

    vga_fill_rect(x + 1, y + 1, 18, 16, background);
    draw_piece_shape7(x + 1, y + 1, piece->type, COL_SHADOW, COL_SHADOW, background);
    draw_piece_shape7(x, y, piece->type, body, hi, background);

    vga_fill_rect(x + 4, y + 17, 3, 2, gas >= 1U ? COL_GAS : COL_MUTED);
    vga_fill_rect(x + 9, y + 17, 3, 2, gas >= 2U ? COL_GAS : COL_MUTED);
    vga_fill_rect(x + 14, y + 17, 3, 2, gas >= 3U ? COL_GAS : COL_MUTED);
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
            draw_piece7(x, y, piece, gas_at(gas, file, rank),
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

static void draw_push_geometry7(int file, int rank,
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

static void draw_frame_polish(void)
{
    int i;
    char file_label[2];
    char rank_label[2];

    vga_fill_rect(7, 21, 166, 2, COL_COPPER);
    vga_fill_rect(7, 185, 166, 2, COL_COPPER);
    vga_fill_rect(7, 21, 2, 166, COL_COPPER);
    vga_fill_rect(171, 21, 2, 166, COL_COPPER);

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
    font_draw_text(11, 7, "CHESS FART", COL_SHADOW, 1);
    font_draw_text(10, 6, "CHESS FART", COL_TEXT, 1);
    font_draw_text(75, 6, "CHECK MATE VENTILATE", COL_GOLD, 1);
    vga_fill_rect(0, 188, 320, 12, COL_BG);
    font_draw_text(10, 190, "BUILD 7  VGA PRESENTATION PASS", COL_MUTED, 1);
}

static void draw_fx(const CfPresentationFx *fx)
{
    int df;
    int dr;
    int ax;
    int ay;
    int step;
    int px;
    int py;
    int jitter;
    cf_u8 color;

    if (fx == 0 || !fx->active) return;
    fart_delta(fx->action.direction, &df, &dr);
    ax = BOARD_X + fx->action.actor_file * SQUARE_SIZE + 10;
    ay = BOARD_Y + (7 - fx->action.actor_rank) * SQUARE_SIZE + 10;
    jitter = (fx->frame & 1) ? 1 : -1;
    color = fx->frame == 2 ? COL_FLASH : COL_FART;

    for (step = 1; step <= fx->frame + 2; ++step) {
        px = ax + df * step * 4 + jitter * (step & 1);
        py = ay - dr * step * 4 + jitter * ((step + 1) & 1);
        vga_fill_rect(px - 2, py - 2, 4, 4, color);
        if (step > 1) vga_fill_rect(px, py - 4, 2, 2, COL_CLOUD);
    }

    if (fx->action.result == CF_FART_PUSH ||
        fx->action.result == CF_FART_PROMOTION) {
        if (fx->action.destination_file >= 0 && fx->action.destination_rank >= 0) {
            px = BOARD_X + fx->action.destination_file * SQUARE_SIZE;
            py = BOARD_Y + (7 - fx->action.destination_rank) * SQUARE_SIZE;
            draw_outline(px + 2 + jitter, py + 2, 16, COL_FLASH);
        }
    }

    draw_outline(7 + jitter, 21, 166, fx->frame == 2 ? COL_FLASH : COL_COPPER);
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
    load_build7_palette(flash);
    redraw_pieces_and_hints(board, gas, legal_moves);
    if (has_selection && fart_mode)
        draw_push_geometry7(selected_file, selected_rank,
                            fart_direction, fart_preview);
    draw_frame_polish();
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

static void draw_cloud(int x, int y, int phase)
{
    vga_fill_rect(x + phase, y + 7, 34, 10, COL_CLOUD);
    vga_fill_rect(x + 6 + phase, y + 2, 16, 8, COL_CLOUD);
    vga_fill_rect(x + 20 + phase, y + 4, 18, 10, COL_CLOUD);
    vga_fill_rect(x + 12 + phase, y, 8, 5, COL_GAS);
    vga_fill_rect(x + 27 + phase, y + 1, 5, 5, COL_GAS);
}

void board_view_render_title7(int menu_index, int frame)
{
    int i;
    int x;
    int phase = frame & 1;

    load_build7_palette(0);
    vga_clear(COL_BG);
    vga_fill_rect(0, 0, 320, 28, COL_SKY);
    vga_fill_rect(0, 169, 320, 31, COL_SKY);

    for (i = 0; i < 16; ++i) {
        x = i * 20;
        vga_fill_rect(x, 172, 20, 20,
                      (i & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT);
    }

    draw_cloud(227, 34, phase);
    draw_cloud(18, 122, -phase);

    font_draw_text(42, 31, "CHESS", COL_SHADOW, 4);
    font_draw_text(38, 27, "CHESS", COL_TEXT, 4);
    font_draw_text(61, 66, "FART", COL_SHADOW, 4);
    font_draw_text(57, 62, "FART", COL_GAS, 4);
    font_draw_text(79, 101, "CHECK MATE VENTILATE", COL_GOLD, 1);

    vga_fill_rect(77, 119, 170, 40, COL_PANEL_EDGE);
    vga_fill_rect(79, 121, 166, 36, COL_PANEL);
    font_draw_text(91, 129,
                   menu_index == 0 ? "X PLAY FART CHESS" : "  PLAY FART CHESS",
                   menu_index == 0 ? COL_GAS : COL_TEXT, 1);
    font_draw_text(91, 143,
                   menu_index == 1 ? "X QUIT TO DOS" : "  QUIT TO DOS",
                   menu_index == 1 ? COL_CAPTURE : COL_TEXT, 1);

    font_draw_text(102, 181, "1992 SHAREWARE EDITION", COL_GOLD, 1);
    font_draw_text(10, 194, "BUILD 7 PRESENTATION PREVIEW", COL_MUTED, 1);
}
