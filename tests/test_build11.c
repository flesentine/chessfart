#include <stdio.h>
#include <string.h>
#include "ux.h"
#include "ui_layout.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static void test_history_ring(void)
{
    CfUxHistory history;
    char line[20];
    int i;
    ux_history_init(&history);
    for (i = 0; i < 40; ++i) {
        sprintf(line, "ACTION %02d", i);
        ux_history_add(&history, line);
    }
    CHECK(history.count == CF_UX_HISTORY_CAPACITY);
    CHECK(strcmp(ux_history_get(&history, 0), "ACTION 08") == 0);
    CHECK(strcmp(ux_history_get(&history, 31), "ACTION 39") == 0);
    CHECK(ux_history_get(&history, 32) == 0);
}

static void test_hit_testing(void)
{
    int file = -1;
    int rank = -1;
    int right = CF_UI_BOARD_X + CF_UI_BOARD_PIXELS - 1;
    int bottom = CF_UI_BOARD_Y + CF_UI_BOARD_PIXELS - 1;

    CHECK(ux_board_hit_test(CF_UI_BOARD_X, CF_UI_BOARD_Y, &file, &rank));
    CHECK(file == 0 && rank == 7);
    CHECK(ux_board_hit_test(right, bottom, &file, &rank));
    CHECK(file == 7 && rank == 0);

    CHECK(!ux_board_hit_test(CF_UI_BOARD_X - 1, CF_UI_BOARD_Y, &file, &rank));
    CHECK(!ux_board_hit_test(CF_UI_BOARD_X, CF_UI_BOARD_Y - 1, &file, &rank));
    CHECK(!ux_board_hit_test(right + 1, bottom, &file, &rank));
    CHECK(!ux_board_hit_test(right, bottom + 1, &file, &rank));

    CHECK(ux_board_hit_test(CF_UI_BOARD_X + 6 * CF_UI_SQUARE_SIZE + 9,
                            CF_UI_BOARD_Y + 7 * CF_UI_SQUARE_SIZE + 9,
                            &file, &rank));
    CHECK(file == 6 && rank == 0);
}

static void test_terminal_labels(void)
{
    CHECK(strcmp(ux_terminal_title(CF_GAME_CHECKMATE, CF_COLOR_WHITE),
                 "BLACK WINS") == 0);
    CHECK(strcmp(ux_terminal_title(CF_GAME_CHECKMATE, CF_COLOR_BLACK),
                 "WHITE WINS") == 0);
    CHECK(strcmp(ux_terminal_title(CF_GAME_STALEMATE, CF_COLOR_WHITE),
                 "STALEMATE") == 0);
}

static void test_attract_push(void)
{
    CfBoard board;
    CfGasState gas;
    CfFartAction action;
    CHECK(ux_build_attract_push(&board, &gas, &action));
    CHECK(action.result == CF_FART_PUSH);
    CHECK(board.squares[3][3].type == CF_PIECE_NONE);
    CHECK(board.squares[4][4].type == CF_PIECE_PAWN);
    CHECK(board.squares[4][4].color == CF_COLOR_BLACK);
    CHECK(gas_at(&gas, 2, 2) == 1U);
    CHECK(gas_at(&gas, 4, 4) == 1U);
    CHECK(board.side_to_move == CF_COLOR_BLACK);
}

int main(void)
{
    test_history_ring();
    test_hit_testing();
    test_terminal_labels();
    test_attract_push();
    if (failures != 0) {
        printf("Build 11 UX tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 11 UX tests passed.\n");
    return 0;
}
