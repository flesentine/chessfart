#include <conio.h>
#include "input_build5.h"
void input5_init(void) { while (kbhit()) (void)getch(); }
CfInputKey5 input5_poll_key(void)
{
    int ch;
    int extended;
    if (!kbhit()) return CF5_KEY_NONE;
    ch = getch();
    if (ch == 0 || ch == 0xE0) {
        extended = getch();
        switch (extended) {
        case 72: return CF5_KEY_UP;
        case 80: return CF5_KEY_DOWN;
        case 75: return CF5_KEY_LEFT;
        case 77: return CF5_KEY_RIGHT;
        case 71: return CF5_KEY_UP_LEFT;
        case 73: return CF5_KEY_UP_RIGHT;
        case 79: return CF5_KEY_DOWN_LEFT;
        case 81: return CF5_KEY_DOWN_RIGHT;
        default: return CF5_KEY_NONE;
        }
    }
    if (ch == 27) return CF5_KEY_ESCAPE;
    if (ch == 13 || ch == ' ') return CF5_KEY_ENTER;
    if (ch == 9) return CF5_KEY_HISTORY;
    if (ch == 'f' || ch == 'F') return CF5_KEY_FART;
    if (ch == 's' || ch == 'S') return CF5_KEY_SAVE;
    if (ch == 'l' || ch == 'L') return CF5_KEY_LOAD;
    if (ch == 'd' || ch == 'D') return CF5_KEY_DIFFICULTY;
    if (ch == 'h' || ch == 'H' || ch == '?') return CF5_KEY_HELP;
    if (ch == 'm' || ch == 'M') return CF5_KEY_HISTORY;
    if (ch == 'c' || ch == 'C') return CF5_KEY_CREDITS;
    if (ch == 'r' || ch == 'R') return CF5_KEY_REPLAY;
    if (ch == 'u' || ch == 'U') return CF5_KEY_UNDO;
    return CF5_KEY_NONE;
}
