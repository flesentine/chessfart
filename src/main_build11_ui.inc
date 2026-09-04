#include <time.h>
#include "ui_assets.h"
#include "ui_layout.h"
#include "ui_theme.h"

#ifdef CF_HOST_BUILD
void vga_host_capture(const char *stem);
static void ux_capture(const char *stem) { vga_host_capture(stem); }
#else
static void ux_capture(const char *stem) { (void)stem; }
#endif

#define UX_COL_BG CF_UI_COL_BG
#define UX_COL_PANEL CF_UI_COL_PANEL
#define UX_COL_GOLD CF_UI_COL_GOLD
#define UX_COL_TEXT CF_UI_COL_TEXT
#define UX_COL_MUTED CF_UI_COL_MUTED
#define UX_COL_GREEN CF_UI_COL_GREEN
#define UX_COL_GAS CF_UI_COL_GAS
#define UX_COL_PANEL_EDGE CF_UI_COL_PANEL_EDGE
#define UX_COL_CURSOR CF_UI_COL_CURSOR
#define UX_COL_SELECTED CF_UI_COL_SELECTED
#define UX_COL_SHADOW CF_UI_COL_SHADOW
#define UX_COL_PANEL_LINE CF_UI_COL_PANEL_LINE
#define UX_COL_PANEL_SOFT CF_UI_COL_PANEL_SOFT
#define UX_COL_CHECK CF_UI_COL_CHECK

static void ux_stamp_build11(void)
{
    /* Retail renderer deliberately hides internal build numbering. */
}

static void ux_draw_centered(int x, int y, int w,
                             const char *text, cf_u8 color)
{
    int tw;
    if (text == 0 || w <= 0) return;
    tw = font_text_width(text, 1);
    if (tw <= w)
        font_draw_text(x + (w - tw) / 2, y, text, color, 1);
    else
        font_draw_text_clipped(x, y, text, color, 1, w);
}

static void ux_draw_modal_frame(int x, int y, int w, int h,
                                const char *title)
{
    vga_fill_rect(x + 3, y + 3, w, h, UX_COL_SHADOW);
    vga_fill_rect(x, y, w, h, UX_COL_PANEL_EDGE);
    vga_fill_rect(x + 2, y + 2, w - 4, h - 4, UX_COL_PANEL);
    vga_fill_rect(x + 2, y + 2, w - 4, 1, UX_COL_GOLD);
    vga_fill_rect(x + 2, y + h - 3, w - 4, 1, UX_COL_SHADOW);
    vga_fill_rect(x + 8, y + CF_UI_MODAL_RULE_Y_OFFSET,
                  w - 16, 1, UX_COL_PANEL_LINE);
    ux_draw_centered(x, y + CF_UI_MODAL_TITLE_Y_OFFSET, w,
                     title, UX_COL_SELECTED);
}

static void ux_draw_modal_footer(int x, int y, int w, int h,
                                 const char *text)
{
    int fy = y + h - CF_UI_MODAL_FOOTER_H;
    vga_fill_rect(x + 8, fy, w - 16, 1, UX_COL_PANEL_LINE);
    ux_draw_centered(x, fy + 6, w, text, UX_COL_MUTED);
}

static void ux_draw_help_page(int page)
{
    int x = CF_UI_MODAL_HELP_X;
    int y = CF_UI_MODAL_HELP_Y;
    int w = CF_UI_MODAL_HELP_W;
    int h = CF_UI_MODAL_HELP_H;

    ux_draw_modal_frame(x, y, w, h, "CHESS FART HELP");
    font_draw_text(x + w - 61, y + 10,
                   page == 0 ? "PAGE 1/2" : "PAGE 2/2",
                   UX_COL_MUTED, 1);

    if (page == 0) {
        font_draw_heading(x + 15, y + 34, "CONTROLS", UX_COL_GOLD, 1);
        font_draw_text(x + 15, y + 46, "ARROWS MOVE CURSOR", UX_COL_GAS, 1);
        font_draw_text(x + 15, y + 56, "ENTER/SPACE SELECT / CONFIRM", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 66, "F FART   S SAVE   L LOAD", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 76, "H/? HELP   TAB/M LOG   C CREDITS", UX_COL_TEXT, 1);

        font_draw_heading(x + 15, y + 90, "GAS + FARTS", UX_COL_GOLD, 1);
        font_draw_text(x + 15, y + 102, "MOVE +1 GAS   CAPTURE +2 GAS", UX_COL_GAS, 1);
        font_draw_text(x + 15, y + 112, "2 GAS = ONE FART ACTION", UX_COL_GAS, 1);
        font_draw_text(x + 15, y + 122, "EMPTY TARGET = PUFF", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 132, "PIECE TARGET = PUSH ONE SQUARE", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 139, "RIGHT CLICK FART   LEFT CLICK BOARD", UX_COL_MUTED, 1);
    } else {
        font_draw_heading(x + 15, y + 34, "CHESS SAFETY", UX_COL_GOLD, 1);
        font_draw_text(x + 15, y + 46, "ALL NORMAL CHESS RULES APPLY", UX_COL_GAS, 1);
        font_draw_text(x + 15, y + 56, "FART PUSHES MUST KEEP YOUR KING SAFE", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 66, "KINGS MAY BE PUSHED IF POSITION LEGAL", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 76, "PUSHED PAWN CAN PROMOTE Q/R/B/N", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 86, "HOME KING/ROOK PUSH KILLS CASTLING", UX_COL_TEXT, 1);

        font_draw_heading(x + 15, y + 98, "CPU + SAVES", UX_COL_GOLD, 1);
        font_draw_text(x + 15, y + 110, "CPU BLACK SEARCHES MOVES + FARTS", UX_COL_GAS, 1);
        font_draw_text(x + 15, y + 120, "D ON TITLE: EASY / MED / HARD", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 130, "GAS COUNTS IN THREEFOLD IDENTITY", UX_COL_TEXT, 1);
        font_draw_text(x + 15, y + 139, "SAVE STORES GAS + REPETITION HISTORY", UX_COL_MUTED, 1);
    }
    ux_draw_modal_footer(x, y, w, h, "LEFT/RIGHT PAGE   ENTER CLOSE");
}

