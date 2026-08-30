#include <stdio.h>

#include "board.h"
#include "board_view.h"
#include "input.h"
#include "vga.h"

int main(void)
{
    CfBoard board;
    CfInputKey key;
    const CfPiece *piece;
    int cursor_file = 4;
    int cursor_rank = 1;
    int selected_file = 0;
    int selected_rank = 0;
    int has_selection = 0;
    int running = 1;
    int changed;

    board_init_starting_position(&board);

    if (vga_init() != 0) {
        fprintf(stderr, "CHESS FART: unable to initialize video.\n");
        return 1;
    }

    input_init();
    board_view_render_build2(&board, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank);
    vga_present();

    while (running) {
        key = input_poll_key();
        changed = 0;

        switch (key) {
        case CF_KEY_ESCAPE:
            running = 0;
            break;
        case CF_KEY_UP:
            if (cursor_rank < 7) {
                ++cursor_rank;
                changed = 1;
            }
            break;
        case CF_KEY_DOWN:
            if (cursor_rank > 0) {
                --cursor_rank;
                changed = 1;
            }
            break;
        case CF_KEY_LEFT:
            if (cursor_file > 0) {
                --cursor_file;
                changed = 1;
            }
            break;
        case CF_KEY_RIGHT:
            if (cursor_file < 7) {
                ++cursor_file;
                changed = 1;
            }
            break;
        case CF_KEY_ENTER:
            piece = board_piece_at(&board, cursor_file, cursor_rank);
            if (piece != 0 && piece->type != CF_PIECE_NONE) {
                if (has_selection && selected_file == cursor_file &&
                    selected_rank == cursor_rank) {
                    has_selection = 0;
                } else {
                    selected_file = cursor_file;
                    selected_rank = cursor_rank;
                    has_selection = 1;
                }
            } else {
                has_selection = 0;
            }
            changed = 1;
            break;
        case CF_KEY_NONE:
        default:
            break;
        }

        if (changed) {
            board_view_render_build2(&board, cursor_file, cursor_rank,
                                     has_selection, selected_file, selected_rank);
            vga_present();
        }
    }

    vga_shutdown();
    return 0;
}
