/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --font JetBrainsMono-Bold.ttf --size 16 --bpp 1 --no-kerning --symbols USBLE0123456789● --format lvgl --lv-font-name lv_font_jetbrainsmono_16 -o lv_font_jetbrainsmono_16.c
 ******************************************************************************/

#include <lvgl.h>

#ifndef LV_FONT_JETBRAINSMONO_16
#define LV_FONT_JETBRAINSMONO_16 1
#endif

#if LV_FONT_JETBRAINSMONO_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x38, 0xfb, 0x1e, 0x3c, 0x7b, 0xf7, 0xe3, 0xc7,
    0x8d, 0xf1, 0xc0,

    /* U+0031 "1" */
    0x38, 0xf3, 0x64, 0xc1, 0x83, 0x6, 0xc, 0x18,
    0x33, 0xff, 0xf0,

    /* U+0032 "2" */
    0x38, 0xfb, 0x1e, 0x30, 0x61, 0x87, 0x1c, 0x71,
    0xc3, 0xff, 0xf0,

    /* U+0033 "3" */
    0xfe, 0xfe, 0x6, 0xc, 0x38, 0x3e, 0x3, 0x3,
    0xc3, 0xc3, 0x7e, 0x3c,

    /* U+0034 "4" */
    0xc, 0x30, 0x61, 0x87, 0xc, 0xf9, 0xe3, 0xff,
    0xfc, 0x18, 0x30,

    /* U+0035 "5" */
    0xff, 0xff, 0xc0, 0xc0, 0xfc, 0xfe, 0x3, 0x3,
    0xc3, 0xc3, 0x7e, 0x3c,

    /* U+0036 "6" */
    0x1c, 0x18, 0x30, 0x30, 0x7e, 0x7e, 0xe7, 0xc3,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+0037 "7" */
    0xff, 0xff, 0xc3, 0xc6, 0x6, 0xc, 0xc, 0x1c,
    0x18, 0x18, 0x30, 0x30,

    /* U+0038 "8" */
    0x3c, 0x7e, 0xc7, 0xc3, 0xc3, 0x7e, 0x7e, 0xe7,
    0xc3, 0xc3, 0x7e, 0x3c,

    /* U+0039 "9" */
    0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0x3e,
    0xc, 0xc, 0x18, 0x38,

    /* U+0042 "B" */
    0xfc, 0xfe, 0xc6, 0xc6, 0xc6, 0xfc, 0xfe, 0xc7,
    0xc3, 0xc3, 0xfe, 0xfc,

    /* U+0045 "E" */
    0xff, 0xff, 0x6, 0xc, 0x1f, 0xff, 0xe0, 0xc1,
    0x83, 0xff, 0xf0,

    /* U+004C "L" */
    0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1,
    0x83, 0xff, 0xf0,

    /* U+0053 "S" */
    0x78, 0xfc, 0xc6, 0xc0, 0xe0, 0x7c, 0x1e, 0x3,
    0xc3, 0xe3, 0x7e, 0x3c,

    /* U+0055 "U" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0xfb, 0xe0,

    /* U+25CF "●" */
    0x1e, 0x1f, 0xe7, 0xfb, 0xff, 0xff, 0xff, 0xff,
    0xfd, 0xfe, 0x7f, 0x87, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 154, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x3, 0xa, 0x11, 0x13, 0x258d
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 66, .range_length = 9614, .glyph_id_start = 11,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 6, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
    .cmap_num = 2,
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
const lv_font_t lv_font_jetbrainsmono_16 = {
#else
lv_font_t lv_font_jetbrainsmono_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_JETBRAINSMONO_16*/

