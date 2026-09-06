#include <stdio.h>
#include <string.h>

#include "board_view_build7.h"
#include "font.h"
#include "ui_assets.h"
#include "ui_layout.h"
#include "ui_theme.h"
#include "vga.h"

/* Legacy local names are aliases only; canonical values live in ui_layout.h. */
#define BOARD_X CF_UI_BOARD_X
#define BOARD_Y CF_UI_BOARD_Y
#define SQUARE_SIZE CF_UI_SQUARE_SIZE
#define BOARD_PIXELS CF_UI_BOARD_PIXELS
#define BOARD_FRAME_X CF_UI_BOARD_FRAME_X
#define BOARD_FRAME_Y CF_UI_BOARD_FRAME_Y
#define BOARD_FRAME_W CF_UI_BOARD_FRAME_W
#define BOARD_FRAME_H CF_UI_BOARD_FRAME_H
#define PANEL_X CF_UI_PANEL_X
#define PANEL_Y CF_UI_PANEL_Y
#define PANEL_W CF_UI_PANEL_W
#define PANEL_H CF_UI_PANEL_H
#define COMMAND_Y CF_UI_COMMAND_Y

/* Legacy local names are aliases only; canonical roles live in ui_theme.h. */
#define COL_BG CF_UI_COL_BG
#define COL_PANEL CF_UI_COL_PANEL
#define COL_SQUARE_LIGHT CF_UI_COL_SQUARE_LIGHT
#define COL_SQUARE_DARK CF_UI_COL_SQUARE_DARK
#define COL_GOLD CF_UI_COL_GOLD
#define COL_TEXT CF_UI_COL_TEXT
#define COL_MUTED CF_UI_COL_MUTED
#define COL_GREEN CF_UI_COL_GREEN
#define COL_GAS CF_UI_COL_GAS
#define COL_BLACK CF_UI_COL_BLACK
#define COL_PANEL_EDGE CF_UI_COL_PANEL_EDGE
#define COL_WHITE_PIECE CF_UI_COL_WHITE_PIECE
#define COL_WHITE_HI CF_UI_COL_WHITE_HI
#define COL_BLACK_PIECE CF_UI_COL_BLACK_PIECE
#define COL_BLACK_HI CF_UI_COL_BLACK_HI
#define COL_CURSOR CF_UI_COL_CURSOR
#define COL_SELECTED CF_UI_COL_SELECTED
#define COL_SHADOW CF_UI_COL_SHADOW
#define COL_LEGAL CF_UI_COL_LEGAL
#define COL_CAPTURE CF_UI_COL_CAPTURE
#define COL_CHECK CF_UI_COL_CHECK
#define COL_PROMOTE CF_UI_COL_PROMOTE
#define COL_FART CF_UI_COL_FART
#define COL_FART_PUSH CF_UI_COL_FART_PUSH
#define COL_SKY CF_UI_COL_SKY
#define COL_CLOUD CF_UI_COL_CLOUD
#define COL_FLASH CF_UI_COL_FLASH
#define COL_COPPER CF_UI_COL_COPPER
#define COL_PANEL_LINE CF_UI_COL_PANEL_LINE
#define COL_PANEL_SOFT CF_UI_COL_PANEL_SOFT

static void load_art_palette(int flash)
{
    cf_u8 palette[VGA_PALETTE_BYTES];
    ui_theme_build_palette(palette, flash);
    vga_set_palette(palette);
}

static void draw_outline(int x, int y, int w, int h, cf_u8 color)
{
    vga_fill_rect(x, y, w, 1, color);
    vga_fill_rect(x, y + h - 1, w, 1, color);
    vga_fill_rect(x, y, 1, h, color);
    vga_fill_rect(x + w - 1, y, 1, h, color);
}

static void draw_panel_box(int x, int y, int w, int h)
{
    vga_fill_rect(x, y, w, h, COL_PANEL_EDGE);
    vga_fill_rect(x + 2, y + 2, w - 4, h - 4, COL_PANEL);
    vga_fill_rect(x + 2, y + 2, w - 4, 1, COL_GOLD);
    vga_fill_rect(x + 2, y + h - 3, w - 4, 1, COL_SHADOW);
}

