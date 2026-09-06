#include <stdio.h>
#include <string.h>
#include "ux.h"
#include "ui_layout.h"
#include "ui_theme.h"

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

static void test_layout_contract(void)
{
    int turn_width = 13 * 6 - 1;
    int state_width = 11 * 6 - 1;
    int short_prompt_width = 6 * 6 - 1;
    int replay_prompt_width = 6 * 6 - 1;
    int esc_width = 3 * 6 - 1;
    int fart_badge_text_width = 9 * 6 - 1;
    int fart_arrows_width = 23 * 6 - 1;
    int fart_enter_width = 13 * 6 - 1;
    int fart_esc_width = 10 * 6 - 1;

    CHECK(CF_UI_SCREEN_W == 320);
    CHECK(CF_UI_SCREEN_H == 200);
    CHECK(CF_UI_SQUARE_SIZE == 18);
    CHECK(CF_UI_BOARD_PIXELS == 144);
    CHECK(CF_UI_HEADER_Y + CF_UI_HEADER_H <= CF_UI_BOARD_FRAME_Y);
    CHECK(CF_UI_HEADER_RULE_Y < CF_UI_BOARD_FRAME_Y);
    CHECK(CF_UI_BOARD_X + CF_UI_BOARD_PIXELS <= CF_UI_PANEL_X);
    CHECK(CF_UI_PANEL_X + CF_UI_PANEL_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_COMMAND_Y + CF_UI_COMMAND_H == CF_UI_SCREEN_H);
    CHECK(CF_UI_PANEL_CONTENT_X == 180);
    CHECK(CF_UI_PANEL_CONTENT_W == 123);
    CHECK(CF_UI_HUD_PIECE_BOX_X + CF_UI_HUD_PIECE_BOX_W <=
          CF_UI_PANEL_X + CF_UI_PANEL_W);
    CHECK(CF_UI_HUD_VALUE_X + state_width <=
          CF_UI_PANEL_CONTENT_X + CF_UI_PANEL_CONTENT_W);
    CHECK(CF_UI_HUD_ACTION_LINE2_Y + 7 <=
          CF_UI_PANEL_Y + CF_UI_PANEL_H - 3);
    CHECK(CF_UI_PROMPT_TURN_X + turn_width < CF_UI_PROMPT_FART_X);
    CHECK(CF_UI_PROMPT_FART_X + short_prompt_width < CF_UI_PROMPT_SAVE_X);
    CHECK(CF_UI_PROMPT_SAVE_X + short_prompt_width < CF_UI_PROMPT_LOAD_X);
    CHECK(CF_UI_PROMPT_LOAD_X + short_prompt_width < CF_UI_PROMPT_HELP_X);
    CHECK(CF_UI_PROMPT_HELP_X + short_prompt_width < CF_UI_PROMPT_REPLAY_X);
    CHECK(CF_UI_PROMPT_REPLAY_X + replay_prompt_width < CF_UI_PROMPT_ESC_X);
    CHECK(CF_UI_PROMPT_ESC_X + esc_width < CF_UI_SCREEN_W);
    CHECK(CF_UI_PROMPT_TEXT_Y + 7 <= CF_UI_SCREEN_H);
    CHECK(CF_UI_FART_BADGE_X >= 0);
    CHECK(CF_UI_FART_BADGE_X + CF_UI_FART_BADGE_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_FART_BADGE_TEXT_X + fart_badge_text_width <=
          CF_UI_FART_BADGE_X + CF_UI_FART_BADGE_W);
    CHECK(CF_UI_FART_PIECE_BOX_X + CF_UI_FART_PIECE_BOX_W <=
          CF_UI_PANEL_X + CF_UI_PANEL_W);
    CHECK(CF_UI_FART_ACTION_LINE2_Y + 7 <=
          CF_UI_PANEL_Y + CF_UI_PANEL_H);
    CHECK(CF_UI_FART_PROMPT_ARROWS_X + fart_arrows_width <
          CF_UI_FART_PROMPT_ENTER_X);
    CHECK(CF_UI_FART_PROMPT_ENTER_X + fart_enter_width <
          CF_UI_FART_PROMPT_ESC_X);
    CHECK(CF_UI_FART_PROMPT_ESC_X + fart_esc_width < CF_UI_SCREEN_W);
    CHECK(CF_UI_TITLE_MENU_X >= 0);
    CHECK(CF_UI_TITLE_MENU_Y >= 0);
    CHECK(CF_UI_TITLE_MENU_X + CF_UI_TITLE_MENU_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_TITLE_ITEM_PLAY_CPU == 0);
    CHECK(CF_UI_TITLE_ITEM_PLAY_LOCAL == 1);
    CHECK(CF_UI_TITLE_ITEM_PRACTICE == 2);
    CHECK(CF_UI_TITLE_ITEM_ATTRACT == 3);
    CHECK(CF_UI_TITLE_ITEM_HELP == 4);
    CHECK(CF_UI_TITLE_ITEM_CREDITS == 5);
    CHECK(CF_UI_TITLE_ITEM_QUIT == 6);
    CHECK(CF_UI_TITLE_ITEM_QUIT + 1 == CF_UI_TITLE_MENU_ITEM_COUNT);
    CHECK(CF_UI_TITLE_MENU_ITEM_COUNT == 7);
    CHECK(CF_UI_TITLE_MENU_ITEM_H == CF_UI_TITLE_MENU_ITEM_STEP);
    CHECK(CF_UI_TITLE_MENU_HIT_Y == CF_UI_TITLE_MENU_ITEM_Y - 1);
    CHECK(CF_UI_TITLE_MENU_HIT_Y >= CF_UI_TITLE_MENU_Y);
    CHECK(CF_UI_TITLE_MENU_HIT_Y + CF_UI_TITLE_MENU_HIT_H <=
          CF_UI_TITLE_MENU_Y + CF_UI_TITLE_MENU_H);
    CHECK(CF_UI_TITLE_MENU_Y + CF_UI_TITLE_MENU_H < CF_UI_TITLE_FOOTER_Y);
    CHECK(CF_UI_TITLE_FOOTER2_Y + 7 < CF_UI_SCREEN_H);
    CHECK(CF_UI_MODAL_HELP_X + CF_UI_MODAL_HELP_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_MODAL_HELP_Y + CF_UI_MODAL_HELP_H <= CF_UI_SCREEN_H);
    CHECK(CF_UI_MODAL_HISTORY_X + CF_UI_MODAL_HISTORY_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_MODAL_HISTORY_Y + CF_UI_MODAL_HISTORY_H <= CF_UI_SCREEN_H);
    CHECK(CF_UI_MODAL_CREDITS_X + CF_UI_MODAL_CREDITS_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_MODAL_CREDITS_Y + CF_UI_MODAL_CREDITS_H <= CF_UI_SCREEN_H);
    CHECK(CF_UI_MODAL_TERMINAL_X + CF_UI_MODAL_TERMINAL_W <= CF_UI_SCREEN_W);
    CHECK(CF_UI_MODAL_TERMINAL_Y + CF_UI_MODAL_TERMINAL_H <= CF_UI_SCREEN_H);
    CHECK(CF_UI_NOTICE_X >= CF_UI_PANEL_X);
    CHECK(CF_UI_NOTICE_X + CF_UI_NOTICE_W <=
          CF_UI_PANEL_X + CF_UI_PANEL_W);
    CHECK(CF_UI_NOTICE_Y + CF_UI_NOTICE_H <=
          CF_UI_PANEL_Y + CF_UI_PANEL_H);
    CHECK(CF_UI_ACTIVE_COLOR_COUNT == 30);
    CHECK(CF_UI_GRAYSCALE_FIRST == 30);
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

static void test_hit_testing_every_square(void)
{
    int file;
    int rank;
    int hit_file;
    int hit_rank;
    int x;
    int y;

    for (rank = 0; rank < 8; ++rank) {
        for (file = 0; file < 8; ++file) {
            x = CF_UI_BOARD_X + file * CF_UI_SQUARE_SIZE;
            y = CF_UI_BOARD_Y + (7 - rank) * CF_UI_SQUARE_SIZE;

            hit_file = -1;
            hit_rank = -1;
            CHECK(ux_board_hit_test(x, y, &hit_file, &hit_rank));
            CHECK(hit_file == file && hit_rank == rank);

            hit_file = -1;
            hit_rank = -1;
            CHECK(ux_board_hit_test(x + CF_UI_SQUARE_SIZE - 1,
                                    y + CF_UI_SQUARE_SIZE - 1,
                                    &hit_file, &hit_rank));
            CHECK(hit_file == file && hit_rank == rank);
        }
    }
}

static void test_title_menu_hit_testing(void)
{
    int item;
    int i;
    int top;
    int bottom;
    int right = CF_UI_TITLE_MENU_X + CF_UI_TITLE_MENU_W - 1;

    for (i = 0; i < CF_UI_TITLE_MENU_ITEM_COUNT; ++i) {
        top = CF_UI_TITLE_MENU_HIT_Y + i * CF_UI_TITLE_MENU_ITEM_STEP;
        bottom = top + CF_UI_TITLE_MENU_ITEM_STEP - 1;

        item = -1;
        CHECK(ux_title_menu_hit_test(CF_UI_TITLE_MENU_X, top, &item));
        CHECK(item == i);

        item = -1;
        CHECK(ux_title_menu_hit_test(right, bottom, &item));
        CHECK(item == i);
    }

    item = -1;
    CHECK(!ux_title_menu_hit_test(CF_UI_TITLE_MENU_X - 1,
                                  CF_UI_TITLE_MENU_HIT_Y, &item));
    CHECK(!ux_title_menu_hit_test(CF_UI_TITLE_MENU_X + CF_UI_TITLE_MENU_W,
                                  CF_UI_TITLE_MENU_HIT_Y, &item));
    CHECK(!ux_title_menu_hit_test(CF_UI_TITLE_MENU_X,
                                  CF_UI_TITLE_MENU_HIT_Y - 1, &item));
    CHECK(!ux_title_menu_hit_test(CF_UI_TITLE_MENU_X,
                                  CF_UI_TITLE_MENU_HIT_Y +
                                  CF_UI_TITLE_MENU_HIT_H, &item));
    CHECK(ux_title_menu_hit_test(CF_UI_TITLE_MENU_X,
                                 CF_UI_TITLE_MENU_HIT_Y, 0));
}

static void test_match_modes(void)
{
    CHECK(ux_match_mode_valid(CF_MATCH_CPU));
    CHECK(ux_match_mode_valid(CF_MATCH_LOCAL));
    CHECK(!ux_match_mode_valid((CfMatchMode)-1));
    CHECK(!ux_match_mode_valid((CfMatchMode)2));
    CHECK(ux_match_mode_uses_cpu(CF_MATCH_CPU));
    CHECK(!ux_match_mode_uses_cpu(CF_MATCH_LOCAL));
    CHECK(strcmp(ux_match_mode_name(CF_MATCH_CPU), "CPU") == 0);
    CHECK(strcmp(ux_match_mode_name(CF_MATCH_LOCAL), "LOCAL 2P") == 0);
    CHECK(strcmp(ux_match_mode_name((CfMatchMode)99), "UNKNOWN") == 0);
    CHECK(strcmp(ux_match_actor_name(CF_MATCH_CPU, CF_COLOR_WHITE),
                 "YOU") == 0);
    CHECK(strcmp(ux_match_actor_name(CF_MATCH_CPU, CF_COLOR_BLACK),
                 "CPU") == 0);
    CHECK(strcmp(ux_match_actor_name(CF_MATCH_CPU, CF_COLOR_NONE),
                 "PLAYER") == 0);
    CHECK(strcmp(ux_match_actor_name(CF_MATCH_LOCAL, CF_COLOR_WHITE),
                 "WHITE") == 0);
    CHECK(strcmp(ux_match_actor_name(CF_MATCH_LOCAL, CF_COLOR_BLACK),
                 "BLACK") == 0);
    CHECK(strcmp(ux_match_actor_name(CF_MATCH_LOCAL, CF_COLOR_NONE),
                 "PLAYER") == 0);
    CHECK(strcmp(ux_match_panel_label(CF_MATCH_CPU), "CPU") == 0);
    CHECK(strcmp(ux_match_panel_label(CF_MATCH_LOCAL), "MODE") == 0);
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
    test_layout_contract();
    test_hit_testing();
    test_hit_testing_every_square();
    test_title_menu_hit_testing();
    test_match_modes();
    test_terminal_labels();
    test_attract_push();
    if (failures != 0) {
        printf("Build 11 UX tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 11 UX tests passed.\n");
    return 0;
}
