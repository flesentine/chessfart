#ifndef CF_BOARD_VIEW_BUILD7_H
#define CF_BOARD_VIEW_BUILD7_H

#include "board.h"
#include "gas.h"

typedef struct CfPresentationFx {
    int active;
    int frame;
    CfFartAction action;
} CfPresentationFx;

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
                              const char *message);

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
                                 const CfPresentationFx *fx);

void board_view_render_title7(int menu_index, int frame);

#endif