static void ux_draw_credits_screen(void)
{
    int x = CF_UI_MODAL_CREDITS_X;
    int y = CF_UI_MODAL_CREDITS_Y;
    int w = CF_UI_MODAL_CREDITS_W;
    int h = CF_UI_MODAL_CREDITS_H;

    ux_draw_modal_frame(x, y, w, h, "CREDITS");
    ui_assets_draw_puff(x + 23, y + 31, 1);
    ui_assets_draw_puff(x + w - 39, y + 31, 1);
    ux_draw_centered(x, y + 34, w, "CHESS FART", UX_COL_GOLD);
    ux_draw_centered(x, y + 53, w, "A FLESENTINE DOS PRODUCTION", UX_COL_TEXT);
    ux_draw_centered(x, y + 72, w, "320x200 VGA / 256 COLORS", UX_COL_GAS);
    ux_draw_centered(x, y + 84, w, "SOUND BLASTER + PC SPEAKER", UX_COL_GAS);
    ux_draw_centered(x, y + 96, w, "OPEN WATCOM C89 TARGET", UX_COL_TEXT);
    ux_draw_centered(x, y + 111, w, "CHECK. MATE. VENTILATE.", UX_COL_GOLD);
    ux_draw_modal_footer(x, y, w, h, "ENTER / ESC CLOSE");
}

static void ux_draw_history_screen(void)
{
    int x = CF_UI_MODAL_HISTORY_X;
    int y = CF_UI_MODAL_HISTORY_Y;
    int w = CF_UI_MODAL_HISTORY_W;
    int h = CF_UI_MODAL_HISTORY_H;
    int first;
    int i;
    const char *line;
    cf_u8 color;

    ux_draw_modal_frame(x, y, w, h, "ACTION LOG");
    if (g_ux_history.count == 0) {
        ux_draw_centered(x, y + 73, w, "NO ACTIONS YET", UX_COL_MUTED);
    } else {
        first = g_ux_history.count > 11 ? g_ux_history.count - 11 : 0;
        for (i = first; i < g_ux_history.count; ++i) {
            line = ux_history_get(&g_ux_history, i);
            if (line != 0) {
                if (strncmp(line, "YOU", 3) == 0)
                    color = UX_COL_GAS;
                else if (strncmp(line, "CPU", 3) == 0)
                    color = UX_COL_GOLD;
                else
                    color = UX_COL_MUTED;
                font_draw_text_clipped(x + 13, y + 34 + (i - first) * 10,
                                       line, color, 1, w - 26);
            }
        }
    }
    ux_draw_modal_footer(x, y, w, h, "TAB/M / ENTER CLOSE");
}

static void ux_draw_terminal_overlay(CfGameStatus status,
                                     CfPieceColor side_to_move)
{
    int x = CF_UI_MODAL_TERMINAL_X;
    int y = CF_UI_MODAL_TERMINAL_Y;
    int w = CF_UI_MODAL_TERMINAL_W;
    int h = CF_UI_MODAL_TERMINAL_H;
    const char *result = ux_terminal_title(status, side_to_move);
    const char *detail;

    if (status == CF_GAME_CHECKMATE)
        detail = "CHECKMATE - NO LEGAL ESCAPE";
    else if (status == CF_GAME_DRAW_THREEFOLD)
        detail = "DRAW 3-FOLD";
    else
        detail = board_game_status_name(status);

    ux_draw_modal_frame(x, y, w, h, "GAME OVER");
    ux_draw_centered(x, y + 30, w, result,
                     status == CF_GAME_CHECKMATE ? UX_COL_GAS : UX_COL_GOLD);
    ux_draw_centered(x, y + 43, w, detail, UX_COL_TEXT);
    ux_draw_modal_footer(x, y, w, h, "H HELP   M LOG   ESC QUIT");
}

