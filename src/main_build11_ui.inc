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

static void ux_stamp_build11(void)
{
    /* Retail renderer deliberately hides internal build numbering. */
}

static void ux_draw_box(int x, int y, int w, int h)
{
    vga_fill_rect(x, y, w, h, UX_COL_BG);
    vga_fill_rect(x, y, w, 2, UX_COL_CURSOR);
    vga_fill_rect(x, y + h - 2, w, 2, UX_COL_CURSOR);
    vga_fill_rect(x, y, 2, h, UX_COL_CURSOR);
    vga_fill_rect(x + w - 2, y, 2, h, UX_COL_CURSOR);
}

static void ux_draw_help_page(int page)
{
    ux_draw_box(14, 12, 292, 176);
    font_draw_text(92, 20, "CHESS FART HELP", UX_COL_CURSOR, 1);
    if (page == 0) {
        font_draw_text(27, 40, "ARROWS  MOVE CURSOR", UX_COL_GAS, 1);
        font_draw_text(27, 51, "ENTER/SPACE  SELECT / CONFIRM", UX_COL_GAS, 1);
        font_draw_text(27, 62, "F  FART MODE    S SAVE    L LOAD", UX_COL_GAS, 1);
        font_draw_text(27, 73, "H/? HELP   TAB/M HISTORY   C CREDITS", UX_COL_GAS, 1);
        font_draw_text(27, 91, "MOVE +1 GAS   CAPTURE +2 GAS", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 102, "2 GAS = ONE FART ACTION", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 113, "EMPTY TARGET = PUFF", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 124, "PIECE TARGET = PUSH ONE SQUARE", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 135, "FARTS NEVER CAPTURE DIRECTLY", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 153, "RIGHT CLICK = FART   LEFT = BOARD", UX_COL_MUTED, 1);
        font_draw_text(71, 170, "LEFT/RIGHT PAGE  ENTER CLOSE", UX_COL_MUTED, 1);
    } else {
        font_draw_text(27, 40, "ALL NORMAL CHESS RULES APPLY", UX_COL_GAS, 1);
        font_draw_text(27, 51, "FART PUSHES MUST KEEP YOUR KING SAFE", UX_COL_GAS, 1);
        font_draw_text(27, 62, "KINGS MAY BE PUSHED IF POSITION LEGAL", UX_COL_GAS, 1);
        font_draw_text(27, 73, "PUSHED PAWN CAN PROMOTE Q/R/B/N", UX_COL_GAS, 1);
        font_draw_text(27, 84, "PUSHING HOME KING/ROOK KILLS CASTLING", UX_COL_GAS, 1);
        font_draw_text(27, 102, "CPU BLACK SEARCHES MOVES + FARTS", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 113, "D ON TITLE: EASY / MED / HARD", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 131, "GAS IS PART OF THREEFOLD IDENTITY", UX_COL_PANEL_EDGE, 1);
        font_draw_text(27, 142, "SAVE STORES EXACT GAS + REP HISTORY", UX_COL_PANEL_EDGE, 1);
        font_draw_text(61, 170, "LEFT/RIGHT PAGE  ENTER CLOSE", UX_COL_MUTED, 1);
    }
}

static void ux_draw_credits_screen(void)
{
    ux_draw_box(34, 24, 252, 150);
    font_draw_text(101, 38, "CHESS FART", UX_COL_CURSOR, 1);
    font_draw_text(70, 60, "A FLESENTINE DOS PRODUCTION", UX_COL_PANEL_EDGE, 1);
    font_draw_text(61, 82, "320x200 VGA  /  256 COLORS", UX_COL_GAS, 1);
    font_draw_text(66, 96, "SOUND BLASTER + PC SPEAKER", UX_COL_GAS, 1);
    font_draw_text(75, 110, "OPEN WATCOM C89 TARGET", UX_COL_GAS, 1);
    font_draw_text(72, 132, "CHECK. MATE. VENTILATE.", UX_COL_GOLD, 1);
    font_draw_text(91, 156, "ENTER / ESC CLOSE", UX_COL_MUTED, 1);
}

