#include <stdio.h>
#include <string.h>

#include "ux.h"
#include "ui_layout.h"

static void square_name(char out[3], int file, int rank)
{
    out[0] = (char)('A' + file);
    out[1] = (char)('1' + rank);
    out[2] = '\0';
}

void ux_history_init(CfUxHistory *history)
{
    if (history == 0) return;
    memset(history, 0, sizeof(*history));
}

void ux_history_add(CfUxHistory *history, const char *line)
{
    int slot;
    if (history == 0 || line == 0) return;
    if (history->count < CF_UX_HISTORY_CAPACITY) {
        slot = (history->start + history->count) % CF_UX_HISTORY_CAPACITY;
        ++history->count;
    } else {
        slot = history->start;
        history->start = (history->start + 1) % CF_UX_HISTORY_CAPACITY;
    }
    strncpy(history->lines[slot], line, CF_UX_HISTORY_LINE - 1U);
    history->lines[slot][CF_UX_HISTORY_LINE - 1U] = '\0';
}

const char *ux_history_get(const CfUxHistory *history, int index)
{
    int slot;
    if (history == 0 || index < 0 || index >= history->count) return 0;
    slot = (history->start + index) % CF_UX_HISTORY_CAPACITY;
    return history->lines[slot];
}

void ux_format_move_line(const char *actor, const CfGasMove *move,
                         char *out, unsigned capacity)
{
    char from[3];
    char to[3];
    const CfMove *m;
    if (out == 0 || capacity == 0U) return;
    out[0] = '\0';
    if (actor == 0 || move == 0) return;
    m = &move->chess_move;
    if ((m->flags & CF_MOVE_CASTLE_KING) != 0U)
        sprintf(out, "%s O-O", actor);
    else if ((m->flags & CF_MOVE_CASTLE_QUEEN) != 0U)
        sprintf(out, "%s O-O-O", actor);
    else {
        square_name(from, m->from_file, m->from_rank);
        square_name(to, m->to_file, m->to_rank);
        if ((m->flags & CF_MOVE_PROMOTION) != 0U)
            sprintf(out, "%s %s-%s=%c", actor, from, to,
                    board_piece_letter(m->promotion));
        else if ((m->flags & CF_MOVE_EN_PASSANT) != 0U)
            sprintf(out, "%s %sx%s EP", actor, from, to);
        else if (m->captured.type != CF_PIECE_NONE)
            sprintf(out, "%s %sx%s", actor, from, to);
        else
            sprintf(out, "%s %s-%s", actor, from, to);
    }
    out[capacity - 1U] = '\0';
}

void ux_format_fart_line(const char *actor, const CfFartAction *fart,
                         char *out, unsigned capacity)
{
    char from[3];
    if (out == 0 || capacity == 0U) return;
    out[0] = '\0';
    if (actor == 0 || fart == 0) return;
    square_name(from, fart->actor_file, fart->actor_rank);
    if (fart->result == CF_FART_PROMOTION)
        sprintf(out, "%s FART %s %s =%c", actor, from,
                gas_direction_name(fart->direction),
                board_piece_letter(fart->promotion));
    else
        sprintf(out, "%s FART %s %s %s", actor, from,
                gas_direction_name(fart->direction),
                gas_fart_preview_name(fart->result));
    out[capacity - 1U] = '\0';
}

void ux_format_cpu_line(const char *actor, const CfCpuAction *action,
                        char *out, unsigned capacity)
{
    (void)actor;
    if (out == 0 || capacity == 0U) return;
    out[0] = '\0';
    if (action == 0) return;
    cpu_format_action(action, out, capacity);
}

const char *ux_terminal_title(CfGameStatus status, CfPieceColor side_to_move)
{
    if (status == CF_GAME_CHECKMATE)
        return side_to_move == CF_COLOR_WHITE ? "BLACK WINS" : "WHITE WINS";
    if (status == CF_GAME_STALEMATE) return "STALEMATE";
    if (status == CF_GAME_DRAW_FIFTY_MOVE) return "DRAW - 50 MOVE";
    if (status == CF_GAME_DRAW_THREEFOLD) return "DRAW - THREEFOLD";
    if (status == CF_GAME_DRAW_INSUFFICIENT) return "DRAW - MATERIAL";
    if (status == CF_GAME_CHECK) return "CHECK";
    return "GAME ON";
}

int ux_board_hit_test(int x, int y, int *file, int *rank)
{
    int screen_file;
    int screen_rank;
    if (x < CF_UI_BOARD_X || y < CF_UI_BOARD_Y ||
        x >= CF_UI_BOARD_X + CF_UI_BOARD_PIXELS ||
        y >= CF_UI_BOARD_Y + CF_UI_BOARD_PIXELS) return 0;
    screen_file = (x - CF_UI_BOARD_X) / CF_UI_SQUARE_SIZE;
    screen_rank = (y - CF_UI_BOARD_Y) / CF_UI_SQUARE_SIZE;
    if (file != 0) *file = screen_file;
    if (rank != 0) *rank = 7 - screen_rank;
    return 1;
}

int ux_title_menu_hit_test(int x, int y, int *menu)
{
    int item;
    if (x < CF_UI_TITLE_MENU_X ||
        x >= CF_UI_TITLE_MENU_X + CF_UI_TITLE_MENU_W ||
        y < CF_UI_TITLE_MENU_HIT_Y ||
        y >= CF_UI_TITLE_MENU_HIT_Y + CF_UI_TITLE_MENU_HIT_H) return 0;
    item = (y - CF_UI_TITLE_MENU_HIT_Y) / CF_UI_TITLE_MENU_ITEM_STEP;
    if (item < 0 || item >= CF_UI_TITLE_MENU_ITEM_COUNT) return 0;
    if (menu != 0) *menu = item;
    return 1;
}

int ux_build_attract_push(CfBoard *board, CfGasState *gas,
                          CfFartAction *action)
{
    if (board == 0 || gas == 0) return 0;
    board_clear(board);
    gas_init(gas);
    board_set_piece(board, 0, 0, CF_PIECE_KING, CF_COLOR_WHITE);
    board_set_piece(board, 7, 7, CF_PIECE_KING, CF_COLOR_BLACK);
    board_set_piece(board, 2, 2, CF_PIECE_KNIGHT, CF_COLOR_WHITE);
    board_set_piece(board, 3, 3, CF_PIECE_PAWN, CF_COLOR_BLACK);
    gas_set(gas, 2, 2, 3U);
    gas_set(gas, 3, 3, 1U);
    return gas_make_fart(board, gas, 2, 2, CF_FART_NE,
                         CF_PIECE_QUEEN, action);
}
