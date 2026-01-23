/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: --font FontAwesome5-Solid+Brands+Regular.woff --size 20 --bpp 1 --no-kerning --symbols abc --lv-font-name lv_font_symbol_20 --format lvgl -o lv_font_symbol_20.c
 ******************************************************************************/

#include <lvgl.h>

#ifndef LV_FONT_SYMBOL_20
#define LV_FONT_SYMBOL_20 1
#endif

#if LV_FONT_SYMBOL_20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0061 "a" */
    0x3e, 0xd8, 0x76, 0x1f, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0x61, 0xd8, 0x73, 0xec,

    /* U+0062 "b" */
    0xc0, 0x30, 0xc, 0x3, 0x0, 0xc0, 0x37, 0xce,
    0x1b, 0x86, 0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0,
    0xf8, 0x6e, 0x1b, 0x78,

    /* U+0063 "c" */
    0x1f, 0x38, 0xd8, 0x18, 0xc, 0x6, 0x3, 0x1,
    0x80, 0x60, 0x38, 0xc7, 0xc0,

    /* U+F0E7 "" */
    0x7f, 0x83, 0xfc, 0x1f, 0xc0, 0xfe, 0x7, 0xf0,
    0x3f, 0x3, 0xff, 0xdf, 0xfe, 0xff, 0xf7, 0xff,
    0x3f, 0xf8, 0xf, 0x80, 0x78, 0x3, 0xc0, 0x1c,
    0x0, 0xe0, 0xe, 0x0, 0x70, 0x3, 0x0, 0x10,
    0x0,

    /* U+F287 "" */
    0x0, 0x7, 0x0, 0x0, 0xf, 0xc0, 0x0, 0x1f,
    0xe0, 0x0, 0x8, 0xe0, 0x0, 0xc, 0x0, 0x3,
    0x84, 0x0, 0x23, 0xe6, 0x0, 0x19, 0xff, 0xff,
    0xff, 0xf8, 0x30, 0x6, 0x38, 0x8, 0x2, 0x0,
    0x6, 0x0, 0x0, 0x1, 0x3c, 0x0, 0x0, 0xfe,
    0x0, 0x0, 0x1f, 0x0, 0x0, 0x7, 0x80,

    /* U+F294 "" */
    0x0, 0x0, 0x80, 0x18, 0x3, 0x80, 0x78, 0x8d,
    0xb9, 0x9b, 0xb6, 0x3f, 0x83, 0xe0, 0x38, 0x7,
    0x81, 0xf8, 0x6d, 0x99, 0x9a, 0x36, 0x7, 0x80,
    0xe0, 0x18, 0x2, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 183, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 14, .adv_w = 202, .box_w = 10, .box_h = 16, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 34, .adv_w = 166, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 47, .adv_w = 200, .box_w = 13, .box_h = 20, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 80, .adv_w = 400, .box_w = 25, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 200, .box_w = 11, .box_h = 21, .ofs_x = 0, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x1, 0x2, 0xf086, 0xf226, 0xf233
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 97, .range_length = 62004, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 6, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
const lv_font_t lv_font_symbol_20 = {
#else
lv_font_t lv_font_symbol_20 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 21,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -7,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_SYMBOL_20*/