static void ux_draw_title(int menu, int frame)
{
    static const char *items[CF_UI_TITLE_MENU_ITEM_COUNT] = {
        "PLAY CPU", "2 PLAYERS", "ATTRACT DEMO",
        "HELP / RULES", "CREDITS", "QUIT TO DOS"
    };
    char line[32];
    const CfAudioConfig *config = audio_get_config();
    const CfAudioStats *stats = audio_get_stats();
    int i;
    int iy;

    board_view_render_title7(menu, frame);

    sprintf(line, "AUDIO %s/%s",
            audio_device_name(stats->actual_device),
            audio_level_name(config->sfx_level));
    font_draw_text(CF_UI_TITLE_MENU_X + 3, CF_UI_TITLE_STATUS_Y,
                   line, UX_COL_MUTED, 1);
    sprintf(line, "CPU %s", cpu_difficulty_name(g_cpu_config.difficulty));
    font_draw_text(201, CF_UI_TITLE_STATUS_Y, line, UX_COL_MUTED, 1);

    vga_fill_rect(CF_UI_TITLE_MENU_X, CF_UI_TITLE_MENU_Y,
                  CF_UI_TITLE_MENU_W, CF_UI_TITLE_MENU_H, UX_COL_PANEL_EDGE);
    vga_fill_rect(CF_UI_TITLE_MENU_INNER_X, CF_UI_TITLE_MENU_INNER_Y,
                  CF_UI_TITLE_MENU_INNER_W, CF_UI_TITLE_MENU_INNER_H,
                  UX_COL_PANEL);
    vga_fill_rect(CF_UI_TITLE_MENU_INNER_X, CF_UI_TITLE_MENU_INNER_Y,
                  CF_UI_TITLE_MENU_INNER_W, 1, UX_COL_GOLD);
    ux_draw_centered(CF_UI_TITLE_MENU_X, CF_UI_TITLE_MENU_LABEL_Y,
                     CF_UI_TITLE_MENU_W, "MAIN MENU", UX_COL_SELECTED);
    vga_fill_rect(CF_UI_TITLE_MENU_X + 10, CF_UI_TITLE_MENU_RULE_Y,
                  CF_UI_TITLE_MENU_W - 20, 1, UX_COL_PANEL_LINE);

    for (i = 0; i < CF_UI_TITLE_MENU_ITEM_COUNT; ++i) {
        iy = CF_UI_TITLE_MENU_ITEM_Y + i * CF_UI_TITLE_MENU_ITEM_STEP;
        if (i == menu) {
            vga_fill_rect(CF_UI_TITLE_MENU_X + 9, iy - 1,
                          CF_UI_TITLE_MENU_W - 18,
                          CF_UI_TITLE_MENU_ITEM_H, UX_COL_PANEL_SOFT);
            vga_fill_rect(CF_UI_TITLE_MENU_X + 13, iy + 2, 4, 4, UX_COL_GAS);
        }
        font_draw_text(CF_UI_TITLE_MENU_ITEM_X, iy, items[i],
                       i == menu ? UX_COL_GAS : UX_COL_TEXT, 1);
    }

    ux_draw_centered(0, CF_UI_TITLE_FOOTER_Y, CF_UI_SCREEN_W,
                     "UP/DOWN SELECT   ENTER CONFIRM", UX_COL_TEXT);
    ux_draw_centered(0, CF_UI_TITLE_FOOTER2_Y, CF_UI_SCREEN_W,
                     "D CPU   F SFX   LEFT/RIGHT AUDIO", UX_COL_MUTED);
}

static void ux_panel_outline(void)
{
    vga_fill_rect(CF_UI_PANEL_X, CF_UI_PANEL_Y,
                  CF_UI_PANEL_W, CF_UI_PANEL_H, UX_COL_PANEL_EDGE);
    vga_fill_rect(CF_UI_PANEL_X + 2, CF_UI_PANEL_Y + 2,
                  CF_UI_PANEL_W - 4, CF_UI_PANEL_H - 4, UX_COL_PANEL);
    vga_fill_rect(CF_UI_PANEL_X + 2, CF_UI_PANEL_Y + 2,
                  CF_UI_PANEL_W - 4, 1, UX_COL_GOLD);
    vga_fill_rect(CF_UI_PANEL_X + 2,
                  CF_UI_PANEL_Y + CF_UI_PANEL_H - 3,
                  CF_UI_PANEL_W - 4, 1, UX_COL_SHADOW);
}

static void ux_draw_gas_pips(int x, int y, cf_u8 gas)
{
    int i;
    for (i = 0; i < 3; ++i) {
        cf_u8 fill = gas > (cf_u8)i ? UX_COL_GAS : UX_COL_PANEL_LINE;
        vga_fill_rect(x + i * 15, y, 9, 7, UX_COL_SHADOW);
        vga_fill_rect(x + 1 + i * 15, y + 1, 7, 5, fill);
        if (gas > (cf_u8)i)
            vga_fill_rect(x + 2 + i * 15, y + 2, 3, 2, UX_COL_GREEN);
    }
}

