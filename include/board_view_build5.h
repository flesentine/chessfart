#ifndef CF_BOARD_VIEW_BUILD5_H
#define CF_BOARD_VIEW_BUILD5_H

#include "board.h"
#include "gas.h"

void board_view_render_build5(const CfBoard *board,
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
                              const char *message);

#endif
