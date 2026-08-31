#include <dos.h>
#include "mouse.h"

static int g_mouse_present;

int mouse_init(void)
{
    union REGS regs;
    regs.x.ax = 0;
    int86(0x33, &regs, &regs);
    g_mouse_present = regs.x.ax != 0;
    if (!g_mouse_present) return 0;
    regs.x.ax = 7;
    regs.x.cx = 0;
    regs.x.dx = 319;
    int86(0x33, &regs, &regs);
    regs.x.ax = 8;
    regs.x.cx = 0;
    regs.x.dx = 199;
    int86(0x33, &regs, &regs);
    regs.x.ax = 1;
    int86(0x33, &regs, &regs);
    return 1;
}

void mouse_shutdown(void)
{
    union REGS regs;
    if (!g_mouse_present) return;
    regs.x.ax = 2;
    int86(0x33, &regs, &regs);
    g_mouse_present = 0;
}

int mouse_poll(CfMouseState *state)
{
    union REGS regs;
    if (state == 0) return 0;
    state->present = g_mouse_present;
    state->x = 0;
    state->y = 0;
    state->buttons = 0U;
    if (!g_mouse_present) return 0;
    regs.x.ax = 3;
    int86(0x33, &regs, &regs);
    state->x = (int)regs.x.cx;
    state->y = (int)regs.x.dx;
    state->buttons = (unsigned)regs.x.bx;
    return 1;
}
