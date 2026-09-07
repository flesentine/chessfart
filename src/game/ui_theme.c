#include "ui_theme.h"
#include "vga.h"

static CfUiTheme g_ui_theme = CF_UI_THEME_ROYAL_BASEMENT;

static void set_rgb(cf_u8 *palette, int index, int r, int g, int b)
{
    int base = index * 3;
    palette[base] = (cf_u8)r;
    palette[base + 1] = (cf_u8)g;
    palette[base + 2] = (cf_u8)b;
}

static void build_royal_basement(cf_u8 *palette, int flash)
{
    set_rgb(palette, CF_UI_COL_BG, 1, 4, 11);
    set_rgb(palette, CF_UI_COL_PANEL, 3, 9, 23);
    set_rgb(palette, CF_UI_COL_SQUARE_LIGHT, 55, 47, 29);
    set_rgb(palette, CF_UI_COL_SQUARE_DARK, 6, 35, 31);
    set_rgb(palette, CF_UI_COL_GOLD,
            flash ? 63 : 58, flash ? 63 : 45, 9);
    set_rgb(palette, CF_UI_COL_TEXT, 61, 58, 49);
    set_rgb(palette, CF_UI_COL_MUTED, 28, 31, 37);
    set_rgb(palette, CF_UI_COL_GREEN, 15, 48, 23);
    set_rgb(palette, CF_UI_COL_GAS,
            flash ? 63 : 38, 58, flash ? 35 : 13);
    set_rgb(palette, CF_UI_COL_BLACK, 0, 0, 0);
    set_rgb(palette, CF_UI_COL_PANEL_EDGE, 13, 19, 32);
    set_rgb(palette, CF_UI_COL_WHITE_PIECE, 58, 52, 41);
    set_rgb(palette, CF_UI_COL_WHITE_HI, 63, 62, 55);
    set_rgb(palette, CF_UI_COL_BLACK_PIECE, 3, 8, 14);
    set_rgb(palette, CF_UI_COL_BLACK_HI, 23, 33, 39);
    set_rgb(palette, CF_UI_COL_CURSOR, 6, 51, 59);
    set_rgb(palette, CF_UI_COL_SELECTED, 8, 59, 54);
    set_rgb(palette, CF_UI_COL_SHADOW, 0, 1, 3);
    set_rgb(palette, CF_UI_COL_LEGAL, 54, 43, 7);
    set_rgb(palette, CF_UI_COL_CAPTURE, 58, 21, 7);
    set_rgb(palette, CF_UI_COL_CHECK, 61, 9, 9);
    set_rgb(palette, CF_UI_COL_PROMOTE, 48, 23, 56);
    set_rgb(palette, CF_UI_COL_FART,
            flash ? 63 : 34, 56, 11);
    set_rgb(palette, CF_UI_COL_FART_PUSH, 56, 35, 8);
    set_rgb(palette, CF_UI_COL_SKY, 2, 10, 25);
    set_rgb(palette, CF_UI_COL_CLOUD, 26, 47, 13);
    set_rgb(palette, CF_UI_COL_FLASH, 63, 63, 40);
    set_rgb(palette, CF_UI_COL_COPPER, 43, 27, 11);
    set_rgb(palette, CF_UI_COL_PANEL_LINE, 24, 31, 43);
    set_rgb(palette, CF_UI_COL_PANEL_SOFT, 8, 15, 29);
}

