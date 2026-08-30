#include "board_view_build7.h"
#include "presentation.h"

#define board_view_render_build6 presentation_render_game
#define gas_make_fart presentation_make_fart
#define main chessfart_build6_game_main
#include "main_build6.c"
#undef main
#undef gas_make_fart
#undef board_view_render_build6

int main(void)
{
    if (!presentation_title_screen()) return 0;
    return chessfart_build6_game_main();
}