static cf_u8 square_color(int file, int rank)
{
    int screen_rank = 7 - rank;
    return ((file + screen_rank) & 1) ? COL_SQUARE_DARK : COL_SQUARE_LIGHT;
}

static void draw_square_surface(int x, int y, int file, int rank, cf_u8 color)
{
    cf_u8 grain;

    grain = color == COL_SQUARE_LIGHT ? COL_COPPER : COL_PANEL_SOFT;
    vga_fill_rect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);
    vga_put_pixel(x + 2, y + 2, grain);
    vga_put_pixel(x + 15, y + 15, grain);
    if (((file * 3 + rank) & 3) == 0)
        vga_put_pixel(x + 13, y + 4, grain);
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

static void draw_corner_mark(int x, int y, cf_u8 color)
{
    vga_fill_rect(x, y, 5, 1, color);
    vga_fill_rect(x, y, 1, 5, color);
}

static void draw_selection_brackets(int x, int y, cf_u8 color)
{
    draw_corner_mark(x + 1, y + 1, color);
    vga_fill_rect(x + 12, y + 1, 5, 1, color);
    vga_fill_rect(x + 16, y + 1, 1, 5, color);
    vga_fill_rect(x + 1, y + 16, 5, 1, color);
    vga_fill_rect(x + 1, y + 12, 1, 5, color);
    vga_fill_rect(x + 12, y + 16, 5, 1, color);
    vga_fill_rect(x + 16, y + 12, 1, 5, color);
}

static void draw_move_dot(int x, int y)
{
    vga_fill_rect(x + 7, y + 7, 4, 4, COL_SHADOW);
    vga_fill_rect(x + 8, y + 7, 2, 4, COL_LEGAL);
    vga_fill_rect(x + 7, y + 8, 4, 2, COL_LEGAL);
}

static void draw_capture_mark(int x, int y)
{
    draw_corner_mark(x + 2, y + 2, COL_CAPTURE);
    vga_fill_rect(x + 11, y + 2, 5, 1, COL_CAPTURE);
    vga_fill_rect(x + 15, y + 2, 1, 5, COL_CAPTURE);
    vga_fill_rect(x + 2, y + 15, 5, 1, COL_CAPTURE);
    vga_fill_rect(x + 2, y + 11, 1, 5, COL_CAPTURE);
    vga_fill_rect(x + 11, y + 15, 5, 1, COL_CAPTURE);
    vga_fill_rect(x + 15, y + 11, 1, 5, COL_CAPTURE);
}

static void draw_piece_gas(int x, int y, cf_u8 gas)
{
    int i;
    if (gas == 0U) return;
    for (i = 0; i < 3; ++i) {
        cf_u8 fill = gas > (cf_u8)i ? COL_GAS : COL_PANEL_LINE;
        vga_fill_rect(x + 5 + i * 3, y + 16, 2, 1, fill);
    }
}

