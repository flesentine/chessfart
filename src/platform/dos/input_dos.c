#include <conio.h>
#include "input.h"
void input_init(void) { while (kbhit()) (void)getch(); }
CfInputKey input_poll_key(void)
{
    int ch;
    int extended;
    if (!kbhit()) return CF_KEY_NONE;
    ch = getch();
    if (ch == 0 || ch == 0xE0) {
        extended = getch();
        switch (extended) {
        case 72: return CF_KEY_UP;
        case 80: return CF_KEY_DOWN;
        case 75: return CF_KEY_LEFT;
        case 77: return CF_KEY_RIGHT;
        case 71: return CF_KEY_UP_LEFT;
        case 73: return CF_KEY_UP_RIGHT;
        case 79: return CF_KEY_DOWN_LEFT;
        case 81: return CF_KEY_DOWN_RIGHT;
        default: return CF_KEY_NONE;
        }
    }
    if (ch == 27) return CF_KEY_ESCAPE;
    if (ch == 13 || ch == ' ') return CF_KEY_ENTER;
    if (ch == 9) return CF_KEY_HISTORY;
    if (ch == 'f' || ch == 'F') return CF_KEY_FART;
    if (ch == 's' || ch == 'S') return CF_KEY_SAVE;
    if (ch == 'l' || ch == 'L') return CF_KEY_LOAD;
    if (ch == 'd' || ch == 'D') return CF_KEY_DIFFICULTY;
    if (ch == 'h' || ch == 'H' || ch == '?') return CF_KEY_HELP;
    if (ch == 'm' || ch == 'M') return CF_KEY_HISTORY;
    if (ch == 'c' || ch == 'C') return CF_KEY_CREDITS;
    if (ch == 'r' || ch == 'R') return CF_KEY_REPLAY;
    if (ch == 'u' || ch == 'U') return CF_KEY_UNDO;
    if (ch == 't' || ch == 'T') return CF_KEY_THEME;
    return CF_KEY_NONE;
}