static void ux_square_name(char *out, int file, int rank)
{
    if (file < 0 || rank < 0) {
        out[0] = '-'; out[1] = '-'; out[2] = '\0';
        return;
    }
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

static void ux_draw_action_two_lines(int x, int y, int second_y,
                                     const char *text, cf_u8 color)
{
    char first[21];
    char second[21];
    int len;
    int split;
    int start;

    if (text == 0 || text[0] == '\0') {
        font_draw_text(x, y, "NONE YET", UX_COL_MUTED, 1);
        return;
    }

    len = (int)strlen(text);
    if (len <= 20) {
        font_draw_text_clipped(x, y, text, color, 1, CF_UI_HUD_DIVIDER_W);
        return;
    }

    split = 20;
    while (split > 0 && text[split] != ' ') --split;
    if (split == 0) split = 20;

    strncpy(first, text, (size_t)split);
    first[split] = '\0';
    start = split;
    while (text[start] == ' ') ++start;
    strncpy(second, text + start, 20U);
    second[20] = '\0';

    font_draw_text_clipped(x, y, first, color, 1, CF_UI_HUD_DIVIDER_W);
    font_draw_text_clipped(x, second_y,
                           second, color, 1, CF_UI_HUD_DIVIDER_W);
}

static void ux_fart_delta(CfFartDirection direction, int *df, int *dr)
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

static int ux_square_in_bounds(int file, int rank)
{
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

static void ux_draw_direction_arrow(int x, int y,
                                    CfFartDirection direction, cf_u8 color)
{
    int df;
    int dr;
    int sx;
    int sy;
    int i;

    ux_fart_delta(direction, &df, &dr);
    if (df == 0 && dr == 0) return;
    sy = -dr;
    for (i = 0; i < 3; ++i)
        vga_fill_rect(x + df * i * 2, y + sy * i * 2, 2, 2, color);
    sx = x + df * 6;
    sy = y + sy * 6;
    vga_fill_rect(sx - 1, sy - 1, 3, 3, color);
}

static void ux_fart_preview_lines(int file, int rank,
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
    ux_fart_delta(direction, &df, &dr);
    tf = file + df;
    tr = rank + dr;
    pf = tf + df;
    pr = tr + dr;
    ux_square_name(target, tf, tr);
    ux_square_name(destination, pf, pr);

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
        if (!ux_square_in_bounds(pf, pr))
            strcpy(line1, "BLOCKED AT EDGE");
        else
            sprintf(line1, "BLOCKED AT %s", destination);
    } else {
        strcpy(line1, "INVALID");
    }
}

static void ux_draw_fart_panel(const CfBoard *board,
                               const CfGasState *gas,
                               int cursor_file,
                               int cursor_rank,
                               int has_selection,
                               int selected_file,
                               int selected_rank,
                               CfFartDirection fart_direction,
                               CfFartPreview fart_preview,
                               int fart_promotion_pending,
                               CfPieceType fart_promotion_choice,
                               const char *action_text)
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
    ux_square_name(square, pf, pr);
    ux_fart_preview_lines(pf, pr, fart_direction, fart_preview,
                          fart_promotion_pending, fart_promotion_choice,
                          preview1, preview2);

    ux_panel_outline();

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_TURN_Y,
                   "TURN", UX_COL_GOLD, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_TURN_Y,
                   board_piece_color_name(board->side_to_move), UX_COL_TEXT, 1);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_STATE_Y,
                   "STATE", UX_COL_MUTED, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_STATE_Y,
                   "FART MODE", UX_COL_GAS, 1);
    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER1_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_SOURCE_LABEL_Y,
                   "SOURCE", UX_COL_SELECTED, 1);
    vga_fill_rect(CF_UI_FART_PIECE_BOX_X, CF_UI_FART_PIECE_BOX_Y,
                  CF_UI_FART_PIECE_BOX_W, CF_UI_FART_PIECE_BOX_H,
                  UX_COL_PANEL_EDGE);
    vga_fill_rect(CF_UI_FART_PIECE_BOX_X + 1, CF_UI_FART_PIECE_BOX_Y + 1,
                  CF_UI_FART_PIECE_BOX_W - 2, CF_UI_FART_PIECE_BOX_H - 2,
                  UX_COL_PANEL_SOFT);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        ui_assets_draw_piece(CF_UI_FART_PIECE_BOX_X + 1,
                             CF_UI_FART_PIECE_BOX_Y + 2,
                             piece, UX_COL_PANEL_SOFT);
        font_draw_text(CF_UI_FART_PIECE_TEXT_X, CF_UI_FART_PIECE_NAME_Y,
                       board_piece_type_name(piece->type), UX_COL_TEXT, 1);
    }
    font_draw_text(CF_UI_FART_PIECE_TEXT_X, CF_UI_FART_PIECE_SQUARE_Y,
                   square, UX_COL_CURSOR, 1);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_GAS_LABEL_Y,
                   "GAS", UX_COL_GOLD, 1);
    ux_draw_gas_pips(CF_UI_HUD_LABEL_X + 1,
                     CF_UI_FART_GAS_PIPS_Y, piece_gas);
    sprintf(line, "%u/3", (unsigned)piece_gas);
    font_draw_text(CF_UI_HUD_VALUE_X + 9, CF_UI_FART_GAS_PIPS_Y,
                   line, piece_gas >= 2U ? UX_COL_GAS : UX_COL_MUTED, 1);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_DIRECTION_Y,
                   "DIRECTION", UX_COL_SELECTED, 1);
    font_draw_text(CF_UI_FART_DIRECTION_VALUE_X, CF_UI_FART_DIRECTION_Y,
                   gas_direction_name(fart_direction), UX_COL_GAS, 1);
    ux_draw_direction_arrow(286, CF_UI_FART_DIRECTION_Y + 3,
                            fart_direction, UX_COL_GAS);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_PREVIEW_LABEL_Y,
                   "PREVIEW", UX_COL_SELECTED, 1);
    font_draw_text_clipped(CF_UI_HUD_LABEL_X, CF_UI_FART_PREVIEW_LINE1_Y,
                           preview1,
                           fart_preview == CF_FART_INVALID ?
                           CF_UI_COL_CHECK : UX_COL_GAS,
                           1, CF_UI_HUD_DIVIDER_W);
    if (preview2[0] != '\0')
        font_draw_text_clipped(CF_UI_HUD_LABEL_X, CF_UI_FART_PREVIEW_LINE2_Y,
                               preview2,
                               fart_preview == CF_FART_PROMOTION ?
                               CF_UI_COL_PROMOTE : UX_COL_GAS,
                               1, CF_UI_HUD_DIVIDER_W);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_CPU_Y,
                   "CPU", UX_COL_SELECTED, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_FART_CPU_Y,
                   cpu_difficulty_name(g_cpu_config.difficulty), UX_COL_GOLD, 1);

    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_FART_LAST_DIVIDER_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_FART_ACTION_LABEL_Y,
                   "LAST ACTION", UX_COL_SELECTED, 1);
    ux_draw_action_two_lines(CF_UI_HUD_LABEL_X,
                             CF_UI_FART_ACTION_LINE1_Y,
                             CF_UI_FART_ACTION_LINE2_Y,
                             action_text, UX_COL_TEXT);
}