static void build_crimson_cellar(cf_u8 *palette, int flash)
{
    set_rgb(palette, CF_UI_COL_BG, 7, 1, 4);
    set_rgb(palette, CF_UI_COL_PANEL, 17, 4, 8);
    set_rgb(palette, CF_UI_COL_SQUARE_LIGHT, 52, 44, 31);
    set_rgb(palette, CF_UI_COL_SQUARE_DARK, 24, 8, 12);
    set_rgb(palette, CF_UI_COL_GOLD,
            flash ? 63 : 58, flash ? 62 : 39, 13);
    set_rgb(palette, CF_UI_COL_TEXT, 61, 55, 47);
    set_rgb(palette, CF_UI_COL_MUTED, 34, 25, 29);
    set_rgb(palette, CF_UI_COL_GREEN, 16, 47, 24);
    set_rgb(palette, CF_UI_COL_GAS,
            flash ? 63 : 39, 58, flash ? 35 : 13);
    set_rgb(palette, CF_UI_COL_BLACK, 0, 0, 0);
    set_rgb(palette, CF_UI_COL_PANEL_EDGE, 34, 10, 14);
    set_rgb(palette, CF_UI_COL_WHITE_PIECE, 60, 55, 46);
    set_rgb(palette, CF_UI_COL_WHITE_HI, 63, 62, 56);
    set_rgb(palette, CF_UI_COL_BLACK_PIECE, 8, 3, 8);
    set_rgb(palette, CF_UI_COL_BLACK_HI, 37, 20, 27);
    set_rgb(palette, CF_UI_COL_CURSOR, 9, 50, 60);
    set_rgb(palette, CF_UI_COL_SELECTED, 54, 29, 12);
    set_rgb(palette, CF_UI_COL_SHADOW, 2, 0, 1);
    set_rgb(palette, CF_UI_COL_LEGAL, 55, 43, 8);
    set_rgb(palette, CF_UI_COL_CAPTURE, 63, 17, 10);
    set_rgb(palette, CF_UI_COL_CHECK, 63, 7, 7);
    set_rgb(palette, CF_UI_COL_PROMOTE, 49, 22, 57);
    set_rgb(palette, CF_UI_COL_FART,
            flash ? 63 : 35, 56, 11);
    set_rgb(palette, CF_UI_COL_FART_PUSH, 58, 35, 8);
    set_rgb(palette, CF_UI_COL_SKY, 8, 1, 9);
    set_rgb(palette, CF_UI_COL_CLOUD, 27, 47, 13);
    set_rgb(palette, CF_UI_COL_FLASH, 63, 63, 40);
    set_rgb(palette, CF_UI_COL_COPPER, 46, 25, 14);
    set_rgb(palette, CF_UI_COL_PANEL_LINE, 42, 16, 20);
    set_rgb(palette, CF_UI_COL_PANEL_SOFT, 13, 4, 7);
}

int ui_theme_valid(CfUiTheme theme)
{
    return theme >= CF_UI_THEME_ROYAL_BASEMENT &&
           theme < CF_UI_THEME_COUNT;
}

int ui_theme_set(CfUiTheme theme)
{
    if (!ui_theme_valid(theme)) return 0;
    g_ui_theme = theme;
    return 1;
}

CfUiTheme ui_theme_get(void)
{
    return g_ui_theme;
}

CfUiTheme ui_theme_next(CfUiTheme theme)
{
    if (!ui_theme_valid(theme)) return CF_UI_THEME_ROYAL_BASEMENT;
    return (CfUiTheme)(((int)theme + 1) % (int)CF_UI_THEME_COUNT);
}

const char *ui_theme_name(CfUiTheme theme)
{
    switch (theme) {
    case CF_UI_THEME_ROYAL_BASEMENT: return "ROYAL BASEMENT";
    case CF_UI_THEME_CRIMSON_CELLAR: return "CRIMSON CELLAR";
    default: return "UNKNOWN";
    }
}

void ui_theme_build_palette(cf_u8 *palette, int flash)
{
    int i;
    int shade;
    if (palette == 0) return;

    for (i = 0; i < VGA_PALETTE_BYTES; ++i) palette[i] = 0;

    if (g_ui_theme == CF_UI_THEME_CRIMSON_CELLAR)
        build_crimson_cellar(palette, flash);
    else
        build_royal_basement(palette, flash);

    for (i = CF_UI_GRAYSCALE_FIRST; i < VGA_PALETTE_COLORS; ++i) {
        shade = (i - CF_UI_GRAYSCALE_FIRST) * 63 /
                (VGA_PALETTE_COLORS - (CF_UI_GRAYSCALE_FIRST + 1));
        set_rgb(palette, i, shade, shade, shade);
    }
}
