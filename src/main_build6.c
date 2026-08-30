#include <stdio.h>
#include <string.h>

#include "board.h"
#include "board_view_build6.h"
#include "gas.h"
#include "input_build5.h"
#include "vga.h"

static CfBoard g_board;
static CfGasState g_gas;
static CfGasHistory g_history;

static void square_name(char *out, int file, int rank)
{
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

static int status_is_terminal(CfGameStatus status)
{
    return status == CF_GAME_CHECKMATE || status == CF_GAME_STALEMATE ||
           status == CF_GAME_DRAW_FIFTY_MOVE ||
           status == CF_GAME_DRAW_THREEFOLD ||
           status == CF_GAME_DRAW_INSUFFICIENT;
}

static int target_is_promotion(const CfMoveList *list, int file, int rank)
{
    int i;
    for (i = 0; i < list->count; ++i)
        if (list->moves[i].to_file == file && list->moves[i].to_rank == rank &&
            (list->moves[i].flags & CF_MOVE_PROMOTION) != 0U) return 1;
    return 0;
}

static CfPieceType cycle_promotion(CfPieceType current, int direction)
{
    static const CfPieceType choices[4] = {
        CF_PIECE_QUEEN, CF_PIECE_ROOK, CF_PIECE_BISHOP, CF_PIECE_KNIGHT
    };
    int i;
    for (i = 0; i < 4; ++i) {
        if (choices[i] == current) {
            i = (i + direction + 4) % 4;
            return choices[i];
        }
    }
    return CF_PIECE_QUEEN;
}

static void set_move_message(char *message, const CfMove *move,
                             CfGameStatus status)
{
    char from[3];
    char to[3];
    if (status_is_terminal(status) || status == CF_GAME_CHECK) {
        strcpy(message, board_game_status_name(status));
        return;
    }
    if ((move->flags & CF_MOVE_CASTLE_KING) != 0U) {
        strcpy(message, "CASTLE KING");
        return;
    }
    if ((move->flags & CF_MOVE_CASTLE_QUEEN) != 0U) {
        strcpy(message, "CASTLE QUEEN");
        return;
    }
    if ((move->flags & CF_MOVE_EN_PASSANT) != 0U) {
        strcpy(message, "EN PASSANT");
        return;
    }
    if ((move->flags & CF_MOVE_PROMOTION) != 0U) {
        sprintf(message, "PROMOTE %c", board_piece_letter(move->promotion));
        return;
    }
    square_name(from, move->from_file, move->from_rank);
    square_name(to, move->to_file, move->to_rank);
    sprintf(message, move->captured.type == CF_PIECE_NONE ?
            "MOVE %s-%s" : "CAPTURE %s-%s", from, to);
}

static CfFartDirection key_to_direction(CfInputKey5 key,
                                        CfFartDirection current)
{
    switch (key) {
    case CF5_KEY_UP: return CF_FART_N;
    case CF5_KEY_UP_RIGHT: return CF_FART_NE;
    case CF5_KEY_RIGHT: return CF_FART_E;
    case CF5_KEY_DOWN_RIGHT: return CF_FART_SE;
    case CF5_KEY_DOWN: return CF_FART_S;
    case CF5_KEY_DOWN_LEFT: return CF_FART_SW;
    case CF5_KEY_LEFT: return CF_FART_W;
    case CF5_KEY_UP_LEFT: return CF_FART_NW;
    default: return current;
    }
}

static void set_fart_message(char *message, const CfFartAction *action,
                             CfGameStatus status)
{
    if (status_is_terminal(status) || status == CF_GAME_CHECK) {
        strcpy(message, board_game_status_name(status));
        return;
    }
    if (action->result == CF_FART_PROMOTION) {
        sprintf(message, "PUSH PROMO %c", board_piece_letter(action->promotion));
    } else {
        sprintf(message, "%s %s", gas_fart_preview_name(action->result),
                gas_direction_name(action->direction));
    }
}

static void finish_fart(CfFartAction *action, char *message,
                        CfGameStatus *status, int *fart_mode,
                        int *fart_promotion_pending, int *has_selection,
                        CfMoveList *legal_moves, CfFartPreview *preview)
{
    gas_history_record(&g_history, &g_board, &g_gas);
    *status = gas_game_status(&g_board, &g_gas, &g_history);
    set_fart_message(message, action, *status);
    *fart_mode = 0;
    *fart_promotion_pending = 0;
    *has_selection = 0;
    legal_moves->count = 0;
    *preview = CF_FART_INVALID;
}

static void init_game(int *cursor_file, int *cursor_rank)
{
#ifdef CF_BUILD6_DEMO
    board_clear(&g_board);
    gas_init(&g_gas);
    board_set_piece(&g_board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(&g_board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(&g_board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(&g_board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(&g_gas, 2, 2, 3U);
    gas_set(&g_gas, 3, 3, 1U);
    *cursor_file = 2;
    *cursor_rank = 2;
#else
    board_init_starting_position(&g_board);
    gas_init(&g_gas);
    *cursor_file = 4;
    *cursor_rank = 1;
#endif
    gas_history_init(&g_history, &g_board, &g_gas);
}

int main(void)
{
    CfInputKey5 key;
    CfMoveList legal_moves;
    const CfPiece *piece;
    CfGasMove made_move;
    CfFartAction fart_action;
    CfGameStatus status;
    CfPieceType promotion_choice = CF_PIECE_QUEEN;
    CfPieceType fart_promotion_choice = CF_PIECE_QUEEN;
    CfFartDirection fart_direction = CF_FART_N;
    CfFartPreview fart_preview = CF_FART_INVALID;
    int promotion_pending = 0;
    int fart_promotion_pending = 0;
    int fart_mode = 0;
    int cursor_file;
    int cursor_rank;
    int selected_file = 0;
    int selected_rank = 0;
    int has_selection = 0;
    int running = 1;
    int changed;
    char message[24];

    init_game(&cursor_file, &cursor_rank);
    status = gas_game_status(&g_board, &g_gas, &g_history);
    legal_moves.count = 0;
    strcpy(message, "WHITE TO MOVE");

    if (vga_init() != 0) {
        fprintf(stderr, "CHESS FART: unable to initialize video.\n");
        return 1;
    }

    input5_init();
    board_view_render_build6(&g_board, &g_gas, cursor_file, cursor_rank,
                             has_selection, selected_file, selected_rank,
                             &legal_moves, status, promotion_pending,
                             promotion_choice, fart_mode, fart_direction,
                             fart_preview, fart_promotion_pending,
                             fart_promotion_choice, message);
    vga_present();

    while (running) {
        key = input5_poll_key();
        changed = 0;

        if (key == CF5_KEY_ESCAPE) {
            running = 0;
            continue;
        }

        if (fart_promotion_pending) {
            switch (key) {
            case CF5_KEY_LEFT:
            case CF5_KEY_DOWN:
                fart_promotion_choice =
                    cycle_promotion(fart_promotion_choice, -1);
                changed = 1;
                break;
            case CF5_KEY_RIGHT:
            case CF5_KEY_UP:
                fart_promotion_choice =
                    cycle_promotion(fart_promotion_choice, 1);
                changed = 1;
                break;
            case CF5_KEY_FART:
                fart_promotion_pending = 0;
                strcpy(message, "AIM FART");
                changed = 1;
                break;
            case CF5_KEY_ENTER:
                if (gas_make_fart(&g_board, &g_gas,
                                  selected_file, selected_rank,
                                  fart_direction, fart_promotion_choice,
                                  &fart_action)) {
                    finish_fart(&fart_action, message, &status, &fart_mode,
                                &fart_promotion_pending, &has_selection,
                                &legal_moves, &fart_preview);
                } else {
                    strcpy(message, "PROMO ILLEGAL");
                }
                changed = 1;
                break;
            default:
                break;
            }
        } else if (promotion_pending) {
            switch (key) {
            case CF5_KEY_LEFT:
            case CF5_KEY_DOWN:
                promotion_choice = cycle_promotion(promotion_choice, -1);
                changed = 1;
                break;
            case CF5_KEY_RIGHT:
            case CF5_KEY_UP:
                promotion_choice = cycle_promotion(promotion_choice, 1);
                changed = 1;
                break;
            case CF5_KEY_ENTER:
                if (gas_make_move_ex(&g_board, &g_gas,
                                     selected_file, selected_rank,
                                     cursor_file, cursor_rank,
                                     promotion_choice, &made_move)) {
                    gas_history_record(&g_history, &g_board, &g_gas);
                    status = gas_game_status(&g_board, &g_gas, &g_history);
                    set_move_message(message, &made_move.chess_move, status);
                    has_selection = 0;
                    legal_moves.count = 0;
                    promotion_pending = 0;
                }
                changed = 1;
                break;
            default:
                break;
            }
        } else if (fart_mode) {
            switch (key) {
            case CF5_KEY_FART:
                fart_mode = 0;
                fart_preview = CF_FART_INVALID;
                strcpy(message, "FART CANCELLED");
                changed = 1;
                break;
            case CF5_KEY_UP:
            case CF5_KEY_DOWN:
            case CF5_KEY_LEFT:
            case CF5_KEY_RIGHT:
            case CF5_KEY_UP_LEFT:
            case CF5_KEY_UP_RIGHT:
            case CF5_KEY_DOWN_LEFT:
            case CF5_KEY_DOWN_RIGHT:
                fart_direction = key_to_direction(key, fart_direction);
                fart_preview = gas_preview_fart(&g_board, &g_gas,
                                                selected_file, selected_rank,
                                                fart_direction);
                changed = 1;
                break;
            case CF5_KEY_ENTER:
                fart_preview = gas_preview_fart(&g_board, &g_gas,
                                                selected_file, selected_rank,
                                                fart_direction);
                if (fart_preview == CF_FART_PROMOTION) {
                    fart_promotion_pending = 1;
                    fart_promotion_choice = CF_PIECE_QUEEN;
                    strcpy(message, "CHOOSE PUSH PROMO");
                } else if (fart_preview != CF_FART_INVALID &&
                           gas_make_fart(&g_board, &g_gas,
                                         selected_file, selected_rank,
                                         fart_direction, CF_PIECE_QUEEN,
                                         &fart_action)) {
                    finish_fart(&fart_action, message, &status, &fart_mode,
                                &fart_promotion_pending, &has_selection,
                                &legal_moves, &fart_preview);
                } else {
                    strcpy(message, "FART ILLEGAL");
                }
                changed = 1;
                break;
            default:
                break;
            }
        } else {
            switch (key) {
            case CF5_KEY_UP:
                if (cursor_rank < 7) { ++cursor_rank; changed = 1; }
                break;
            case CF5_KEY_DOWN:
                if (cursor_rank > 0) { --cursor_rank; changed = 1; }
                break;
            case CF5_KEY_LEFT:
                if (cursor_file > 0) { --cursor_file; changed = 1; }
                break;
            case CF5_KEY_RIGHT:
                if (cursor_file < 7) { ++cursor_file; changed = 1; }
                break;
            case CF5_KEY_FART:
                if (status_is_terminal(status)) {
                    strcpy(message, board_game_status_name(status));
                } else if (!has_selection) {
                    strcpy(message, "SELECT FIRST");
                } else if (gas_piece_can_fart(&g_board, &g_gas,
                                              selected_file, selected_rank)) {
                    fart_mode = 1;
                    fart_direction = CF_FART_N;
                    fart_preview = gas_preview_fart(&g_board, &g_gas,
                                                    selected_file,
                                                    selected_rank,
                                                    fart_direction);
                    strcpy(message, "AIM FART");
                } else {
                    strcpy(message, "NEED 2 GAS");
                }
                changed = 1;
                break;
            case CF5_KEY_ENTER:
                if (status_is_terminal(status)) {
                    strcpy(message, board_game_status_name(status));
                    changed = 1;
                    break;
                }
                piece = board_piece_at(&g_board, cursor_file, cursor_rank);
                if (!has_selection) {
                    if (piece != 0 && piece->type != CF_PIECE_NONE &&
                        piece->color == g_board.side_to_move) {
                        selected_file = cursor_file;
                        selected_rank = cursor_rank;
                        has_selection = 1;
                        board_generate_legal_moves(&g_board, selected_file,
                                                   selected_rank, &legal_moves);
                        strcpy(message, "CHOOSE TARGET");
                    } else if (piece != 0 &&
                               piece->type != CF_PIECE_NONE) {
                        strcpy(message, "WRONG COLOR");
                    } else {
                        strcpy(message, "EMPTY SQUARE");
                    }
                } else if (cursor_file == selected_file &&
                           cursor_rank == selected_rank) {
                    has_selection = 0;
                    legal_moves.count = 0;
                    strcpy(message, "CANCELLED");
                } else if (board_move_is_legal(&g_board,
                                                selected_file, selected_rank,
                                                cursor_file, cursor_rank)) {
                    if (target_is_promotion(&legal_moves,
                                            cursor_file, cursor_rank)) {
                        promotion_pending = 1;
                        promotion_choice = CF_PIECE_QUEEN;
                        strcpy(message, "CHOOSE PROMO");
                    } else if (gas_make_move(&g_board, &g_gas,
                                             selected_file, selected_rank,
                                             cursor_file, cursor_rank,
                                             &made_move)) {
                        gas_history_record(&g_history, &g_board, &g_gas);
                        status = gas_game_status(&g_board, &g_gas, &g_history);
                        set_move_message(message, &made_move.chess_move,
                                         status);
                        has_selection = 0;
                        legal_moves.count = 0;
                    }
                } else if (piece != 0 && piece->type != CF_PIECE_NONE &&
                           piece->color == g_board.side_to_move) {
                    selected_file = cursor_file;
                    selected_rank = cursor_rank;
                    board_generate_legal_moves(&g_board, selected_file,
                                               selected_rank, &legal_moves);
                    strcpy(message, "NEW SOURCE");
                } else {
                    strcpy(message, "ILLEGAL MOVE");
                }
                changed = 1;
                break;
            default:
                break;
            }
        }

        if (changed) {
            if (fart_mode && !fart_promotion_pending)
                fart_preview = gas_preview_fart(&g_board, &g_gas,
                                                selected_file, selected_rank,
                                                fart_direction);
            board_view_render_build6(&g_board, &g_gas,
                                     cursor_file, cursor_rank,
                                     has_selection,
                                     selected_file, selected_rank,
                                     &legal_moves, status,
                                     promotion_pending, promotion_choice,
                                     fart_mode, fart_direction, fart_preview,
                                     fart_promotion_pending,
                                     fart_promotion_choice, message);
            vga_present();
        }
    }

    vga_shutdown();
    return 0;
}
