/*******************************************************************************
 * Size: 24 px
 * Bpp: 1
 * Opts: --font JetBrainsMono-Bold.ttf --size 24 --bpp 1 --no-kerning --symbols 0123456789 --format lvgl --lv-font-name lv_font_jetbrainsmono_24 -o lv_font_jetbrainsmono_24.c
 ******************************************************************************/

#include <lvgl.h>

#ifndef LV_FONT_JETBRAINSMONO_24
#define LV_FONT_JETBRAINSMONO_24 1
#endif

#if LV_FONT_JETBRAINSMONO_24

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x1f, 0xf, 0xf9, 0xc7, 0x70, 0x7e, 0xf, 0xc1,
    0xf8, 0x3f, 0x7, 0xee, 0xfd, 0xdf, 0x83, 0xf0,
    0x7e, 0xf, 0xc1, 0xf8, 0x3b, 0x8e, 0x7f, 0xc3,
    0xe0,

    /* U+0031 "1" */
    0xf, 0x7, 0xe1, 0xfc, 0x7f, 0x8e, 0x71, 0x8e,
    0x1, 0xc0, 0x38, 0x7, 0x0, 0xe0, 0x1c, 0x3,
    0x80, 0x70, 0xe, 0x1, 0xc0, 0x38, 0xff, 0xff,
    0xfc,

    /* U+0032 "2" */
    0x1f, 0xf, 0xf9, 0xc7, 0x70, 0x7e, 0xe, 0x1,
    0xc0, 0x38, 0xf, 0x1, 0xc0, 0x78, 0x1e, 0x7,
    0x81, 0xe0, 0x78, 0x1e, 0x7, 0x80, 0xff, 0xff,
    0xfc,

    /* U+0033 "3" */
    0x7f, 0xef, 0xfc, 0x7, 0x1, 0xc0, 0x70, 0x1c,
    0x7, 0xc0, 0xfc, 0x1f, 0xc0, 0x3c, 0x3, 0x80,
    0x70, 0xf, 0xc1, 0xf8, 0x3f, 0x8e, 0x7f, 0xc3,
    0xe0,

    /* U+0034 "4" */
    0x3, 0xc0, 0x70, 0x1c, 0x7, 0x80, 0xe0, 0x3c,
    0xf, 0x1, 0xc0, 0x78, 0xee, 0x1f, 0x83, 0xf0,
    0x7f, 0xff, 0xff, 0xc0, 0x38, 0x7, 0x0, 0xe0,
    0x1c,

    /* U+0035 "5" */
    0x7f, 0xcf, 0xf9, 0xc0, 0x38, 0x7, 0x0, 0xe0,
    0x1f, 0xc3, 0xfe, 0x1, 0xc0, 0x1c, 0x3, 0x80,
    0x70, 0xf, 0xc1, 0xf8, 0x3b, 0x8e, 0x7f, 0xc3,
    0xe0,

    /* U+0036 "6" */
    0x7, 0x80, 0x70, 0xf, 0x0, 0xe0, 0x1c, 0x3,
    0xc0, 0x38, 0x7, 0xf8, 0x7f, 0xe7, 0xe, 0xe0,
    0x7e, 0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x77, 0xe,
    0x3f, 0xc1, 0xf8,

    /* U+0037 "7" */
    0xff, 0xff, 0xff, 0xe0, 0xfe, 0xe, 0xe0, 0xe0,
    0x1e, 0x1, 0xc0, 0x3c, 0x3, 0x80, 0x38, 0x7,
    0x80, 0x70, 0xf, 0x0, 0xe0, 0xe, 0x1, 0xe0,
    0x1c, 0x3, 0xc0,

    /* U+0038 "8" */
    0x1f, 0x7, 0xfc, 0xf1, 0xee, 0xe, 0xe0, 0xee,
    0xe, 0x71, 0xc3, 0xf8, 0x3f, 0xc7, 0xe, 0xe0,
    0x7e, 0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x77, 0xe,
    0x3f, 0xc1, 0xf8,

    /* U+0039 "9" */
    0x1f, 0x83, 0xfc, 0x70, 0xee, 0x7, 0xe0, 0x7e,
    0x7, 0xe0, 0x7e, 0x7, 0x70, 0xe3, 0xfe, 0x1f,
    0xe0, 0x1c, 0x3, 0xc0, 0x78, 0x7, 0x0, 0xf0,
    0xe, 0x1, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 230, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 25, .adv_w = 230, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 230, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 230, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 230, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 230, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 230, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 230, .box_w = 12, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 230, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 230, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
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
const lv_font_t lv_font_jetbrainsmono_24 = {
#else
lv_font_t lv_font_jetbrainsmono_24 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -4,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_JETBRAINSMONO_24*/

