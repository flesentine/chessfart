#include <conio.h>
#include <dos.h>
#include <malloc.h>

#include "vga.h"

static cf_u8 __far *g_backbuffer = 0;
static cf_u8 __far *g_vram = (cf_u8 __far *)MK_FP(0xA000, 0x0000);

int vga_init(void)
{
    union REGS regs;

    g_backbuffer = (cf_u8 __far *)_fmalloc(VGA_FRAME_SIZE);
    if (g_backbuffer == 0) {
        return -1;
    }

    regs.w.ax = 0x0013;
    int86(0x10, &regs, &regs);
    return 0;
}

void vga_shutdown(void)
{
    union REGS regs;

    regs.w.ax = 0x0003;
    int86(0x10, &regs, &regs);

    if (g_backbuffer != 0) {
        _ffree(g_backbuffer);
        g_backbuffer = 0;
    }
}

void vga_set_palette(const cf_u8 *palette)
{
    int i;
    if (palette == 0) {
        return;
    }
    outp(0x3C8, 0);
    for (i = 0; i < VGA_PALETTE_BYTES; ++i) {
        outp(0x3C9, palette[i]);
    }
}

void vga_clear(cf_u8 color)
{
    unsigned long i;
    if (g_backbuffer == 0) {
        return;
    }
    for (i = 0; i < VGA_FRAME_SIZE; ++i) {
        g_backbuffer[i] = color;
    }
}

void vga_put_pixel(int x, int y, cf_u8 color)
{
    unsigned long offset;
    if (g_backbuffer == 0 || x < 0 || y < 0 || x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }
    offset = (unsigned long)y * VGA_WIDTH + (unsigned long)x;
    g_backbuffer[offset] = color;
}

void vga_fill_rect(int x, int y, int w, int h, cf_u8 color)
{
    int ix;
    int iy;
    for (iy = 0; iy < h; ++iy) {
        for (ix = 0; ix < w; ++ix) {
            vga_put_pixel(x + ix, y + iy, color);
        }
    }
}

void vga_present(void)
{
    unsigned long i;
    if (g_backbuffer == 0) {
        return;
    }
    for (i = 0; i < VGA_FRAME_SIZE; ++i) {
        g_vram[i] = g_backbuffer[i];
    }
}