static void ux_draw_history_screen(void)
{
    int first;
    int i;
    const char *line;
    ux_draw_box(24, 16, 272, 168);
    font_draw_text(105, 27, "ACTION LOG", UX_COL_CURSOR, 1);
    if (g_ux_history.count == 0) {
        font_draw_text(82, 91, "NO ACTIONS YET", UX_COL_PANEL_EDGE, 1);
    } else {
        first = g_ux_history.count > 12 ? g_ux_history.count - 12 : 0;
        for (i = first; i < g_ux_history.count; ++i) {
            line = ux_history_get(&g_ux_history, i);
            if (line != 0)
                font_draw_text(35, 46 + (i - first) * 10, line, UX_COL_GAS, 1);
        }
    }
    font_draw_text(91, 169, "TAB/M / ENTER CLOSE", UX_COL_MUTED, 1);
}

static void ux_draw_terminal_overlay(CfGameStatus status,
                                     CfPieceColor side_to_move)
{
    const char *title = ux_terminal_title(status, side_to_move);
    ux_draw_box(42, 69, 236, 62);
    font_draw_text(91, 82, title, UX_COL_CURSOR, 1);
    if (status == CF_GAME_CHECKMATE)
        font_draw_text(81, 101, "KING HAS NO LEGAL ESCAPE", UX_COL_PANEL_EDGE, 1);
    else
        font_draw_text(76, 101, board_game_status_name(status), UX_COL_PANEL_EDGE, 1);
    font_draw_text(79, 118, "H HELP   M LOG   ESC QUIT", UX_COL_MUTED, 1);
}

