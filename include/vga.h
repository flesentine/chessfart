#ifndef CF_VGA_H
#define CF_VGA_H

#include "cf_types.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_FRAME_SIZE 64000UL
#define VGA_PALETTE_COLORS 256
#define VGA_PALETTE_BYTES 768

int vga_init(void);
void vga_shutdown(void);
void vga_set_palette(const cf_u8 *palette);
void vga_clear(cf_u8 color);
void vga_put_pixel(int x, int y, cf_u8 color);
void vga_fill_rect(int x, int y, int w, int h, cf_u8 color);
void vga_present(void);

#endif