static void draw_board_frame(void)
{
    vga_fill_rect(BOARD_FRAME_X, BOARD_FRAME_Y,
                  BOARD_FRAME_W, BOARD_FRAME_H, COL_SHADOW);
    vga_fill_rect(BOARD_FRAME_X + 1, BOARD_FRAME_Y + 1,
                  BOARD_FRAME_W - 2, BOARD_FRAME_H - 2, COL_COPPER);
    vga_fill_rect(BOARD_FRAME_X + 2, BOARD_FRAME_Y + 2,
                  BOARD_FRAME_W - 4, 1, COL_GOLD);
    vga_fill_rect(BOARD_FRAME_X + 2, BOARD_FRAME_Y + 2,
                  1, BOARD_FRAME_H - 4, COL_GOLD);
    vga_fill_rect(BOARD_FRAME_X + 2, BOARD_FRAME_Y + BOARD_FRAME_H - 3,
                  BOARD_FRAME_W - 4, 1, COL_SHADOW);
    vga_fill_rect(BOARD_FRAME_X + BOARD_FRAME_W - 3, BOARD_FRAME_Y + 2,
                  1, BOARD_FRAME_H - 4, COL_SHADOW);
    vga_fill_rect(BOARD_FRAME_X + 3, BOARD_FRAME_Y + 3,
                  BOARD_FRAME_W - 6, BOARD_FRAME_H - 6, COL_BG);
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

    draw_board_frame();

    for (rank = 0; rank < 8; ++rank) {
        screen_rank = 7 - rank;
        for (file = 0; file < 8; ++file) {
            x = BOARD_X + file * SQUARE_SIZE;
            y = BOARD_Y + screen_rank * SQUARE_SIZE;
            color = square_color(file, rank);
            draw_square_surface(x, y, file, rank, color);

            mi = move_index_at(legal_moves, file, rank);
            if (mi >= 0) {
                if (legal_moves->moves[mi].captured.type != CF_PIECE_NONE)
                    draw_capture_mark(x, y);
                else
                    draw_move_dot(x, y);
            }

            piece = board_piece_at(board, file, rank);
            if (piece != 0 && piece->type != CF_PIECE_NONE) {
                ui_assets_draw_piece_overlay(x, y + 1, piece);
                draw_piece_gas(x, y, gas_at(gas, file, rank));
            }
        }
    }

    if (has_selection) {
        x = BOARD_X + selected_file * SQUARE_SIZE;
        y = BOARD_Y + (7 - selected_rank) * SQUARE_SIZE;
        draw_selection_brackets(x, y, COL_SELECTED);
    }

    x = BOARD_X + cursor_file * SQUARE_SIZE;
    y = BOARD_Y + (7 - cursor_rank) * SQUARE_SIZE;
    draw_outline(x, y, SQUARE_SIZE, SQUARE_SIZE, COL_CURSOR);

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

static void draw_fart_trail(int file, int rank,
                            CfFartDirection direction,
                            CfFartPreview preview)
{
    int df;
    int dr;
    int sx;
    int sy;
    int dy;
    int i;
    int steps;

    fart_delta(direction, &df, &dr);
    if (df == 0 && dr == 0) return;
    sx = BOARD_X + file * SQUARE_SIZE + 9;
    sy = BOARD_Y + (7 - rank) * SQUARE_SIZE + 9;
    dy = -dr;
    steps = preview == CF_FART_PUSH || preview == CF_FART_PROMOTION ? 6 : 3;
    for (i = 1; i <= steps; ++i) {
        int px = sx + df * i * 5;
        int py = sy + dy * i * 5;
        int size = i == steps ? 2 : 1;
        if (px - 1 < BOARD_X || py - 1 < BOARD_Y ||
            px - 1 + size > BOARD_X + BOARD_PIXELS ||
            py - 1 + size > BOARD_Y + BOARD_PIXELS)
            break;
        vga_fill_rect(px - 1, py - 1, size, size,
                      preview == CF_FART_INVALID ? COL_CHECK : COL_GAS);
    }
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

    fart_delta(direction, &df, &dr);
    tf = file + df;
    tr = rank + dr;
    pf = tf + df;
    pr = tr + dr;
    if (tf < 0 || tf > 7 || tr < 0 || tr > 7) return;

    x = BOARD_X + tf * SQUARE_SIZE;
    y = BOARD_Y + (7 - tr) * SQUARE_SIZE;
    if (preview == CF_FART_PUFF) {
        draw_outline(x + 5, y + 5, 8, 8, COL_GAS);
        return;
    }
    if (preview == CF_FART_INVALID) {
        draw_outline(x + 5, y + 5, 8, 8, COL_CHECK);
        return;
    }
    draw_selection_brackets(x, y, COL_FART_PUSH);

    if (pf < 0 || pf > 7 || pr < 0 || pr > 7) return;
    x = BOARD_X + pf * SQUARE_SIZE;
    y = BOARD_Y + (7 - pr) * SQUARE_SIZE;
    color = preview == CF_FART_PUSH ? COL_GREEN :
            (preview == CF_FART_PROMOTION ? COL_PROMOTE : COL_CHECK);
    draw_outline(x + 5, y + 5, 8, 8, color);
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

static void draw_gas_pips(int x, int y, cf_u8 gas);

static int square_in_bounds(int file, int rank)
{
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

static void fart_preview_lines(int file, int rank,
                               CfFartDirection direction,
                               CfFartPreview preview,
                               int promotion_pending,
                               CfPieceType promotion_choice,
                               char *line1, char *line2)
{
    int df;
    int dr;
    int tf;
    int tr;
    int pf;
    int pr;
    char target[3];
    char destination[3];

    line1[0] = '\0';
    line2[0] = '\0';
    fart_delta(direction, &df, &dr);
    tf = file + df;
    tr = rank + dr;
    pf = tf + df;
    pr = tr + dr;
    square_name(target, tf, tr);
    square_name(destination, pf, pr);

    if (preview == CF_FART_PUSH || preview == CF_FART_PROMOTION) {
        sprintf(line1, "PUSH %s TO %s", target, destination);
        if (preview == CF_FART_PROMOTION) {
            if (promotion_pending)
                sprintf(line2, "PROMOTE %s",
                        board_piece_type_name(promotion_choice));
            else
                strcpy(line2, "PROMOTION");
        }
    } else if (preview == CF_FART_PUFF) {
        sprintf(line1, "PUFF INTO %s", target);
    } else if (preview == CF_FART_BLOCKED) {
        if (!square_in_bounds(pf, pr))
            strcpy(line1, "BLOCKED AT EDGE");
        else
            sprintf(line1, "BLOCKED AT %s", destination);
    } else {
        strcpy(line1, "INVALID");
    }
}

static void draw_direction_arrow(int x, int y,
                                 CfFartDirection direction, cf_u8 color)
{
    int df;
    int dr;
    int sy;
    int i;

    fart_delta(direction, &df, &dr);
    if (df == 0 && dr == 0) return;
    sy = -dr;
    for (i = 0; i < 3; ++i)
        vga_fill_rect(x + df * i * 2, y + sy * i * 2, 2, 2, color);
    vga_fill_rect(x + df * 6 - 1, y + sy * 6 - 1, 3, 3, color);
}

static void draw_fart_panel_art(const CfBoard *board, const CfGasState *gas,
                                int cursor_file, int cursor_rank,
                                int has_selection,
                                int selected_file, int selected_rank,
                                CfFartDirection fart_direction,
                                CfFartPreview fart_preview,
                                int fart_promotion_pending,
                                CfPieceType fart_promotion_choice,
                                const char *message)
{
    const CfPiece *piece;
    int pf;
    int pr;
    cf_u8 piece_gas;
    char square[3];
    char line[16];
    char preview1[24];
    char preview2[24];

    pf = has_selection ? selected_file : cursor_file;
    pr = has_selection ? selected_rank : cursor_rank;
    piece = board_piece_at(board, pf, pr);
    piece_gas = piece != 0 && piece->type != CF_PIECE_NONE ?
                gas_at(gas, pf, pr) : 0U;
    square_name(square, pf, pr);
    fart_preview_lines(pf, pr, fart_direction, fart_preview,
                       fart_promotion_pending, fart_promotion_choice,
                       preview1, preview2);

    draw_panel_box(PANEL_X, PANEL_Y, PANEL_W, PANEL_H);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_TURN_Y,
                   "TURN", COL_GOLD, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_TURN_Y,
                   board_piece_color_name(board->side_to_move), COL_TEXT, 1);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_STATE_Y,
                   "STATE", COL_MUTED, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_STATE_Y,
                   "FART MODE", COL_GAS, 1);
    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER1_Y,
                  CF_UI_HUD_DIVIDER_W, 1, COL_PANEL_LINE);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_SOURCE_LABEL_Y,
                   "SOURCE", COL_SELECTED, 1);
    vga_fill_rect(CF_UI_FART_PIECE_BOX_X, CF_UI_FART_PIECE_BOX_Y,
                  CF_UI_FART_PIECE_BOX_W, CF_UI_FART_PIECE_BOX_H,
                  COL_PANEL_EDGE);
    vga_fill_rect(CF_UI_FART_PIECE_BOX_X + 1, CF_UI_FART_PIECE_BOX_Y + 1,
                  CF_UI_FART_PIECE_BOX_W - 2, CF_UI_FART_PIECE_BOX_H - 2,
                  COL_PANEL_SOFT);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        ui_assets_draw_piece(CF_UI_FART_PIECE_BOX_X + 1,
                             CF_UI_FART_PIECE_BOX_Y + 2,
                             piece, COL_PANEL_SOFT);
        font_draw_text(CF_UI_FART_PIECE_TEXT_X, CF_UI_FART_PIECE_NAME_Y,
                       board_piece_type_name(piece->type), COL_TEXT, 1);
    }
    font_draw_text(CF_UI_FART_PIECE_TEXT_X, CF_UI_FART_PIECE_SQUARE_Y,
                   square, COL_CURSOR, 1);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_GAS_LABEL_Y,
                   "GAS", COL_GOLD, 1);
    draw_gas_pips(CF_UI_HUD_LABEL_X + 1, CF_UI_FART_GAS_PIPS_Y, piece_gas);
    sprintf(line, "%u/3", (unsigned)piece_gas);
    font_draw_text(CF_UI_HUD_VALUE_X + 9, CF_UI_FART_GAS_PIPS_Y,
                   line, piece_gas >= 2U ? COL_GAS : COL_MUTED, 1);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_DIRECTION_Y,
                   "DIRECTION", COL_SELECTED, 1);
    font_draw_text(CF_UI_FART_DIRECTION_VALUE_X, CF_UI_FART_DIRECTION_Y,
                   gas_direction_name(fart_direction), COL_GAS, 1);
    draw_direction_arrow(286, CF_UI_FART_DIRECTION_Y + 3,
                         fart_direction, COL_GAS);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_PREVIEW_LABEL_Y,
                   "PREVIEW", COL_SELECTED, 1);
    font_draw_text_clipped(CF_UI_HUD_LABEL_X, CF_UI_FART_PREVIEW_LINE1_Y,
                           preview1,
                           fart_preview == CF_FART_INVALID ?
                           COL_CHECK : COL_GAS,
                           1, CF_UI_HUD_DIVIDER_W);
    if (preview2[0] != '\0')
        font_draw_text_clipped(CF_UI_HUD_LABEL_X, CF_UI_FART_PREVIEW_LINE2_Y,
                               preview2,
                               fart_preview == CF_FART_PROMOTION ?
                               COL_PROMOTE : COL_GAS,
                               1, CF_UI_HUD_DIVIDER_W);

    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_FART_LAST_DIVIDER_Y,
                  CF_UI_HUD_DIVIDER_W, 1, COL_PANEL_LINE);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_ACTION_LABEL_Y,
                   "ACTION", COL_SELECTED, 1);
    if (message != 0 && message[0] != '\0')
        font_draw_text_clipped(CF_UI_HUD_LABEL_X,
                               CF_UI_FART_ACTION_LINE1_Y,
                               message, COL_TEXT, 1, CF_UI_HUD_DIVIDER_W);
}

