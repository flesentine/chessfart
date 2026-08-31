#ifndef CF_PERSISTENCE_UI_H
#define CF_PERSISTENCE_UI_H

#include "audio_game.h"
#include "persistence.h"

int persistence_title_screen(const char *config_path);

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
                             const char *message);

#endif
