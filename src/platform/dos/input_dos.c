#include <conio.h>

#include "input.h"

void input_init(void)
{
    while (kbhit()) {
        (void)getch();
    }
}

CfInputKey input_poll_key(void)
{
    int ch;
    int extended;

    if (!kbhit()) {
        return CF_KEY_NONE;
    }

    ch = getch();
    if (ch == 0 || ch == 0xE0) {
        extended = getch();
        switch (extended) {
        case 72: return CF_KEY_UP;
        case 80: return CF_KEY_DOWN;
        case 75: return CF_KEY_LEFT;
        case 77: return CF_KEY_RIGHT;
        default: return CF_KEY_NONE;
        }
    }

    if (ch == 27) {
        return CF_KEY_ESCAPE;
    }
    if (ch == 13) {
        return CF_KEY_ENTER;
    }
    return CF_KEY_NONE;
}
