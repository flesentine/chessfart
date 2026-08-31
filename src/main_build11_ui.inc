#include <time.h>

#ifdef CF_HOST_BUILD
void vga_host_capture(const char *stem);
static void ux_capture(const char *stem) { vga_host_capture(stem); }
#else
static void ux_capture(const char *stem) { (void)stem; }
#endif

static void ux_stamp_build11(void)
{
    vga_fill_rect(188, 27, 116, 10, 1);
    font_draw_text(190, 29, "BUILD 11", 4, 1);
}

static void ux_draw_box(int x, int y, int w, int h)
{
    vga_fill_rect(x, y, w, h, 0);
    vga_fill_rect(x, y, w, 2, 15);
    vga_fill_rect(x, y + h - 2, w, 2, 15);
    vga_fill_rect(x, y, 2, h, 15);
    vga_fill_rect(x + w - 2, y, 2, h, 15);
}

static void ux_draw_help_page(int page)
{
    ux_draw_box(14, 12, 292, 176);
    font_draw_text(92, 20, "CHESS FART HELP", 15, 1);
    if (page == 0) {
        font_draw_text(27, 40, "ARROWS  MOVE CURSOR", 8, 1);
        font_draw_text(27, 51, "ENTER/SPACE  SELECT / CONFIRM", 8, 1);
        font_draw_text(27, 62, "F  FART MODE    S SAVE    L LOAD", 8, 1);
        font_draw_text(27, 73, "H/? HELP   TAB/M HISTORY   C CREDITS", 8, 1);
        font_draw_text(27, 91, "MOVE +1 GAS   CAPTURE +2 GAS", 10, 1);
        font_draw_text(27, 102, "2 GAS = ONE FART ACTION", 10, 1);
        font_draw_text(27, 113, "EMPTY TARGET = PUFF", 10, 1);
        font_draw_text(27, 124, "PIECE TARGET = PUSH ONE SQUARE", 10, 1);
        font_draw_text(27, 135, "FARTS NEVER CAPTURE DIRECTLY", 10, 1);
        font_draw_text(27, 153, "RIGHT CLICK = FART   LEFT = BOARD", 6, 1);
        font_draw_text(71, 170, "LEFT/RIGHT PAGE  ENTER CLOSE", 6, 1);
    } else {
        font_draw_text(27, 40, "ALL NORMAL CHESS RULES APPLY", 8, 1);
        font_draw_text(27, 51, "FART PUSHES MUST KEEP YOUR KING SAFE", 8, 1);
        font_draw_text(27, 62, "KINGS MAY BE PUSHED IF POSITION LEGAL", 8, 1);
        font_draw_text(27, 73, "PUSHED PAWN CAN PROMOTE Q/R/B/N", 8, 1);
        font_draw_text(27, 84, "PUSHING HOME KING/ROOK KILLS CASTLING", 8, 1);
        font_draw_text(27, 102, "CPU BLACK SEARCHES MOVES + FARTS", 10, 1);
        font_draw_text(27, 113, "D ON TITLE: EASY / MED / HARD", 10, 1);
        font_draw_text(27, 131, "GAS IS PART OF THREEFOLD IDENTITY", 10, 1);
        font_draw_text(27, 142, "SAVE STORES EXACT GAS + REP HISTORY", 10, 1);
        font_draw_text(61, 170, "LEFT/RIGHT PAGE  ENTER CLOSE", 6, 1);
    }
}

static void ux_draw_credits_screen(void)
{
    ux_draw_box(34, 24, 252, 150);
    font_draw_text(101, 38, "CHESS FART", 15, 1);
    font_draw_text(70, 60, "A FLESENTINE DOS PRODUCTION", 10, 1);
    font_draw_text(61, 82, "320x200 VGA  /  256 COLORS", 8, 1);
    font_draw_text(66, 96, "SOUND BLASTER + PC SPEAKER", 8, 1);
    font_draw_text(75, 110, "OPEN WATCOM C89 TARGET", 8, 1);
    font_draw_text(72, 132, "CHECK. MATE. VENTILATE.", 14, 1);
    font_draw_text(91, 156, "ENTER / ESC CLOSE", 6, 1);
}

static void ux_draw_history_screen(void)
{
    int first;
    int i;
    const char *line;
    ux_draw_box(24, 16, 272, 168);
    font_draw_text(105, 27, "ACTION LOG", 15, 1);
    if (g_ux_history.count == 0) {
        font_draw_text(82, 91, "NO ACTIONS YET", 8, 1);
    } else {
        first = g_ux_history.count > 12 ? g_ux_history.count - 12 : 0;
        for (i = first; i < g_ux_history.count; ++i) {
            line = ux_history_get(&g_ux_history, i);
            if (line != 0)
                font_draw_text(35, 46 + (i - first) * 10, line, 8, 1);
        }
    }
    font_draw_text(91, 169, "TAB/M / ENTER CLOSE", 6, 1);
}

