#ifndef CF_FONT_H
#define CF_FONT_H

#include "cf_types.h"

void font_draw_char(int x, int y, char ch, cf_u8 color, int scale);
void font_draw_text(int x, int y, const char *text, cf_u8 color, int scale);
void font_draw_heading(int x, int y, const char *text,
                       cf_u8 color, int scale);
void font_draw_text_clipped(int x, int y, const char *text,
                            cf_u8 color, int scale, int max_width);
int font_text_width(const char *text, int scale);

#endif
