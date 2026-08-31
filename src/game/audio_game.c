#include <stdio.h>
#include <string.h>

#include "audio_game.h"
#include "board_view_build7.h"
#include "font.h"
#include "presentation.h"
#include "vga.h"

static CfGameStatus g_last_status = CF_GAME_ONGOING;
static int g_have_status;
static char g_last_message[32];

static int text_has(const char *text, const char *needle)
{
    if (text == 0 || needle == 0) return 0;
    return strstr(text, needle) != 0;
}

static int invalid_message(const char *message)
{
    return text_has(message, "ILLEGAL") ||
           text_has(message, "WRONG") ||
           text_has(message, "EMPTY") ||
           text_has(message, "NEED") ||
           text_has(message, "INVALID") ||
           text_has(message, "SELECT FIRST");
}

static void draw_audio_title_overlay(void)
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
    font_draw_text(10, 193, "BUILD 8  LEFT RIGHT DEVICE  F SFX", 6, 1);
}

static void render_title(int menu, int frame)
{
    board_view_render_title7(menu, frame);
    draw_audio_title_overlay();
    vga_present();
}

int audio_title_screen(void)
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
    vga_shutdown();
    return 1;
#else
    input5_init();
    for (;;) {
        key = input5_poll_key();
        if (key == CF5_KEY_ESCAPE) {
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
            render_title(menu, frame);
        } else if (key == CF5_KEY_FART) {
            audio_cycle_sfx_level();
            ++frame;
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            render_title(menu, frame);
        } else if (key == CF5_KEY_ENTER) {
            audio_play_event(CF_AUDIO_MENU_CONFIRM);
            vga_shutdown();
            return menu == 0;
        }
    }
#endif
}

CfInputKey5 audio_game_poll_key(void)
{
    CfInputKey5 key = input5_poll_key();
    switch (key) {
    case CF5_KEY_UP:
    case CF5_KEY_DOWN:
    case CF5_KEY_LEFT:
    case CF5_KEY_RIGHT:
    case CF5_KEY_UP_LEFT:
    case CF5_KEY_UP_RIGHT:
    case CF5_KEY_DOWN_LEFT:
    case CF5_KEY_DOWN_RIGHT:
        audio_play_event(CF_AUDIO_CURSOR);
        break;
    case CF5_KEY_ENTER:
        audio_play_event(CF_AUDIO_SELECT);
        break;
    case CF5_KEY_FART:
        audio_play_event(CF_AUDIO_FART_READY);
        break;
    default:
        break;
    }
    return key;
}

static void sound_for_move(const CfGasMove *move)
{
    if ((move->chess_move.flags & CF_MOVE_PROMOTION) != 0U) {
        audio_play_event(CF_AUDIO_PROMOTION);
    } else if (move->chess_move.captured.type != CF_PIECE_NONE) {
        audio_play_event(CF_AUDIO_CAPTURE);
    } else {
        audio_play_event(CF_AUDIO_MOVE);
    }
}

int audio_game_make_move(CfBoard *board, CfGasState *gas,
                         int from_file, int from_rank,
                         int to_file, int to_rank,
                         CfGasMove *made_move)
{
    CfGasMove local;
    if (!gas_make_move(board, gas, from_file, from_rank,
                       to_file, to_rank, &local)) return 0;
    sound_for_move(&local);
    if (made_move != 0) *made_move = local;
    return 1;
}

int audio_game_make_move_ex(CfBoard *board, CfGasState *gas,
                            int from_file, int from_rank,
                            int to_file, int to_rank,
                            CfPieceType promotion,
                            CfGasMove *made_move)
{
    CfGasMove local;
    if (!gas_make_move_ex(board, gas, from_file, from_rank,
                          to_file, to_rank, promotion, &local)) return 0;
    sound_for_move(&local);
    if (made_move != 0) *made_move = local;
    return 1;
}

int audio_game_make_fart(CfBoard *board, CfGasState *gas,
                         int file, int rank,
                         CfFartDirection direction,
                         CfPieceType promotion,
                         CfFartAction *action)
{
    CfBoard before_board;
    CfGasState before_gas;
    CfFartAction local;

    if (board == 0 || gas == 0) return 0;
    before_board = *board;
    before_gas = *gas;
    if (!gas_make_fart(board, gas, file, rank, direction,
                       promotion, &local)) return 0;

    audio_play_fart_result(local.result);
    presentation_animate_fart(&before_board, &before_gas,
                              board, gas, &local);
    if (local.result == CF_FART_PROMOTION)
        audio_play_event(CF_AUDIO_PROMOTION);
    if (action != 0) *action = local;
    return 1;
}

static void maybe_play_status(CfGameStatus status)
{
    if (!g_have_status) {
        g_last_status = status;
        g_have_status = 1;
        return;
    }
    if (status == g_last_status) return;
    if (status == CF_GAME_CHECKMATE)
        audio_play_event(CF_AUDIO_CHECKMATE);
    else if (status == CF_GAME_CHECK)
        audio_play_event(CF_AUDIO_CHECK);
    g_last_status = status;
}

static void maybe_play_message(const char *message)
{
    if (message == 0) return;
    if (strcmp(message, g_last_message) == 0) return;
    if (invalid_message(message)) audio_play_event(CF_AUDIO_INVALID);
    strncpy(g_last_message, message, sizeof(g_last_message) - 1U);
    g_last_message[sizeof(g_last_message) - 1U] = '\0';
}

void audio_game_render(const CfBoard *board,
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
    const CfAudioConfig *config;
    const CfAudioStats *stats;

    presentation_render_game(board, gas, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             legal_moves, status, promotion_pending,
                             promotion_choice, fart_mode, fart_direction,
                             fart_preview, fart_promotion_pending,
                             fart_promotion_choice, message);

    vga_fill_rect(188, 27, 116, 10, 1);
    font_draw_text(190, 29, "BUILD 8", 4, 1);
    config = audio_get_config();
    stats = audio_get_stats();
    vga_fill_rect(188, 129, 116, 9, 1);
    sprintf(line, "AUDIO %s %s", audio_device_name(stats->actual_device),
            audio_level_name(config->sfx_level));
    font_draw_text(190, 130, line, 8, 1);
    vga_fill_rect(0, 188, 320, 12, 0);
    font_draw_text(10, 190, "BUILD 8  DIGITAL AUDIO ONLINE", 6, 1);

    maybe_play_status(status);
    maybe_play_message(message);
}
