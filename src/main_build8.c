#include "audio.h"
#include "audio_game.h"

#define board_view_render_build6 audio_game_render
#define gas_make_fart audio_game_make_fart
#define gas_make_move audio_game_make_move
#define gas_make_move_ex audio_game_make_move_ex
#define input5_poll_key audio_game_poll_key
#define main chessfart_build6_game_main
#include "main_build6.c"
#undef main
#undef input5_poll_key
#undef gas_make_move_ex
#undef gas_make_move
#undef gas_make_fart
#undef board_view_render_build6

int main(void)
{
    CfAudioConfig config;
    int result;

    audio_default_config(&config);
    (void)audio_init(&config);
    if (!audio_title_screen()) {
        audio_shutdown();
        return 0;
    }
    result = chessfart_build6_game_main();
    audio_shutdown();
    return result;
}
