#include <string.h>
#include <emscripten.h>
#include "vga.h"

static cf_u8 g_buffer[VGA_FRAME_SIZE];
static cf_u8 g_palette[VGA_PALETTE_BYTES];

int vga_init(void)
{
    memset(g_buffer, 0, sizeof(g_buffer));
    memset(g_palette, 0, sizeof(g_palette));
    return 0;
}

void vga_shutdown(void)
{
}

void vga_set_palette(const cf_u8 *palette)
{
    if (palette != 0) memcpy(g_palette, palette, sizeof(g_palette));
}

void vga_clear(cf_u8 color)
{
    memset(g_buffer, color, sizeof(g_buffer));
}

void vga_put_pixel(int x, int y, cf_u8 color)
{
    if (x < 0 || y < 0 || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    g_buffer[(unsigned long)y * VGA_WIDTH + (unsigned long)x] = color;
}

void vga_fill_rect(int x, int y, int w, int h, cf_u8 color)
{
    int ix;
    int iy;
    for (iy = 0; iy < h; ++iy)
        for (ix = 0; ix < w; ++ix)
            vga_put_pixel(x + ix, y + iy, color);
}

void vga_present(void)
{
    EM_ASM({
        var canvas = Module['canvas'] || document.getElementById('canvas');
        if (!canvas) return;
        var ctx = canvas.getContext('2d', { alpha: false });
        var image = Module.cfImageData;
        if (!image || image.width !== 320 || image.height !== 200) {
            image = ctx.createImageData(320, 200);
            Module.cfImageData = image;
        }
        var src = HEAPU8.subarray($0, $0 + 64000);
        var pal = HEAPU8.subarray($1, $1 + 768);
        var dst = image.data;
        var i, p, d;
        for (i = 0; i < 64000; ++i) {
            p = src[i] * 3;
            d = i * 4;
            dst[d] = (pal[p] * 255 / 63) | 0;
            dst[d + 1] = (pal[p + 1] * 255 / 63) | 0;
            dst[d + 2] = (pal[p + 2] * 255 / 63) | 0;
            dst[d + 3] = 255;
        }
        ctx.putImageData(image, 0, 0);
    }, g_buffer, g_palette);
}