static const char *ux_hud_status_name(CfGameStatus status)
{
    if (status == CF_GAME_DRAW_THREEFOLD) return "DRAW 3-FOLD";
    return board_game_status_name(status);
}

static void ux_draw_retail_panel(const CfBoard *board,
                                 const CfGasState *gas,
                                 int cursor_file,
                                 int cursor_rank,
                                 int has_selection,
                                 int selected_file,
                                 int selected_rank,
                                 CfGameStatus status,
                                 const char *action_text,
                                 int fart_mode,
                                 CfFartDirection fart_direction,
                                 CfFartPreview fart_preview,
                                 int fart_promotion_pending,
                                 CfPieceType fart_promotion_choice)
{
    const CfPiece *piece;
    const char *mode_label;
    int pf;
    int pr;
    cf_u8 piece_gas;
    char square[3];
    char line[16];

    if (fart_mode) {
        ux_draw_fart_panel(board, gas, cursor_file, cursor_rank,
                           has_selection, selected_file, selected_rank,
                           fart_direction, fart_preview,
                           fart_promotion_pending, fart_promotion_choice,
                           action_text);
        return;
    }

    pf = has_selection ? selected_file : cursor_file;
    pr = has_selection ? selected_rank : cursor_rank;
    piece = board_piece_at(board, pf, pr);
    piece_gas = 0U;
    ux_square_name(square, pf, pr);

    if (has_selection)
        mode_label = "SELECTED";
    else if (piece != 0 && piece->type != CF_PIECE_NONE)
        mode_label = "CURSOR";
    else
        mode_label = "EMPTY";

    ux_panel_outline();

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_TURN_Y,
                   "TURN", UX_COL_GOLD, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_TURN_Y,
                   board_piece_color_name(board->side_to_move), UX_COL_TEXT, 1);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_STATE_Y,
                   "STATE", UX_COL_MUTED, 1);
    font_draw_text_clipped(CF_UI_HUD_VALUE_X, CF_UI_HUD_STATE_Y,
                           ux_hud_status_name(status),
                           status == CF_GAME_CHECK || status == CF_GAME_CHECKMATE ?
                           CF_UI_COL_CHECK : UX_COL_GREEN, 1,
                           CF_UI_PANEL_CONTENT_X + CF_UI_PANEL_CONTENT_W -
                           CF_UI_HUD_VALUE_X);
    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER1_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_PIECE_LABEL_Y,
                   mode_label,
                   has_selection ? UX_COL_SELECTED :
                   (piece != 0 && piece->type != CF_PIECE_NONE ?
                    UX_COL_CURSOR : UX_COL_MUTED), 1);
    vga_fill_rect(CF_UI_HUD_PIECE_BOX_X, CF_UI_HUD_PIECE_BOX_Y,
                  CF_UI_HUD_PIECE_BOX_W, CF_UI_HUD_PIECE_BOX_H,
                  UX_COL_PANEL_EDGE);
    vga_fill_rect(CF_UI_HUD_PIECE_BOX_X + 2, CF_UI_HUD_PIECE_BOX_Y + 2,
                  18, 18, UX_COL_PANEL_SOFT);

    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        ui_assets_draw_piece(CF_UI_HUD_PIECE_BOX_X + 2,
                             CF_UI_HUD_PIECE_BOX_Y + 3,
                             piece, UX_COL_PANEL_SOFT);
        font_draw_text(CF_UI_HUD_PIECE_TEXT_X, CF_UI_HUD_PIECE_NAME_Y,
                       board_piece_type_name(piece->type), UX_COL_TEXT, 1);
        piece_gas = gas_at(gas, pf, pr);
    }
    font_draw_text(CF_UI_HUD_PIECE_TEXT_X, CF_UI_HUD_PIECE_SQUARE_Y,
                   square, UX_COL_CURSOR, 1);

    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER2_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_GAS_LABEL_Y,
                   "GAS", UX_COL_GOLD, 1);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        ux_draw_gas_pips(CF_UI_HUD_LABEL_X + 1,
                         CF_UI_HUD_GAS_PIPS_Y, piece_gas);
        sprintf(line, "%u/3", (unsigned)piece_gas);
        font_draw_text(CF_UI_HUD_VALUE_X + 9, CF_UI_HUD_GAS_PIPS_Y,
                       line, piece_gas >= 2U ? UX_COL_GAS : UX_COL_MUTED, 1);
    } else {
        font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_GAS_LABEL_Y,
                       "N/A", UX_COL_MUTED, 1);
    }

    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER3_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_CPU_Y,
                   "CPU", UX_COL_SELECTED, 1);
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_CPU_Y,
                   cpu_difficulty_name(g_cpu_config.difficulty), UX_COL_GOLD, 1);

    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER4_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);
    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_ACTION_LABEL_Y,
                   "LAST ACTION", UX_COL_SELECTED, 1);
    ux_draw_action_two_lines(CF_UI_HUD_LABEL_X,
                             CF_UI_HUD_ACTION_LINE1_Y,
                             CF_UI_HUD_ACTION_LINE2_Y,
                             action_text, UX_COL_TEXT);
}

