#include "input.h"

static int g_index;

void input_init(void)
{
    g_index = 0;
}

CfInputKey input_poll_key(void)
{
    static const CfInputKey script[] = {
        CF_KEY_ENTER,
        CF_KEY_RIGHT,
        CF_KEY_RIGHT,
        CF_KEY_UP,
        CF_KEY_ESCAPE
    };
    int count = (int)(sizeof(script) / sizeof(script[0]));

    if (g_index >= count) {
        return CF_KEY_ESCAPE;
    }
    return script[g_index++];
}
