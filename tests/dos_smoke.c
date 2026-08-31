#include <stdio.h>

#include "audio.h"
#include "mouse.h"
#include "version.h"
#include "vga.h"

int main(void)
{
    CfAudioConfig audio;
    FILE *fp;

    if (vga_init() != 0) return 2;
    vga_clear(1);
    vga_fill_rect(12, 12, 80, 24, 15);
    vga_fill_rect(16, 16, 72, 16, 4);
    vga_present();

    (void)mouse_init();
    mouse_shutdown();

    audio_default_config(&audio);
    audio.device = CF_AUDIO_DEVICE_AUTO;
    audio.sfx_level = CF_AUDIO_LEVEL_LOW;
    if (audio_init(&audio) == 0)
        audio_play_event(CF_AUDIO_MENU_CONFIRM);
    audio_shutdown();
    vga_shutdown();

    fp = fopen("DOSSMOKE.OK", "wt");
    if (fp == 0) return 3;
    fprintf(fp, "CHESS FART %s DOS SMOKE OK\n", CF_VERSION_STRING);
    fclose(fp);
    return 0;
}
