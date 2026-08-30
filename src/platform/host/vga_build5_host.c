#include <stdio.h>
#include <string.h>

#include "vga.h"

static cf_u8 g_buffer[VGA_FRAME_SIZE];
static cf_u8 g_palette[VGA_PALETTE_BYTES];

int vga_init(void)
{
    memset(g_buffer, 0, sizeof(g_buffer));
    memset(g_palette, 0, sizeof(g_palette));
    return 0;
}

void vga_shutdown(void) { }

void vga_set_palette(const cf_u8 *palette)
{
    if (palette != 0) memcpy(g_palette, palette, sizeof(g_palette));
}

void vga_clear(cf_u8 color) { memset(g_buffer, color, sizeof(g_buffer)); }

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
    FILE *fp;
    int x;
    int y;
    int index;
    unsigned char rgb[3];
    fp = fopen("build/host/chessfart_build5.ppm", "wb");
    if (fp == 0) {
        fprintf(stderr, "Unable to write Build 5 preview.\n");
        return;
    }
    fprintf(fp, "P6\n%d %d\n255\n", VGA_WIDTH, VGA_HEIGHT);
    for (y = 0; y < VGA_HEIGHT; ++y) {
        for (x = 0; x < VGA_WIDTH; ++x) {
            index = g_buffer[(unsigned long)y * VGA_WIDTH + (unsigned long)x] * 3;
            rgb[0] = (unsigned char)(g_palette[index] * 255 / 63);
            rgb[1] = (unsigned char)(g_palette[index + 1] * 255 / 63);
            rgb[2] = (unsigned char)(g_palette[index + 2] * 255 / 63);
            fwrite(rgb, 1, 3, fp);
        }
    }
    fclose(fp);
    printf("Build 5 host preview: build/host/chessfart_build5.ppm\n");
}
