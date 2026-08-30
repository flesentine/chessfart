#ifndef CF_BOARD_VIEW_H
#define CF_BOARD_VIEW_H

#include "board.h"

void board_view_render_build3(const CfBoard *board,
                              int cursor_file,
                              int cursor_rank,
                              int has_selection,
                              int selected_file,
                              int selected_rank,
                              const CfMoveList *legal_moves,
                              const char *message);

#endif
