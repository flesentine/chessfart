#include <conio.h>

#include "input.h"

void input_init(void)
{
    while (kbhit()) {
        (void)getch();
    }
}

int input_escape_pressed(void)
{
    int ch;
    if (!kbhit()) {
        return 0;
    }
    ch = getch();
    if (ch == 0 || ch == 0xE0) {
        if (kbhit()) {
            (void)getch();
        }
        return 0;
    }
    return ch == 27;
}
