#include <ctype.h>
#include <string.h>

#include "font.h"
#include "vga.h"

typedef struct FontGlyph {
    char ch;
    cf_u8 row[7];
} FontGlyph;

static const FontGlyph k_glyphs[] = {
    {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}},
    {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}},
    {'E',{31,16,16,30,16,16,31}}, {'F',{31,16,16,30,16,16,16}},
    {'G',{14,17,16,23,17,17,15}}, {'H',{17,17,17,31,17,17,17}},
    {'I',{31,4,4,4,4,4,31}}, {'J',{7,2,2,2,18,18,12}},
    {'K',{17,18,20,24,20,18,17}}, {'L',{16,16,16,16,16,16,31}},
    {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}},
    {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}},
    {'Q',{14,17,17,17,21,18,13}}, {'R',{30,17,17,30,20,18,17}},
    {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}},
    {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}},
    {'W',{17,17,17,21,21,21,10}}, {'X',{17,17,10,4,10,17,17}},
    {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}},
    {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}},
    {'2',{14,17,1,2,4,8,31}}, {'3',{30,1,1,14,1,1,30}},
    {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}},
    {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}},
    {'8',{14,17,17,14,17,17,14}}, {'9',{14,17,17,15,1,1,14}},
    {':',{0,4,4,0,4,4,0}}, {'-',{0,0,0,31,0,0,0}},
    {'.',{0,0,0,0,0,12,12}}, {'/',{1,2,2,4,8,8,16}},
    {'!',{4,4,4,4,4,0,4}}, {' ',{0,0,0,0,0,0,0}}
};

static const cf_u8 *find_glyph(char ch)
{
    unsigned int i;
    char upper = (char)toupper((unsigned char)ch);
    for (i = 0; i < sizeof(k_glyphs) / sizeof(k_glyphs[0]); ++i) {
        if (k_glyphs[i].ch == upper) {
            return k_glyphs[i].row;
        }
    }
    return k_glyphs[sizeof(k_glyphs) / sizeof(k_glyphs[0]) - 1].row;
}

void font_draw_char(int x, int y, char ch, cf_u8 color, int scale)
{
    const cf_u8 *glyph = find_glyph(ch);
    int row;
    int col;
    int sx;
    int sy;

    if (scale < 1) {
        scale = 1;
    }

    for (row = 0; row < 7; ++row) {
        for (col = 0; col < 5; ++col) {
            if ((glyph[row] & (1U << (4 - col))) != 0U) {
                for (sy = 0; sy < scale; ++sy) {
                    for (sx = 0; sx < scale; ++sx) {
                        vga_put_pixel(x + col * scale + sx,
                                      y + row * scale + sy,
                                      color);
                    }
                }
            }
        }
    }
}

void font_draw_text(int x, int y, const char *text, cf_u8 color, int scale)
{
    int cursor = x;
    if (text == 0) {
        return;
    }
    while (*text != '\0') {
        font_draw_char(cursor, y, *text, color, scale);
        cursor += 6 * scale;
        ++text;
    }
}

int font_text_width(const char *text, int scale)
{
    int len;
    if (text == 0) {
        return 0;
    }
    if (scale < 1) {
        scale = 1;
    }
    len = (int)strlen(text);
    if (len == 0) {
        return 0;
    }
    return len * 6 * scale - scale;
}
