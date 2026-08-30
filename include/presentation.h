#ifndef CF_PRESENTATION_H
#define CF_PRESENTATION_H

#include "board.h"
#include "gas.h"

int presentation_title_screen(void);
int presentation_make_fart(CfBoard *board, CfGasState *gas,
                           int file, int rank, CfFartDirection direction,
                           CfPieceType promotion, CfFartAction *action);

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
                              const char *message);

#endif