static void ux_draw_terminal_overlay(CfGameStatus status,
                                     CfPieceColor side_to_move)
{
    const char *title = ux_terminal_title(status, side_to_move);
    ux_draw_box(42, 69, 236, 62);
    font_draw_text(91, 82, title, 15, 1);
    if (status == CF_GAME_CHECKMATE)
        font_draw_text(81, 101, "KING HAS NO LEGAL ESCAPE", 10, 1);
    else
        font_draw_text(76, 101, board_game_status_name(status), 10, 1);
    font_draw_text(79, 118, "H HELP   M LOG   ESC QUIT", 6, 1);
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
    vga_fill_rect(44, 112, 232, 76, 0);
    for (i = 0; i < 5; ++i) {
        sprintf(line, "%c %s", i == menu ? '>' : ' ', items[i]);
        font_draw_text(91, 117 + i * 12, line, i == menu ? 15 : 8, 1);
    }
    vga_fill_rect(0, 92, 320, 16, 0);
    sprintf(line, "AUDIO %s %s   CPU %s",
            audio_device_name(stats->actual_device),
            audio_level_name(config->sfx_level),
            cpu_difficulty_name(g_cpu_config.difficulty));
    font_draw_text(67, 97, line, 8, 1);
    vga_fill_rect(0, 192, 320, 8, 0);
    font_draw_text(8, 193, "BUILD 11  D CPU  F SFX  H HELP  MOUSE OPTIONAL", 6, 1);
}

static void ux_render_cached_base(void)
{
    const char *shown;
    const char *last;
    char short_line[20];
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
    vga_fill_rect(188, 27, 116, 10, 1);
    font_draw_text(190, 29, "BUILD 11", 4, 1);
    vga_fill_rect(188, 138, 116, 19, 1);
    sprintf(short_line, "CPU %s D%d N%lu",
            cpu_difficulty_name(g_cpu_config.difficulty),
            g_cpu_stats.depth_completed, g_cpu_stats.nodes);
    font_draw_text(190, 140, short_line, 8, 1);
    sprintf(short_line, "CUT %lu S%d", g_cpu_stats.cutoffs, g_cpu_stats.score);
    font_draw_text(190, 149, short_line, 6, 1);
    vga_fill_rect(188, 158, 116, 27, 1);
    font_draw_text(190, 159, "LAST ACTION", 6, 1);
    last = g_ux_history.count > 0 ?
           ux_history_get(&g_ux_history, g_ux_history.count - 1) : "NONE";
    strncpy(short_line, last == 0 ? "NONE" : last, sizeof(short_line) - 1U);
    short_line[sizeof(short_line) - 1U] = '\0';
    font_draw_text(190, 169, short_line, 8, 1);
    vga_fill_rect(0, 188, 320, 12, 0);
    font_draw_text(7, 190, "BUILD 11  H HELP  TAB/M LOG  C CREDITS  S/L SAVE", 6, 1);
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
                            CF_PIECE_QUEEN, "ATTRACT: CHARGED KNIGHT");
    ux_stamp_build11();
    vga_present();
    if (gas_make_fart(&after_board, &after_gas, 2, 2, CF_FART_NE,
                      CF_PIECE_QUEEN, &action)) {
        presentation_animate_fart(&before_board, &before_gas,
                                  &after_board, &after_gas, &action);
        persistence_render_game(&after_board, &after_gas, 2, 2, 0, 0, 0,
                                &moves, CF_GAME_ONGOING, 0, CF_PIECE_QUEEN,
                                0, CF_FART_NE, CF_FART_INVALID, 0,
                                CF_PIECE_QUEEN, "DEMO: D4 BLASTED TO E5");
        ux_stamp_build11();
        vga_fill_rect(0, 188, 320, 12, 0);
        font_draw_text(37, 190, "ATTRACT DEMO - CHECK. MATE. VENTILATE.", 10, 1);
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
    ux_stamp_build11();
    vga_fill_rect(0, 188, 320, 12, 0);
    font_draw_text(37, 190, "BUILD 11  TERMINAL STATE  H HELP  M LOG", 6, 1);
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
            if ((pressed & 1U) != 0U && mouse.x >= 44 && mouse.x < 276 &&
                mouse.y >= 112 && mouse.y < 184) {
                menu = (mouse.y - 112) / 12;
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
