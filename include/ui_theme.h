#ifndef CF_UI_THEME_H
#define CF_UI_THEME_H

#include "cf_types.h"

/* Canonical retail VGA palette roles. Keep renderer and UX overlays in sync. */
#define CF_UI_COL_BG 0
#define CF_UI_COL_PANEL 1
#define CF_UI_COL_SQUARE_LIGHT 2
#define CF_UI_COL_SQUARE_DARK 3
#define CF_UI_COL_GOLD 4
#define CF_UI_COL_TEXT 5
#define CF_UI_COL_MUTED 6
#define CF_UI_COL_GREEN 7
#define CF_UI_COL_GAS 8
#define CF_UI_COL_BLACK 9
#define CF_UI_COL_PANEL_EDGE 10
#define CF_UI_COL_WHITE_PIECE 11
#define CF_UI_COL_WHITE_HI 12
#define CF_UI_COL_BLACK_PIECE 13
#define CF_UI_COL_BLACK_HI 14
#define CF_UI_COL_CURSOR 15
#define CF_UI_COL_SELECTED 16
#define CF_UI_COL_SHADOW 17
#define CF_UI_COL_LEGAL 18
#define CF_UI_COL_CAPTURE 19
#define CF_UI_COL_CHECK 20
#define CF_UI_COL_PROMOTE 21
#define CF_UI_COL_FART 22
#define CF_UI_COL_FART_PUSH 23
#define CF_UI_COL_SKY 24
#define CF_UI_COL_CLOUD 25
#define CF_UI_COL_FLASH 26
#define CF_UI_COL_COPPER 27
#define CF_UI_COL_PANEL_LINE 28
#define CF_UI_COL_PANEL_SOFT 29

#define CF_UI_ACTIVE_COLOR_COUNT 30
#define CF_UI_GRAYSCALE_FIRST CF_UI_ACTIVE_COLOR_COUNT

typedef enum CfUiTheme {
    CF_UI_THEME_ROYAL_BASEMENT = 0,
    CF_UI_THEME_CRIMSON_CELLAR,
    CF_UI_THEME_COUNT
} CfUiTheme;

int ui_theme_valid(CfUiTheme theme);
int ui_theme_set(CfUiTheme theme);
CfUiTheme ui_theme_get(void);
const char *ui_theme_name(CfUiTheme theme);
void ui_theme_build_palette(cf_u8 *palette, int flash);

#endif
