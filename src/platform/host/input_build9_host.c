#include "input_build5.h"

static int g_index;

void input5_init(void) { g_index = 0; }

CfInputKey5 input5_poll_key(void)
{
    static const CfInputKey5 script[] = {
        CF5_KEY_SAVE,
        CF5_KEY_ENTER,
        CF5_KEY_FART,
        CF5_KEY_UP_RIGHT,
        CF5_KEY_ENTER,
        CF5_KEY_LOAD,
        CF5_KEY_ENTER,
        CF5_KEY_FART,
        CF5_KEY_UP_RIGHT,
        CF5_KEY_ENTER,
        CF5_KEY_ESCAPE
    };
    int count = (int)(sizeof(script) / sizeof(script[0]));
    if (g_index >= count) return CF5_KEY_ESCAPE;
    return script[g_index++];
}