static void ux_draw_command_bar(const CfBoard *board,
                                int fart_mode,
                                int fart_promotion_pending)
{
    char line[16];

    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, CF_UI_COMMAND_H, UX_COL_BG);
    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, 1, UX_COL_GOLD);

    if (fart_mode) {
        font_draw_text(CF_UI_FART_PROMPT_ARROWS_X, CF_UI_PROMPT_TEXT_Y,
                       fart_promotion_pending ?
                       "ARROWS CHOOSE PIECE" : "ARROWS CHANGE DIRECTION",
                       UX_COL_GAS, 1);
        font_draw_text(CF_UI_FART_PROMPT_ENTER_X, CF_UI_PROMPT_TEXT_Y,
                       "ENTER CONFIRM", UX_COL_TEXT, 1);
        font_draw_text(CF_UI_FART_PROMPT_ESC_X, CF_UI_PROMPT_TEXT_Y,
                       "ESC CANCEL", UX_COL_MUTED, 1);
        return;
    }

    sprintf(line, "%s TO MOVE", board_piece_color_name(board->side_to_move));
    font_draw_text(CF_UI_PROMPT_TURN_X, CF_UI_PROMPT_TEXT_Y,
                   line, UX_COL_GOLD, 1);
    font_draw_text(CF_UI_PROMPT_FART_X, CF_UI_PROMPT_TEXT_Y,
                   "F FART", UX_COL_GAS, 1);
    font_draw_text(CF_UI_PROMPT_SAVE_X, CF_UI_PROMPT_TEXT_Y,
                   "S SAVE", UX_COL_TEXT, 1);
    font_draw_text(CF_UI_PROMPT_LOAD_X, CF_UI_PROMPT_TEXT_Y,
                   "L LOAD", UX_COL_TEXT, 1);
    font_draw_text(CF_UI_PROMPT_HELP_X, CF_UI_PROMPT_TEXT_Y,
                   "H HELP", UX_COL_TEXT, 1);
    font_draw_text(CF_UI_PROMPT_ESC_X, CF_UI_PROMPT_TEXT_Y,
                   "ESC MENU", UX_COL_MUTED, 1);
}

static void ux_draw_persistence_notice(const char *message)
{
    int inner_w;
    int tw;
    cf_u8 accent;

    if (message == 0) return;
    if (strncmp(message, "SAVE ", 5) != 0 &&
        strncmp(message, "LOAD ", 5) != 0) return;

    accent = strstr(message, " OK") != 0 ? UX_COL_GAS : UX_COL_CHECK;
    vga_fill_rect(CF_UI_NOTICE_X + 2, CF_UI_NOTICE_Y + 2,
                  CF_UI_NOTICE_W, CF_UI_NOTICE_H, UX_COL_SHADOW);
    vga_fill_rect(CF_UI_NOTICE_X, CF_UI_NOTICE_Y,
                  CF_UI_NOTICE_W, CF_UI_NOTICE_H, UX_COL_PANEL_EDGE);
    vga_fill_rect(CF_UI_NOTICE_X + 2, CF_UI_NOTICE_Y + 2,
                  CF_UI_NOTICE_W - 4, CF_UI_NOTICE_H - 4, UX_COL_PANEL_SOFT);
    vga_fill_rect(CF_UI_NOTICE_X + 2, CF_UI_NOTICE_Y + 2,
                  CF_UI_NOTICE_W - 4, 1, accent);
    inner_w = CF_UI_NOTICE_W - 10;
    tw = font_text_width(message, 1);
    if (tw <= inner_w)
        font_draw_text(CF_UI_NOTICE_X + (CF_UI_NOTICE_W - tw) / 2,
                       CF_UI_NOTICE_TEXT_Y, message, accent, 1);
    else
        font_draw_text_clipped(CF_UI_NOTICE_X + 5,
                               CF_UI_NOTICE_TEXT_Y, message,
                               accent, 1, inner_w);
}

static void ux_render_cached_base(void)
{
    const char *shown;
    const char *last;
    const char *action;
    if (!g_ux_cache.valid) return;

    shown = g_cpu_message_pending ? g_cpu_message : g_ux_cache.message;
    persistence_render_game(&g_ux_cache.board, &g_ux_cache.gas,
                            g_ux_cache.cursor_file, g_ux_cache.cursor_rank,
                            g_ux_cache.has_selection,
                            g_ux_cache.selected_file, g_ux_cache.selected_rank,
                            &g_ux_cache.moves, g_ux_cache.status,
                            g_ux_cache.promotion_pending,
                            g_ux_cache.promotion_choice,
                            g_ux_cache.fart_mode,
                            g_ux_cache.fart_direction,
                            g_ux_cache.fart_preview,
                            g_ux_cache.fart_promotion_pending,
                            g_ux_cache.fart_promotion_choice, shown);

    last = g_ux_history.count > 0 ?
           ux_history_get(&g_ux_history, g_ux_history.count - 1) : 0;
    if (g_ux_cache.fart_mode)
        action = last;
    else
        action = shown != 0 && shown[0] != '\0' ? shown : last;
    ux_draw_retail_panel(&g_ux_cache.board, &g_ux_cache.gas,
                         g_ux_cache.cursor_file, g_ux_cache.cursor_rank,
                         g_ux_cache.has_selection,
                         g_ux_cache.selected_file, g_ux_cache.selected_rank,
                         g_ux_cache.status, action, g_ux_cache.fart_mode,
                         g_ux_cache.fart_direction, g_ux_cache.fart_preview,
                         g_ux_cache.fart_promotion_pending,
                         g_ux_cache.fart_promotion_choice);
    ux_draw_command_bar(&g_ux_cache.board, g_ux_cache.fart_mode,
                        g_ux_cache.fart_promotion_pending);
    ux_draw_persistence_notice(shown);

    if (ux_terminal(g_ux_cache.status))
        ux_draw_terminal_overlay(g_ux_cache.status,
                                 g_ux_cache.board.side_to_move);
    g_cpu_message_pending = 0;
}

