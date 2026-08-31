static void draw_cpu_title_overlay(void)
{
    char line[40];
    const CfAudioConfig *config = audio_get_config();
    const CfAudioStats *stats = audio_get_stats();
    vga_fill_rect(0, 154, 320, 15, 0);
    sprintf(line, "AUDIO %s %s  CPU %s",
            audio_device_name(stats->actual_device),
            audio_level_name(config->sfx_level),
            cpu_difficulty_name(g_cpu_config.difficulty));
    font_draw_text(72, 158, line, 8, 1);
    vga_fill_rect(0, 192, 320, 8, 0);
    font_draw_text(10, 193, "BUILD 10  D CPU  LEFT RIGHT AUDIO  F SFX", 6, 1);
}

static void render_cpu_title(int menu, int frame)
{
    board_view_render_title7(menu, frame);
    draw_cpu_title_overlay();
    vga_present();
}

int cpu_title_screen(const char *config_path)
{
    int menu = 0;
    int frame = 0;
#ifndef CF_HOST_BUILD
    CfInputKey5 key;
#endif
    if (vga_init() != 0) return 0;
    render_cpu_title(menu, frame);
#ifdef CF_HOST_BUILD
    audio_play_event(CF_AUDIO_MENU_CONFIRM);
    (void)persistence_save_config(config_path, audio_get_config());
    vga_shutdown();
    return 1;
#else
    input5_init();
    for (;;) {
        key = input5_poll_key();
        if (key == CF5_KEY_ESCAPE) {
            (void)persistence_save_config(config_path, audio_get_config());
            vga_shutdown();
            return 0;
        }
        if (key == CF5_KEY_UP || key == CF5_KEY_DOWN) {
            menu = menu == 0 ? 1 : 0;
            ++frame; audio_play_event(CF_AUDIO_CURSOR); render_cpu_title(menu, frame);
        } else if (key == CF5_KEY_LEFT || key == CF5_KEY_RIGHT) {
            audio_cycle_device(); ++frame; audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
            render_cpu_title(menu, frame);
        } else if (key == CF5_KEY_FART) {
            audio_cycle_sfx_level(); ++frame; audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
            render_cpu_title(menu, frame);
        } else if (key == CF5_KEY_DIFFICULTY) {
            cpu_cycle_difficulty(&g_cpu_config); ++frame;
            audio_play_event(CF_AUDIO_MENU_CONFIRM); render_cpu_title(menu, frame);
        } else if (key == CF5_KEY_ENTER) {
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
            vga_shutdown();
            return menu == 0;
        }
    }
#endif
}

void cpu_render_game(const CfBoard *board, const CfGasState *gas,
                     int cf, int cr, int hs, int sf, int sr,
                     const CfMoveList *moves, CfGameStatus status,
                     int pp, CfPieceType pc, int fm, CfFartDirection fd,
                     CfFartPreview fp, int fpp, CfPieceType fpc,
                     const char *message)
{
    char line[40];
    const char *shown = g_cpu_message_pending ? g_cpu_message : message;
    persistence_render_game(board, gas, cf, cr, hs, sf, sr, moves, status,
                            pp, pc, fm, fd, fp, fpp, fpc, shown);
    vga_fill_rect(188, 27, 116, 10, 1);
    font_draw_text(190, 29, "BUILD 10", 4, 1);
    vga_fill_rect(188, 138, 116, 19, 1);
    sprintf(line, "CPU %s D%d N%lu", cpu_difficulty_name(g_cpu_config.difficulty),
            g_cpu_stats.depth_completed, g_cpu_stats.nodes);
    font_draw_text(190, 140, line, 8, 1);
    sprintf(line, "CUT %lu SCORE %d", g_cpu_stats.cutoffs, g_cpu_stats.score);
    font_draw_text(190, 149, line, 6, 1);
    vga_fill_rect(0, 188, 320, 12, 0);
    font_draw_text(10, 190, "BUILD 10  YOU WHITE  CPU BLACK  S SAVE  L LOAD", 6, 1);
    g_cpu_message_pending = 0;
}
