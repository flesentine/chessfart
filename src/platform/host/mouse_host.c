#include "mouse.h"
int mouse_init(void) { return 0; }
void mouse_shutdown(void) { }
int mouse_poll(CfMouseState *state)
{
    if (state != 0) {
        state->present = 0;
        state->x = 0;
        state->y = 0;
        state->buttons = 0U;
    }
    return 0;
}
