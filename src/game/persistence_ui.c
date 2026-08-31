#include <stdio.h>

#include "audio.h"
#include "board_view_build7.h"
#include "font.h"
#include "input_build5.h"
#include "persistence_ui.h"
#include "vga.h"

static void draw_title_overlay(void)
{
    char line[32];
    const CfAudioConfig *config = audio_get_config();
    const CfAudioStats *stats = audio_get_stats();

    vga_fill_rect(0, 158, 320, 11, 0);
    sprintf(line, "AUDIO %s  SFX %s",
            audio_device_name(stats->actual_device),
            audio_level_name(config->sfx_level));
    font_draw_text(91, 160, line, 8, 1);
    vga_fill_rect(0, 192, 320, 8, 0);
    font_draw_text(10, 193, "BUILD 9  SAVE LOAD CONFIG", 6, 1);
}

static void render_title(int menu, int frame)
{
    board_view_render_title7(menu, frame);
    draw_title_overlay();
    vga_present();
}

int persistence_title_screen(const char *config_path)
{
    int menu = 0;
    int frame = 0;
#ifndef CF_HOST_BUILD
    CfInputKey5 key;
#endif

    if (vga_init() != 0) return 0;
    render_title(menu, frame);

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
            ++frame;
            audio_play_event(CF_AUDIO_CURSOR);
            render_title(menu, frame);
        } else if (key == CF5_KEY_LEFT || key == CF5_KEY_RIGHT) {
            audio_cycle_device();
            ++frame;
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
            render_title(menu, frame);
        } else if (key == CF5_KEY_FART) {
            audio_cycle_sfx_level();
            ++frame;
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
            render_title(menu, frame);
        } else if (key == CF5_KEY_ENTER) {
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            (void)persistence_save_config(config_path, audio_get_config());
            vga_shutdown();
            return menu == 0;
        }
    }
#endif
}

void persistence_render_game(const CfBoard *board,
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
    audio_game_render(board, gas, cursor_file, cursor_rank,
                      has_selection, selected_file, selected_rank,
                      legal_moves, status, promotion_pending,
                      promotion_choice, fart_mode, fart_direction,
                      fart_preview, fart_promotion_pending,
                      fart_promotion_choice, message);
    vga_fill_rect(188, 27, 116, 10, 1);
    font_draw_text(190, 29, "BUILD 9", 4, 1);
    vga_fill_rect(0, 188, 320, 12, 0);
    font_draw_text(10, 190, "BUILD 9  S SAVE  L LOAD", 6, 1);
}