static void ux_draw_title(int menu, int frame)
{
    static const char *items[5] = {
        "PLAY CPU", "ATTRACT DEMO", "HELP / RULES", "CREDITS", "QUIT TO DOS"
    };
    char line[40];
    const CfAudioConfig *config = audio_get_config();
    const CfAudioStats *stats = audio_get_stats();
    int i;

    board_view_render_title7(menu == 4 ? 1 : 0, frame);
    vga_fill_rect(69, 102, 182, 78, UX_COL_PANEL_EDGE);
    vga_fill_rect(71, 104, 178, 74, UX_COL_PANEL);
    vga_fill_rect(71, 104, 178, 1, UX_COL_GOLD);

    for (i = 0; i < 5; ++i) {
        sprintf(line, "%c %s", i == menu ? '>' : ' ', items[i]);
        font_draw_text(94, 111 + i * 12, line,
                       i == menu ? UX_COL_GAS : UX_COL_TEXT, 1);
    }

    sprintf(line, "AUDIO %s %s   CPU %s",
            audio_device_name(stats->actual_device),
            audio_level_name(config->sfx_level),
            cpu_difficulty_name(g_cpu_config.difficulty));
    font_draw_text(74, 89, line, UX_COL_MUTED, 1);
    font_draw_text(43, 187, "D CPU   F SFX   LEFT/RIGHT AUDIO   MOUSE OPTIONAL",
                   UX_COL_MUTED, 1);
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

static void ux_draw_action_two_lines(int x, int y,
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
    font_draw_text_clipped(x, CF_UI_HUD_ACTION_LINE2_Y,
                           second, color, 1, CF_UI_HUD_DIVIDER_W);
}

static void ux_draw_legacy_panel(const CfBoard *board,
                                 const CfGasState *gas,
                                 int cursor_file,
                                 int cursor_rank,
                                 int has_selection,
                                 int selected_file,
                                 int selected_rank,
                                 CfGameStatus status,
                                 const char *action_text)
{
    const CfPiece *piece;
    const CfAudioConfig *config = audio_get_config();
    const CfAudioStats *audio_stats = audio_get_stats();
    int pf = has_selection ? selected_file : cursor_file;
    int pr = has_selection ? selected_rank : cursor_rank;
    cf_u8 piece_gas = 0U;
    char square[3];
    char line[24];

    ux_panel_outline();
    font_draw_text(180, 31, "TURN", UX_COL_GOLD, 1);
    font_draw_text(223, 31, board_piece_color_name(board->side_to_move), UX_COL_TEXT, 1);
    font_draw_text(180, 42, "STATE", UX_COL_MUTED, 1);
    font_draw_text(223, 42, board_game_status_name(status),
                   status == CF_GAME_CHECK || status == CF_GAME_CHECKMATE ?
                   UX_COL_GOLD : UX_COL_GREEN, 1);
    vga_fill_rect(180, 52, 123, 1, UX_COL_PANEL_LINE);

    font_draw_text(180, 58, has_selection ? "SELECTED PIECE" : "CURSOR PIECE",
                   UX_COL_SELECTED, 1);
    ux_square_name(square, pf, pr);
    piece = board_piece_at(board, pf, pr);
    vga_fill_rect(181, 68, 22, 22, UX_COL_PANEL_EDGE);
    vga_fill_rect(183, 70, 18, 18, UX_COL_PANEL_SOFT);
    if (piece != 0 && piece->type != CF_PIECE_NONE) {
        ui_assets_draw_piece(183, 71, piece, UX_COL_PANEL_SOFT);
        font_draw_text(208, 69, board_piece_type_name(piece->type), UX_COL_TEXT, 1);
        piece_gas = gas_at(gas, pf, pr);
    } else {
        font_draw_text(208, 69, "EMPTY", UX_COL_MUTED, 1);
    }
    font_draw_text(208, 80, square, UX_COL_CURSOR, 1);

    vga_fill_rect(180, 94, 123, 1, UX_COL_PANEL_LINE);
    font_draw_text(180, 99, "GAS RESERVE", UX_COL_GOLD, 1);
    ux_draw_gas_pips(181, 110, piece_gas);
    sprintf(line, "%u/3", (unsigned)piece_gas);
    font_draw_text(232, 110, line, piece_gas >= 2U ? UX_COL_GAS : UX_COL_MUTED, 1);

    vga_fill_rect(180, 121, 123, 1, UX_COL_PANEL_LINE);
    font_draw_text(180, 126, "AUDIO", UX_COL_SELECTED, 1);
    sprintf(line, "%s %s", audio_device_name(audio_stats->actual_device),
            audio_level_name(config->sfx_level));
    font_draw_text(180, 137, line, UX_COL_TEXT, 1);
    font_draw_text(244, 126, "CPU", UX_COL_SELECTED, 1);
    font_draw_text(244, 137, cpu_difficulty_name(g_cpu_config.difficulty), UX_COL_GOLD, 1);

    vga_fill_rect(180, 147, 123, 1, UX_COL_PANEL_LINE);
    font_draw_text(180, 152, "LAST ACTION", UX_COL_SELECTED, 1);
    if (action_text != 0 && action_text[0] != '\0') {
        strncpy(line, action_text, sizeof(line) - 1U);
        line[sizeof(line) - 1U] = '\0';
        font_draw_text(180, 163, line, UX_COL_TEXT, 1);
    } else {
        font_draw_text(180, 163, "NONE YET", UX_COL_MUTED, 1);
    }
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
                                 int fart_mode)
{
    const CfPiece *piece;
    const char *mode_label;
    int pf;
    int pr;
    cf_u8 piece_gas;
    char square[3];
    char line[16];

    if (fart_mode) {
        ux_draw_legacy_panel(board, gas, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             status, action_text);
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
    font_draw_text(CF_UI_HUD_VALUE_X, CF_UI_HUD_STATE_Y,
                   board_game_status_name(status),
                   status == CF_GAME_CHECK || status == CF_GAME_CHECKMATE ?
                   CF_UI_COL_CHECK : UX_COL_GREEN, 1);
    vga_fill_rect(CF_UI_HUD_LABEL_X, CF_UI_HUD_DIVIDER1_Y,
                  CF_UI_HUD_DIVIDER_W, 1, UX_COL_PANEL_LINE);

    font_draw_text(CF_UI_HUD_LABEL_X, CF_UI_HUD_PIECE_LABEL_Y,
                   mode_label,
                   has_selection ? UX_COL_SELECTED :
                   (piece != 0 && piece->type != CF_PIECE_NONE ?
                    UX_COL_CURSOR : UX_COL_MUTED), 1);
    vga_fill_rect(CF_UI_HUD_PIECE_BOX_X, CF_UI_HUD_PIECE_BOX_Y,
                  22, 22, UX_COL_PANEL_EDGE);
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
                             action_text, UX_COL_TEXT);
}

static void ux_draw_legacy_command_bar(void)
{
    static const char *labels[8] = {
        "F FART", "S SAVE", "L LOAD", "H HELP",
        "M LOG", "C CRED", "D CPU", "ESC"
    };
    int i;
    int x;

    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, CF_UI_COMMAND_H, UX_COL_BG);
    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, 1, UX_COL_GOLD);
    for (i = 0; i < 8; ++i) {
        x = i * 40;
        vga_fill_rect(x + 1, CF_UI_COMMAND_Y + 2, 38, 15, UX_COL_PANEL_EDGE);
        vga_fill_rect(x + 2, CF_UI_COMMAND_Y + 3, 36, 13, UX_COL_PANEL_SOFT);
        font_draw_text(x + 3, CF_UI_COMMAND_Y + 6, labels[i],
                       i == 0 ? UX_COL_GAS : UX_COL_TEXT, 1);
    }
}