static void draw_gas_pips(int x, int y, cf_u8 gas)
{
    int i;
    for (i = 0; i < 3; ++i) {
        cf_u8 fill = gas > (cf_u8)i ? COL_GAS : COL_PANEL_LINE;
        vga_fill_rect(x + i * 15, y + 1, 9, 5, COL_SHADOW);
        vga_fill_rect(x + 1 + i * 15, y, 7, 7, fill);
        vga_fill_rect(x + i * 15, y + 2, 9, 3, fill);
        if (gas > (cf_u8)i)
            vga_fill_rect(x + 2 + i * 15, y + 1, 3, 2, COL_GREEN);
    }
}

static void draw_panel_art(const CfBoard *board, const CfGasState *gas,
                           int cursor_file, int cursor_rank,
                           int has_selection, int selected_file, int selected_rank,
                           const CfMoveList *legal_moves, CfGameStatus status,
                           int promotion_pending, CfPieceType promotion_choice,
                           int fart_mode, CfFartDirection fart_direction,
                           CfFartPreview fart_preview,
                           int fart_promotion_pending,
                           CfPieceType fart_promotion_choice,
                           const char *message)
{
    const CfPiece *piece;
    int piece_file;
    int piece_rank;
    char square[3];
    char line[24];
    cf_u8 piece_gas = 0U;

    if (fart_mode) {
        draw_fart_panel_art(board, gas, cursor_file, cursor_rank,
                            has_selection, selected_file, selected_rank,
                            fart_direction, fart_preview,
                            fart_promotion_pending, fart_promotion_choice,
                            message);
        return;
    }

    draw_panel_box(PANEL_X, PANEL_Y, PANEL_W, PANEL_H);

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
        piece_gas = gas_at(gas, piece_file, piece_rank);
    } else {
        font_draw_text(208, 69, "EMPTY", COL_MUTED, 1);
    }
    font_draw_text(208, 80, square, COL_CURSOR, 1);

    vga_fill_rect(180, 94, 123, 1, COL_PANEL_LINE);
    font_draw_text(180, 99, "GAS RESERVE", COL_GOLD, 1);
    draw_gas_pips(181, 109, piece_gas);
    sprintf(line, "%u/3", (unsigned)piece_gas);
    font_draw_text(232, 110, line, piece_gas >= 2U ? COL_GAS : COL_MUTED, 1);
    vga_fill_rect(180, 121, 123, 1, COL_PANEL_LINE);

    if (promotion_pending) {
        sprintf(line, "PROMOTE %s", board_piece_type_name(promotion_choice));
        font_draw_text(180, 128, line, COL_PROMOTE, 1);
    } else if (has_selection) {
        sprintf(line, "%d LEGAL MOVES", legal_moves != 0 ? legal_moves->count : 0);
        font_draw_text(180, 128, line, COL_MUTED, 1);
        font_draw_text(180, 139, piece_gas >= 2U ? "FART READY" : "BUILD GAS",
                       piece_gas >= 2U ? COL_GAS : COL_MUTED, 1);
    } else {
        font_draw_text(180, 128, "READY", COL_GREEN, 1);
        if (message != 0 && message[0] != '\0') {
            strncpy(line, message, sizeof(line) - 1U);
            line[sizeof(line) - 1U] = '\0';
            font_draw_text(180, 139, line, COL_MUTED, 1);
        }
    }
}

