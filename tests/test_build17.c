#include <stdio.h>
#include <string.h>

#include "ui_theme.h"

static int failures;
#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; \
} } while (0)

static int role_differs(const cf_u8 *a, const cf_u8 *b, int role)
{
    int base = role * 3;
    return a[base] != b[base] ||
           a[base + 1] != b[base + 1] ||
           a[base + 2] != b[base + 2];
}

static void test_theme_identity(void)
{
    CHECK(CF_UI_THEME_ROYAL_BASEMENT == 0);
    CHECK(CF_UI_THEME_CRIMSON_CELLAR == 1);
    CHECK(CF_UI_THEME_COUNT == 2);
    CHECK(ui_theme_valid(CF_UI_THEME_ROYAL_BASEMENT));
    CHECK(ui_theme_valid(CF_UI_THEME_CRIMSON_CELLAR));
    CHECK(!ui_theme_valid((CfUiTheme)-1));
    CHECK(!ui_theme_valid(CF_UI_THEME_COUNT));
    CHECK(strcmp(ui_theme_name(CF_UI_THEME_ROYAL_BASEMENT),
                 "ROYAL BASEMENT") == 0);
    CHECK(strcmp(ui_theme_name(CF_UI_THEME_CRIMSON_CELLAR),
                 "CRIMSON CELLAR") == 0);
}

static void test_palette_switch(void)
{
    cf_u8 royal[VGA_PALETTE_BYTES];
    cf_u8 crimson[VGA_PALETTE_BYTES];
    cf_u8 flash[VGA_PALETTE_BYTES];
    int i;

    CHECK(ui_theme_set(CF_UI_THEME_ROYAL_BASEMENT));
    CHECK(ui_theme_get() == CF_UI_THEME_ROYAL_BASEMENT);
    ui_theme_build_palette(royal, 0);

    CHECK(ui_theme_set(CF_UI_THEME_CRIMSON_CELLAR));
    CHECK(ui_theme_get() == CF_UI_THEME_CRIMSON_CELLAR);
    ui_theme_build_palette(crimson, 0);
    ui_theme_build_palette(flash, 1);

    CHECK(role_differs(royal, crimson, CF_UI_COL_BG));
    CHECK(role_differs(royal, crimson, CF_UI_COL_PANEL));
    CHECK(role_differs(royal, crimson, CF_UI_COL_SQUARE_LIGHT));
    CHECK(role_differs(royal, crimson, CF_UI_COL_SQUARE_DARK));
    CHECK(role_differs(royal, crimson, CF_UI_COL_BLACK_HI));
    CHECK(role_differs(crimson, flash, CF_UI_COL_GOLD));
    CHECK(role_differs(crimson, flash, CF_UI_COL_GAS));
    CHECK(role_differs(crimson, flash, CF_UI_COL_FART));

    for (i = 0; i < VGA_PALETTE_BYTES; ++i)
        CHECK(crimson[i] <= 63U);

    CHECK(!ui_theme_set((CfUiTheme)99));
    CHECK(ui_theme_get() == CF_UI_THEME_CRIMSON_CELLAR);
    CHECK(ui_theme_set(CF_UI_THEME_ROYAL_BASEMENT));
}

int main(void)
{
    test_theme_identity();
    test_palette_switch();

    if (failures != 0) {
        printf("Build 17 theme tests failed: %d\n", failures);
        return 1;
    }
    printf("Build 17 theme tests passed.\n");
    return 0;
}