static void ux_draw_command_bar(const CfBoard *board, int fart_mode)
{
    char line[16];

    if (fart_mode) {
        ux_draw_legacy_command_bar();
        return;
    }

    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, CF_UI_COMMAND_H, UX_COL_BG);
    vga_fill_rect(CF_UI_COMMAND_X, CF_UI_COMMAND_Y,
                  CF_UI_COMMAND_W, 1, UX_COL_GOLD);
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
    action = shown != 0 && shown[0] != '\0' ? shown : last;
    ux_draw_retail_panel(&g_ux_cache.board, &g_ux_cache.gas,
                         g_ux_cache.cursor_file, g_ux_cache.cursor_rank,
                         g_ux_cache.has_selection,
                         g_ux_cache.selected_file, g_ux_cache.selected_rank,
                         g_ux_cache.status, action, g_ux_cache.fart_mode);
    ux_draw_command_bar(&g_ux_cache.board, g_ux_cache.fart_mode);

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
                         CF_GAME_ONGOING, "AIM NE TO PUSH", 1);
    ux_draw_command_bar(&before_board, 1);
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
                             CF_GAME_ONGOING, "D4 BLASTED TO E5", 0);
        ux_draw_command_bar(&after_board, 0);
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
                         CF_GAME_CHECKMATE, "CHECKMATE", 0);
    ux_draw_command_bar(&board, 0);
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
            if ((pressed & 1U) != 0U && mouse.x >= 69 && mouse.x < 251 &&
                mouse.y >= 104 && mouse.y < 178) {
                menu = (mouse.y - 110) / 12;
                if (menu < 0) menu = 0;
                if (menu > 4) menu = 4;
                key = CF5_KEY_ENTER;
            }
        }
        if (key != CF5_KEY_NONE) last_input = clock();
        if (key == CF5_KEY_ESCAPE) {
            (void)persistence_save_config(config_path, audio_get_config());
            vga_shutdown();
            return 0;
        }
        if (key == CF5_KEY_UP) {
            menu = (menu + 4) % 5;
            ++frame; audio_play_event(CF_AUDIO_CURSOR);
        } else if (key == CF5_KEY_DOWN) {
            menu = (menu + 1) % 5;
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
            if (menu == 0) {
                (void)persistence_save_config(config_path, audio_get_config());
                vga_shutdown();
                return 1;
            }
            if (menu == 1) ux_run_attract();
            else if (menu == 2) { ux_draw_help_page(0); vga_present(); ux_wait_for_close(1); }
            else if (menu == 3) { ux_draw_credits_screen(); vga_present(); ux_wait_for_close(0); }
            else {
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
