/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --font FontAwesome5-Solid+Brands+Regular.woff --size 16 --bpp 1 --no-kerning --symbols abc --lv-font-name lv_font_symbol_16 --format lvgl -o lv_font_symbol_16.c
 ******************************************************************************/

#include <lvgl.h>

#ifndef LV_FONT_SYMBOL_16
#define LV_FONT_SYMBOL_16 1
#endif

#if LV_FONT_SYMBOL_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0061 "a" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0x9d,

    /* U+0062 "b" */
    0x81, 0x2, 0x4, 0xb, 0x98, 0xa0, 0xc1, 0x83,
    0x7, 0x15, 0xc0,

    /* U+0063 "c" */
    0x3d, 0x18, 0x20, 0x82, 0x4, 0x4f,

    /* U+F287 "" */
    0x0, 0x38, 0x0, 0xf, 0x80, 0x1, 0xb8, 0x0,
    0x10, 0x0, 0x63, 0x0, 0xf, 0x20, 0x6, 0xff,
    0xff, 0xff, 0xc, 0x6, 0x60, 0x40, 0x0, 0x6,
    0xf0, 0x0, 0x3f, 0x0, 0x0, 0xf0,

    /* U+F294 "" */
    0x0, 0x8, 0xc, 0xe, 0x4b, 0x6b, 0x3e, 0x1c,
    0x1c, 0x3e, 0x6b, 0xcb, 0xe, 0xc, 0x8, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 146, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 7, .adv_w = 162, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 18, .adv_w = 133, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 24, .adv_w = 320, .box_w = 20, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 160, .box_w = 8, .box_h = 16, .ofs_x = 1, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x1, 0x2, 0xf226, 0xf233
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 97, .range_length = 62004, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 5, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_symbol_16 = {
#else
lv_font_t lv_font_symbol_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -6,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_SYMBOL_16*/