static void draw_header(int fart_mode)
{
    vga_fill_rect(CF_UI_HEADER_X, CF_UI_HEADER_Y,
                  CF_UI_HEADER_W, CF_UI_HEADER_H, COL_BG);
    font_draw_heading(CF_UI_HEADER_TITLE_X, CF_UI_HEADER_TITLE_Y,
                      "CHESS FART", COL_GOLD, 1);
    if (fart_mode) {
        font_draw_text(CF_UI_FART_HEADER_TAGLINE_X, CF_UI_HEADER_TAGLINE_Y,
                       "CHECK. MATE.", COL_TEXT, 1);
        ui_assets_draw_puff(CF_UI_FART_HEADER_PUFF_X,
                            CF_UI_FART_HEADER_PUFF_Y, 0);
        vga_fill_rect(CF_UI_FART_BADGE_X, CF_UI_FART_BADGE_Y,
                      CF_UI_FART_BADGE_W, CF_UI_FART_BADGE_H,
                      COL_PANEL_EDGE);
        vga_fill_rect(CF_UI_FART_BADGE_X + 1, CF_UI_FART_BADGE_Y + 1,
                      CF_UI_FART_BADGE_W - 2, CF_UI_FART_BADGE_H - 2,
                      COL_PANEL_SOFT);
        vga_fill_rect(CF_UI_FART_BADGE_X + 1, CF_UI_FART_BADGE_Y + 1,
                      CF_UI_FART_BADGE_W - 2, 1, COL_GAS);
        font_draw_text(CF_UI_FART_BADGE_TEXT_X, CF_UI_FART_BADGE_TEXT_Y,
                       "FART MODE", COL_GAS, 1);
    } else {
        font_draw_text(CF_UI_HEADER_TAGLINE_X, CF_UI_HEADER_TAGLINE_Y,
                       "CHECK. MATE. VENTILATE.", COL_TEXT, 1);
    }
    vga_fill_rect(CF_UI_HEADER_X, CF_UI_HEADER_RULE_Y,
                  CF_UI_HEADER_W, 1, COL_COPPER);
}