static void ux_redraw_cached(void)
{
    ux_capture("chessfart_build11");
    ux_render_cached_base();
    vga_present();
}

static void ux_wait_for_close(int page_mode)
{
#ifndef CF_HOST_BUILD
    CfInputKey5 key;
    int page = 0;
    for (;;) {
        key = input5_poll_key();
        if (page_mode && (key == CF5_KEY_LEFT || key == CF5_KEY_RIGHT)) {
            page = page == 0 ? 1 : 0;
            ux_draw_help_page(page);
            vga_present();
        } else if (key == CF5_KEY_ESCAPE || key == CF5_KEY_ENTER ||
                   key == CF5_KEY_HELP || key == CF5_KEY_HISTORY ||
                   key == CF5_KEY_CREDITS) {
            break;
        }
    }
#else
    (void)page_mode;
#endif
}

void ux_show_help_modal(void)
{
    ux_capture("chessfart_build11_help");
    ux_draw_help_page(0);
    vga_present();
    ux_wait_for_close(1);
    ux_redraw_cached();
}

void ux_show_history_modal(void)
{
    ux_capture("chessfart_build11_history");
    ux_draw_history_screen();
    vga_present();
    ux_wait_for_close(0);
    ux_redraw_cached();
}

void ux_show_credits_modal(void)
{
    ux_capture("chessfart_build11_credits");
    ux_draw_credits_screen();
    vga_present();
    ux_wait_for_close(0);
    ux_redraw_cached();
}

