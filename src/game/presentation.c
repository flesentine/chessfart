#include "board_view.h"
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
        board_view_render_game_fx(board, gas,
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
    board_view_render_game(board, gas, cursor_file, cursor_rank,
                           has_selection, selected_file, selected_rank,
                           legal_moves, status, promotion_pending,
                           promotion_choice, fart_mode, fart_direction,
                           fart_preview, fart_promotion_pending,
                           fart_promotion_choice, message);
}

