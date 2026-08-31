#include "board_view_build7.h"
#include "font.h"
#include "input_build5.h"
#include "presentation.h"
#include "vga.h"
#ifdef CF_WEB_BUILD
#include <emscripten.h>
#endif

static void frame_delay(void)
{
#ifdef CF_WEB_BUILD
    emscripten_sleep(42);
#elif !defined(CF_HOST_BUILD)
    volatile unsigned long i;
    for (i = 0UL; i < 18000UL; ++i) { }
#endif
}

int presentation_title_screen(void)
{
    int menu = 0;
    int frame = 0;
#ifndef CF_HOST_BUILD
    CfInputKey5 key;
#endif

    if (vga_init() != 0) return 0;
    board_view_render_title7(menu, frame);
    vga_present();

#ifdef CF_HOST_BUILD
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
            board_view_render_title7(menu, frame);
            vga_present();
        } else if (key == CF5_KEY_ENTER) {
            vga_shutdown();
            return menu == 0;
        }
    }
#endif
}

void presentation_animate_fart(const CfBoard *before_board,
                               const CfGasState *before_gas,
                               const CfBoard *after_board,
                               const CfGasState *after_gas,
                               const CfFartAction *action)
{
    CfMoveList empty_moves;
    CfPresentationFx fx;
    const CfBoard *board;
    const CfGasState *gas;
    CfGameStatus status;
    int frame;

    empty_moves.count = 0;
    fx.active = 1;
    fx.action = *action;

    for (frame = 0; frame < 5; ++frame) {
        fx.frame = frame;
        if (frame < 3) {
            board = before_board;
            gas = before_gas;
        } else {
            board = after_board;
            gas = after_gas;
        }
        status = board_is_in_check(board, board->side_to_move) ?
                 CF_GAME_CHECK : CF_GAME_ONGOING;
        board_view_render_build7_fx(board, gas,
                                    action->actor_file, action->actor_rank,
                                    1, action->actor_file, action->actor_rank,
                                    &empty_moves, status,
                                    0, CF_PIECE_QUEEN,
                                    1, action->direction, action->result,
                                    0, CF_PIECE_QUEEN,
                                    frame < 3 ? "THRRRPP!" : "VENTILATED",
                                    &fx);
        vga_present();
        frame_delay();
    }
}

void presentation_render_game(const CfBoard *board,
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
    board_view_render_build7(board, gas, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             legal_moves, status, promotion_pending,
                             promotion_choice, fart_mode, fart_direction,
                             fart_preview, fart_promotion_pending,
                             fart_promotion_choice, message);
    vga_fill_rect(188, 27, 116, 10, 1);
    font_draw_text(190, 29, "BUILD 7", 4, 1);
}

int presentation_make_fart(CfBoard *board, CfGasState *gas,
                           int file, int rank, CfFartDirection direction,
                           CfPieceType promotion, CfFartAction *action)
{
    CfBoard before_board;
    CfGasState before_gas;
    CfFartAction local;

    if (board == 0 || gas == 0) return 0;
    before_board = *board;
    before_gas = *gas;

    if (!gas_make_fart(board, gas, file, rank, direction,
                       promotion, &local)) return 0;
    presentation_animate_fart(&before_board, &before_gas, board, gas, &local);
    if (action != 0) *action = local;
    return 1;
}
