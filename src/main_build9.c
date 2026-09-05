#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "audio_game.h"
#include "board.h"
#include "gas.h"
#include "input_build5.h"
#include "persistence.h"
#include "persistence_ui.h"
#include "vga.h"

#ifdef CF_HOST_BUILD
#define BUILD9_SAVE_PATH "build/host/CHESSFRT.SAV"
#define BUILD9_CONFIG_PATH "build/host/CHESSFRT.CFG"
#else
#define BUILD9_SAVE_PATH CF_DEFAULT_SAVE_PATH
#define BUILD9_CONFIG_PATH CF_DEFAULT_CONFIG_PATH
#endif

static CfBoard g_board;
static CfGasState g_gas;
static CfGasHistory g_history;

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
        if (list->moves[i].to_file == file &&
            list->moves[i].to_rank == rank &&
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

static void square_name(char *out, int file, int rank)
{
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

static void set_move_message(char *message, const CfMove *move,
                             CfGameStatus status)
{
    char from[3];
    char to[3];
    if (status_is_terminal(status) || status == CF_GAME_CHECK) {
        strcpy(message, board_game_status_name(status));
    } else if ((move->flags & CF_MOVE_CASTLE_KING) != 0U) {
        strcpy(message, "CASTLE KING");
    } else if ((move->flags & CF_MOVE_CASTLE_QUEEN) != 0U) {
        strcpy(message, "CASTLE QUEEN");
    } else if ((move->flags & CF_MOVE_EN_PASSANT) != 0U) {
        strcpy(message, "EN PASSANT");
    } else if ((move->flags & CF_MOVE_PROMOTION) != 0U) {
        sprintf(message, "PROMOTE %c", board_piece_letter(move->promotion));
    } else {
        square_name(from, move->from_file, move->from_rank);
        square_name(to, move->to_file, move->to_rank);
        sprintf(message, move->captured.type == CF_PIECE_NONE ?
                "MOVE %s-%s" : "CAPTURE %s-%s", from, to);
    }
}

static void set_fart_message(char *message, const CfFartAction *action,
                             CfGameStatus status)
{
    if (status_is_terminal(status) || status == CF_GAME_CHECK) {
        strcpy(message, board_game_status_name(status));
    } else if (action->result == CF_FART_PROMOTION) {
        sprintf(message, "PUSH PROMO %c",
                board_piece_letter(action->promotion));
    } else {
        sprintf(message, "%s %s",
                gas_fart_preview_name(action->result),
                gas_direction_name(action->direction));
    }
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

static void play_key_sound(CfInputKey5 key)
{
    switch (key) {
    case CF5_KEY_UP:
    case CF5_KEY_DOWN:
    case CF5_KEY_LEFT:
    case CF5_KEY_RIGHT:
    case CF5_KEY_UP_LEFT:
    case CF5_KEY_UP_RIGHT:
    case CF5_KEY_DOWN_LEFT:
    case CF5_KEY_DOWN_RIGHT:
        audio_play_event(CF_AUDIO_CURSOR);
        break;
    case CF5_KEY_ENTER:
        audio_play_event(CF_AUDIO_SELECT);
        break;
    case CF5_KEY_FART:
        audio_play_event(CF_AUDIO_FART_READY);
        break;
    case CF5_KEY_SAVE:
    case CF5_KEY_LOAD:
        audio_play_event(CF_AUDIO_MENU_CONFIRM);
        break;
    default:
        break;
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

static void render(int cursor_file, int cursor_rank,
                   int has_selection, int selected_file, int selected_rank,
                   const CfMoveList *legal_moves, CfGameStatus status,
                   int promotion_pending, CfPieceType promotion_choice,
                   int fart_mode, CfFartDirection fart_direction,
                   CfFartPreview fart_preview,
                   int fart_promotion_pending,
                   CfPieceType fart_promotion_choice,
                   const char *message)
{
    persistence_render_game(&g_board, &g_gas,
                            cursor_file, cursor_rank,
                            has_selection, selected_file, selected_rank,
                            legal_moves, status,
                            promotion_pending, promotion_choice,
                            fart_mode, fart_direction, fart_preview,
                            fart_promotion_pending,
                            fart_promotion_choice, message);
    vga_present();
}

int main(void)
{
    CfAudioConfig config;
    CfPersistenceResult persist_result;
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

    audio_default_config(&config);
    persist_result = persistence_load_config(BUILD9_CONFIG_PATH, &config);
    if (persist_result != CF_PERSIST_OK &&
        persist_result != CF_PERSIST_NOT_FOUND)
        audio_default_config(&config);

    (void)audio_init(&config);
    if (!persistence_title_screen(BUILD9_CONFIG_PATH)) {
        (void)persistence_save_config(BUILD9_CONFIG_PATH,
                                      audio_get_config());
        audio_shutdown();
        return 0;
    }

    init_game(&cursor_file, &cursor_rank);
    status = gas_game_status(&g_board, &g_gas, &g_history);
    legal_moves.count = 0;
    strcpy(message, "WHITE TO MOVE");

    if (vga_init() != 0) {
        audio_shutdown();
        return 1;
    }
    input5_init();
    render(cursor_file, cursor_rank, has_selection,
           selected_file, selected_rank, &legal_moves, status,
           promotion_pending, promotion_choice,
           fart_mode, fart_direction, fart_preview,
           fart_promotion_pending, fart_promotion_choice, message);

    while (running) {
        key = input5_poll_key();
        changed = 0;
        play_key_sound(key);

        if (key == CF5_KEY_ESCAPE) {
            if (fart_mode) {
                fart_mode = 0;
                fart_promotion_pending = 0;
                fart_preview = CF_FART_INVALID;
                strcpy(message, "FART CANCELLED");
                changed = 1;
            } else if (promotion_pending) {
                promotion_pending = 0;
                strcpy(message, "PROMO CANCELLED");
                changed = 1;
            } else {
                running = 0;
                continue;
            }
        }

        if (key == CF5_KEY_SAVE) {
            persist_result = persistence_save_game(BUILD9_SAVE_PATH,
                                                   &g_board, &g_gas,
                                                   &g_history);
            sprintf(message, "SAVE %s",
                    persistence_result_name(persist_result));
            if (persist_result != CF_PERSIST_OK)
                audio_play_event(CF_AUDIO_INVALID);
            changed = 1;
        } else if (key == CF5_KEY_LOAD) {
            persist_result = persistence_load_game(BUILD9_SAVE_PATH,
                                                   &g_board, &g_gas,
                                                   &g_history);
            sprintf(message, "LOAD %s",
                    persistence_result_name(persist_result));
            if (persist_result == CF_PERSIST_OK) {
                status = gas_game_status(&g_board, &g_gas, &g_history);
                has_selection = 0;
                legal_moves.count = 0;
                promotion_pending = 0;
                fart_promotion_pending = 0;
                fart_mode = 0;
                fart_preview = CF_FART_INVALID;
            } else {
                audio_play_event(CF_AUDIO_INVALID);
            }
            changed = 1;
        } else if (fart_promotion_pending) {
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
                if (audio_game_make_fart(&g_board, &g_gas,
                                         selected_file, selected_rank,
                                         fart_direction,
                                         fart_promotion_choice,
                                         &fart_action)) {
                    finish_fart(&fart_action, message, &status,
                                &fart_mode, &fart_promotion_pending,
                                &has_selection, &legal_moves,
                                &fart_preview);
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
                if (audio_game_make_move_ex(&g_board, &g_gas,
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
                           audio_game_make_fart(&g_board, &g_gas,
                                                selected_file, selected_rank,
                                                fart_direction,
                                                CF_PIECE_QUEEN,
                                                &fart_action)) {
                    finish_fart(&fart_action, message, &status,
                                &fart_mode, &fart_promotion_pending,
                                &has_selection, &legal_moves,
                                &fart_preview);
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
                        board_generate_legal_moves(&g_board,
                                                   selected_file,
                                                   selected_rank,
                                                   &legal_moves);
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
                                                selected_file,
                                                selected_rank,
                                                cursor_file,
                                                cursor_rank)) {
                    if (target_is_promotion(&legal_moves,
                                            cursor_file, cursor_rank)) {
                        promotion_pending = 1;
                        promotion_choice = CF_PIECE_QUEEN;
                        strcpy(message, "CHOOSE PROMO");
                    } else if (audio_game_make_move(&g_board, &g_gas,
                                                    selected_file,
                                                    selected_rank,
                                                    cursor_file,
                                                    cursor_rank,
                                                    &made_move)) {
                        gas_history_record(&g_history, &g_board, &g_gas);
                        status = gas_game_status(&g_board, &g_gas,
                                                &g_history);
                        set_move_message(message, &made_move.chess_move,
                                         status);
                        has_selection = 0;
                        legal_moves.count = 0;
                    }
                } else if (piece != 0 &&
                           piece->type != CF_PIECE_NONE &&
                           piece->color == g_board.side_to_move) {
                    selected_file = cursor_file;
                    selected_rank = cursor_rank;
                    board_generate_legal_moves(&g_board,
                                               selected_file,
                                               selected_rank,
                                               &legal_moves);
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
                                                selected_file,
                                                selected_rank,
                                                fart_direction);
            render(cursor_file, cursor_rank, has_selection,
                   selected_file, selected_rank, &legal_moves, status,
                   promotion_pending, promotion_choice,
                   fart_mode, fart_direction, fart_preview,
                   fart_promotion_pending, fart_promotion_choice, message);
        }
    }

    (void)persistence_save_config(BUILD9_CONFIG_PATH, audio_get_config());
    vga_shutdown();
    audio_shutdown();
    return 0;
}