static void draw_command_bar(CfPieceColor side_to_move,
                             int fart_mode,
                             int fart_promotion_pending)
{
    char line[16];

    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, CF_UI_COMMAND_H, COL_BG);
    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, 1, COL_GOLD);

    if (fart_mode) {
        font_draw_text(CF_UI_FART_PROMPT_ARROWS_X, CF_UI_PROMPT_TEXT_Y,
                       fart_promotion_pending ?
                       "ARROWS CHOOSE PIECE" : "ARROWS CHANGE DIRECTION",
                       COL_GAS, 1);
        font_draw_text(CF_UI_FART_PROMPT_ENTER_X, CF_UI_PROMPT_TEXT_Y,
                       "ENTER CONFIRM", COL_TEXT, 1);
        font_draw_text(CF_UI_FART_PROMPT_ESC_X, CF_UI_PROMPT_TEXT_Y,
                       "ESC CANCEL", COL_MUTED, 1);
        return;
    }

    sprintf(line, "%s TO MOVE", board_piece_color_name(side_to_move));
    font_draw_text(CF_UI_PROMPT_TURN_X, CF_UI_PROMPT_TEXT_Y,
                   line, COL_GOLD, 1);
    font_draw_text(CF_UI_PROMPT_FART_X, CF_UI_PROMPT_TEXT_Y,
                   "F FART", COL_GAS, 1);
    font_draw_text(CF_UI_PROMPT_SAVE_X, CF_UI_PROMPT_TEXT_Y,
                   "S SAVE", COL_TEXT, 1);
    font_draw_text(CF_UI_PROMPT_LOAD_X, CF_UI_PROMPT_TEXT_Y,
                   "L LOAD", COL_TEXT, 1);
    font_draw_text(CF_UI_PROMPT_HELP_X, CF_UI_PROMPT_TEXT_Y,
                   "H HELP", COL_TEXT, 1);
    font_draw_text(CF_UI_PROMPT_ESC_X, CF_UI_PROMPT_TEXT_Y,
                   "ESC MENU", COL_MUTED, 1);
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
            draw_selection_brackets(px + jitter, py,
                                    frame == 2 ? COL_FLASH : COL_FART_PUSH);
        }
    }
    draw_outline(BOARD_FRAME_X + jitter, BOARD_FRAME_Y,
                 BOARD_FRAME_W, BOARD_FRAME_H,
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

    load_art_palette(flash);
    vga_clear(COL_BG);
    draw_header(fart_mode);
    draw_board(board, gas, cursor_file, cursor_rank,
               has_selection, selected_file, selected_rank, legal_moves);
    if (has_selection && fart_mode) {
        draw_fart_trail(selected_file, selected_rank,
                        fart_direction, fart_preview);
        draw_push_geometry(selected_file, selected_rank,
                           fart_direction, fart_preview);
    }
    draw_panel_art(board, gas, cursor_file, cursor_rank,
                   has_selection, selected_file, selected_rank,
                   legal_moves, status, promotion_pending,
                   promotion_choice, fart_mode, fart_direction,
                   fart_preview, fart_promotion_pending,
                   fart_promotion_choice, message);
    draw_command_bar(board->side_to_move, fart_mode,
                     fart_promotion_pending);
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
    draw_outline(4, 4, 312, 192, COL_PANEL_EDGE);
    draw_outline(6, 6, 308, 188, COL_COPPER);
    font_draw_heading(100, 20, "CHESS FART", COL_GOLD, 2);
    font_draw_text(88, 44, "CHECK. MATE. VENTILATE.", COL_SELECTED, 1);
    vga_fill_rect(52, 61, 216, 1, COL_PANEL_LINE);
    vga_fill_rect(52, 64, 216, 1, COL_COPPER);
    ui_assets_draw_puff(65 + phase, 33, 1);
    ui_assets_draw_puff(239 - phase, 33, 1);
    font_draw_text(116, 77, "DOS VGA EDITION", COL_MUTED, 1);
}
