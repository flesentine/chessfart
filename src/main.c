#include <stdio.h>

#include "board_view.h"
#include "input.h"
#include "vga.h"

int main(void)
{
    if (vga_init() != 0) {
        fprintf(stderr, "CHESS FART: unable to initialize video.\n");
        return 1;
    }

    input_init();
    board_view_render_build1();
    vga_present();

    while (!input_escape_pressed()) {
        /* Build 1 intentionally has no game loop work yet. */
    }

    vga_shutdown();
    return 0;
}