static void ux_setup_demo(CfBoard *board, CfGasState *gas)
{
    board_clear(board);
    gas_init(gas);
    board_set_piece(board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(gas, 2, 2, 3U);
    gas_set(gas, 3, 3, 1U);
}

static void ux_run_attract(void)
{
    CfBoard before_board;
    CfBoard after_board;
    CfGasState before_gas;
    CfGasState after_gas;
    CfFartAction action;
    CfMoveList moves;
    ux_setup_demo(&before_board, &before_gas);
    after_board = before_board;
    after_gas = before_gas;
    moves.count = 0;

    ux_capture("chessfart_build11_attract");
    persistence_render_game(&before_board, &before_gas, 2, 2, 1, 2, 2,
                            &moves, CF_GAME_ONGOING, 0, CF_PIECE_QUEEN,
                            1, CF_FART_NE, CF_FART_PUSH, 0,
                            CF_PIECE_QUEEN, "CHARGED KNIGHT");
    ux_draw_retail_panel(&before_board, &before_gas, 2, 2, 1, 2, 2,
                         CF_GAME_ONGOING, "AIM NE TO PUSH", 1,
                         CF_FART_NE, CF_FART_PUSH, 0, CF_PIECE_QUEEN);
    ux_draw_command_bar(&before_board, 1, 0);
    vga_present();

    if (gas_make_fart(&after_board, &after_gas, 2, 2, CF_FART_NE,
                      CF_PIECE_QUEEN, &action)) {
        presentation_animate_fart(&before_board, &before_gas,
                                  &after_board, &after_gas, &action);
        persistence_render_game(&after_board, &after_gas, 2, 2, 0, 0, 0,
                                &moves, CF_GAME_ONGOING, 0, CF_PIECE_QUEEN,
                                0, CF_FART_NE, CF_FART_INVALID, 0,
                                CF_PIECE_QUEEN, "D4 BLASTED TO E5");
        ux_draw_retail_panel(&after_board, &after_gas, 2, 2, 0, 0, 0,
                             CF_GAME_ONGOING, "D4 BLASTED TO E5", 0,
                             CF_FART_N, CF_FART_INVALID,
                             0, CF_PIECE_QUEEN);
        ux_draw_command_bar(&after_board, 0, 0);
        vga_present();
    }
}

#ifdef CF_HOST_BUILD
static void ux_render_terminal_preview(void)
{
    CfBoard board;
    CfGasState gas;
    CfMoveList moves;
    board_init_starting_position(&board);
    gas_init(&gas);
    moves.count = 0;
    ux_capture("chessfart_build11_terminal");
    persistence_render_game(&board, &gas, 4, 1, 0, 0, 0, &moves,
                            CF_GAME_CHECKMATE, 0, CF_PIECE_QUEEN, 0,
                            CF_FART_N, CF_FART_INVALID, 0,
                            CF_PIECE_QUEEN, "CHECKMATE");
    ux_draw_retail_panel(&board, &gas, 4, 1, 0, 0, 0,
                         CF_GAME_CHECKMATE, "CHECKMATE", 0,
                         CF_FART_N, CF_FART_INVALID,
                         0, CF_PIECE_QUEEN);
    ux_draw_command_bar(&board, 0, 0);
    ux_draw_terminal_overlay(CF_GAME_CHECKMATE, CF_COLOR_BLACK);
    vga_present();
}
#endif

int ux_title_screen(const char *config_path)
{
    int menu = 0;
    int frame = 0;
#ifndef CF_HOST_BUILD
    CfInputKey5 key;
    clock_t last_input;
    CfMouseState mouse;
    unsigned old_buttons = 0U;
    unsigned pressed;
#endif
    if (vga_init() != 0) return 0;
    ux_capture("chessfart_build11_title");
    ux_draw_title(menu, frame);
    vga_present();
#ifdef CF_HOST_BUILD
    ux_run_attract();
    ux_capture("chessfart_build11_help");
    ux_draw_help_page(0);
    vga_present();
    ux_capture("chessfart_build11_credits");
    ux_draw_credits_screen();
    vga_present();
    ux_render_terminal_preview();
    ux_capture("chessfart_build11_title");
    ux_draw_title(menu, ++frame);
    vga_present();
    audio_play_event(CF_AUDIO_MENU_CONFIRM);
    (void)persistence_save_config(config_path, audio_get_config());
    vga_shutdown();
    return 1;
#else
    input5_init();
    last_input = clock();
    for (;;) {
        key = input5_poll_key();
        if (mouse_poll(&mouse) && mouse.present) {
            pressed = mouse.buttons & ~old_buttons;
            old_buttons = mouse.buttons;
            if ((pressed & 1U) != 0U &&
                ux_title_menu_hit_test(mouse.x, mouse.y, &menu))
                key = CF5_KEY_ENTER;
        }
        if (key != CF5_KEY_NONE) last_input = clock();
        if (key == CF5_KEY_ESCAPE) {
            (void)persistence_save_config(config_path, audio_get_config());
            vga_shutdown();
            return 0;
        }
        if (key == CF5_KEY_UP) {
            menu = (menu + CF_UI_TITLE_MENU_ITEM_COUNT - 1) %
                   CF_UI_TITLE_MENU_ITEM_COUNT;
            ++frame; audio_play_event(CF_AUDIO_CURSOR);
        } else if (key == CF5_KEY_DOWN) {
            menu = (menu + 1) % CF_UI_TITLE_MENU_ITEM_COUNT;
            ++frame; audio_play_event(CF_AUDIO_CURSOR);
        } else if (key == CF5_KEY_LEFT || key == CF5_KEY_RIGHT) {
            audio_cycle_device(); ++frame; audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
        } else if (key == CF5_KEY_FART) {
            audio_cycle_sfx_level(); ++frame; audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
        } else if (key == CF5_KEY_DIFFICULTY) {
            cpu_cycle_difficulty(&g_cpu_config); ++frame;
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
        } else if (key == CF5_KEY_HELP) {
            ux_draw_help_page(0); vga_present(); ux_wait_for_close(1); ++frame;
        } else if (key == CF5_KEY_CREDITS) {
            ux_draw_credits_screen(); vga_present(); ux_wait_for_close(0); ++frame;
        } else if (key == CF5_KEY_ENTER) {
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            if (menu == CF_UI_TITLE_ITEM_PLAY_CPU ||
                menu == CF_UI_TITLE_ITEM_PLAY_LOCAL) {
                g_match_mode = menu == CF_UI_TITLE_ITEM_PLAY_LOCAL ?
                               CF_MATCH_LOCAL : CF_MATCH_CPU;
                (void)persistence_save_config(config_path, audio_get_config());
                vga_shutdown();
                return 1;
            }
            if (menu == CF_UI_TITLE_ITEM_ATTRACT) ux_run_attract();
            else if (menu == CF_UI_TITLE_ITEM_HELP) {
                ux_draw_help_page(0); vga_present(); ux_wait_for_close(1);
            } else if (menu == CF_UI_TITLE_ITEM_CREDITS) {
                ux_draw_credits_screen(); vga_present(); ux_wait_for_close(0);
            } else if (menu == CF_UI_TITLE_ITEM_QUIT) {
                (void)persistence_save_config(config_path, audio_get_config());
                vga_shutdown();
                return 0;
            }
            ++frame;
        } else if ((clock() - last_input) > (clock_t)(12L * CLOCKS_PER_SEC)) {
            ux_run_attract();
            last_input = clock();
            ++frame;
        } else {
            continue;
        }
        ux_capture("chessfart_build11_title");
        ux_draw_title(menu, frame);
        vga_present();
    }
#endif
}

void ux_render_game(const CfBoard *board, const CfGasState *gas,
                    int cf, int cr, int hs, int sf, int sr,
                    const CfMoveList *moves, CfGameStatus status,
                    int pp, CfPieceType pc, int fm, CfFartDirection fd,
                    CfFartPreview fp, int fpp, CfPieceType fpc,
                    const char *message)
{
    if (board == 0 || gas == 0 || moves == 0) return;
    g_ux_cache.valid = 1;
    g_ux_cache.board = *board;
    g_ux_cache.gas = *gas;
    g_ux_cache.cursor_file = cf;
    g_ux_cache.cursor_rank = cr;
    g_ux_cache.has_selection = hs;
    g_ux_cache.selected_file = sf;
    g_ux_cache.selected_rank = sr;
    g_ux_cache.moves = *moves;
    g_ux_cache.status = status;
    g_ux_cache.promotion_pending = pp;
    g_ux_cache.promotion_choice = pc;
    g_ux_cache.fart_mode = fm;
    g_ux_cache.fart_direction = fd;
    g_ux_cache.fart_preview = fp;
    g_ux_cache.fart_promotion_pending = fpp;
    g_ux_cache.fart_promotion_choice = fpc;
    strncpy(g_ux_cache.message, message == 0 ? "" : message,
            sizeof(g_ux_cache.message) - 1U);
    g_ux_cache.message[sizeof(g_ux_cache.message) - 1U] = '\0';
    ux_capture("chessfart_build11");
    ux_render_cached_base();
}
