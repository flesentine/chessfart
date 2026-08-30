#include <stdio.h>
#include <string.h>

#include "board.h"
#include "board_view.h"
#include "input.h"
#include "vga.h"

static void square_name(char *out, int file, int rank)
{
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

int main(void)
{
    CfBoard board;
    CfInputKey key;
    CfMoveList legal_moves;
    const CfPiece *piece;
    CfMove made_move;
    int cursor_file = 4;
    int cursor_rank = 1;
    int selected_file = 0;
    int selected_rank = 0;
    int has_selection = 0;
    int running = 1;
    int changed;
    char message[24];
    char from[3];
    char to[3];

    board_init_starting_position(&board);
    legal_moves.count = 0;
    strcpy(message, "WHITE TO MOVE");

    if (vga_init() != 0) {
        fprintf(stderr, "CHESS FART: unable to initialize video.\n");
        return 1;
    }

    input_init();
    board_view_render_build3(&board, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             &legal_moves, message);
    vga_present();

    while (running) {
        key = input_poll_key();
        changed = 0;
        switch (key) {
        case CF_KEY_ESCAPE:
            running = 0;
            break;
        case CF_KEY_UP:
            if (cursor_rank < 7) { ++cursor_rank; changed = 1; }
            break;
        case CF_KEY_DOWN:
            if (cursor_rank > 0) { --cursor_rank; changed = 1; }
            break;
        case CF_KEY_LEFT:
            if (cursor_file > 0) { --cursor_file; changed = 1; }
            break;
        case CF_KEY_RIGHT:
            if (cursor_file < 7) { ++cursor_file; changed = 1; }
            break;
        case CF_KEY_ENTER:
            piece = board_piece_at(&board, cursor_file, cursor_rank);
            if (!has_selection) {
                if (piece != 0 && piece->type != CF_PIECE_NONE && piece->color == board.side_to_move) {
                    selected_file = cursor_file;
                    selected_rank = cursor_rank;
                    has_selection = 1;
                    board_generate_legal_moves(&board, selected_file, selected_rank, &legal_moves);
                    strcpy(message, "CHOOSE TARGET");
                } else if (piece != 0 && piece->type != CF_PIECE_NONE) {
                    strcpy(message, "WRONG COLOR");
                } else {
                    strcpy(message, "EMPTY SQUARE");
                }
            } else if (cursor_file == selected_file && cursor_rank == selected_rank) {
                has_selection = 0;
                legal_moves.count = 0;
                strcpy(message, "CANCELLED");
            } else if (board_move_is_legal(&board, selected_file, selected_rank, cursor_file, cursor_rank)) {
                square_name(from, selected_file, selected_rank);
                square_name(to, cursor_file, cursor_rank);
                if (board_make_move(&board, selected_file, selected_rank, cursor_file, cursor_rank, &made_move)) {
                    sprintf(message, made_move.captured.type == CF_PIECE_NONE ? "MOVE %s-%s" : "CAPTURE %s-%s", from, to);
                    has_selection = 0;
                    legal_moves.count = 0;
                }
            } else if (piece != 0 && piece->type != CF_PIECE_NONE && piece->color == board.side_to_move) {
                selected_file = cursor_file;
                selected_rank = cursor_rank;
                board_generate_legal_moves(&board, selected_file, selected_rank, &legal_moves);
                strcpy(message, "NEW SOURCE");
            } else {
                strcpy(message, "ILLEGAL MOVE");
            }
            changed = 1;
            break;
        case CF_KEY_NONE:
        default:
            break;
        }

        if (changed) {
            board_view_render_build3(&board, cursor_file, cursor_rank,
                                     has_selection, selected_file, selected_rank,
                                     &legal_moves, message);
            vga_present();
        }
    }

    vga_shutdown();
    return 0;
}
