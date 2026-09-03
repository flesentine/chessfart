#ifndef CF_UI_ASSETS_H
#define CF_UI_ASSETS_H

#include "board.h"

#define CF_UI_PIECE_W 18
#define CF_UI_PIECE_H 16
#define CF_UI_PUFF_W 16
#define CF_UI_PUFF_H 16

void ui_assets_draw_piece(int x, int y, const CfPiece *piece, cf_u8 background);
void ui_assets_draw_piece_overlay(int x, int y, const CfPiece *piece);
void ui_assets_draw_puff(int x, int y, int frame);

#endif
