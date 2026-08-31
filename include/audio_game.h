#ifndef CF_AUDIO_GAME_H
#define CF_AUDIO_GAME_H

#include "audio.h"
#include "board.h"
#include "gas.h"
#include "input_build5.h"

int audio_title_screen(void);
CfInputKey5 audio_game_poll_key(void);

int audio_game_make_move(CfBoard *board, CfGasState *gas,
                         int from_file, int from_rank,
                         int to_file, int to_rank,
                         CfGasMove *made_move);
int audio_game_make_move_ex(CfBoard *board, CfGasState *gas,
                            int from_file, int from_rank,
                            int to_file, int to_rank,
                            CfPieceType promotion,
                            CfGasMove *made_move);
int audio_game_make_fart(CfBoard *board, CfGasState *gas,
                         int file, int rank,
                         CfFartDirection direction,
                         CfPieceType promotion,
                         CfFartAction *action);

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
                       const char *message);

#endif
