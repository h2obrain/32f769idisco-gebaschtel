/*
 * hbftgl-test.c
 *
 *  Created on: 18 Jul 2019
 *      Author: h2obrain
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "drivers/dma2d_helper_functions.h"
#include "support/clock.h"

#include "hbftgl-markup.h"
#include "freetype-gl.h"
#include "font-manager.h"
#include "text-buffer.h"

#include "data/fonts/DejaVuSerif_ttf.h"
#include "data/fonts/amiri-0.104/amiri-regular_ttf.h"
#include "data/fonts/fireflysung-nano_ttf.h"

#include "data/texts/lorem_ipsum_dolor.h"

//#define PRELOAD_GLYPHS

static font_manager_t *font_manager;
static text_buffer_t  *buffer;

void hbftgl_markup_init(dma2d_pixel_buffer_t *render_surface) {
	dma2d_fill(render_surface, 0xff000000, 0,0,render_surface->width, render_surface->height);

	gfx_color_t black  = {.argb8888.c=0xff000000};
	gfx_color_t white  = {.argb8888.c=0xffffffff};
	gfx_color_t yellow = {.argb8888.c=0xffffff00};
	gfx_color_t grey   = {.argb8888.c=0xff888888};
	gfx_color_t none   = {.argb8888.c=0x00ffffff};

	float dpi = 216; // 72 216 233.238

	memory_font_t DejaVuSerif      = { (void *)DejaVuSerif_ttf, DejaVuSerif_ttf_size };
	memory_font_t amiri_regular    = { (void *)amiri_regular_ttf, amiri_regular_ttf_size };
	memory_font_t fireflysung_nano = { (void *)fireflysung_nano_ttf, fireflysung_nano_ttf_size };

	font_manager = font_manager_new(512, 512, LCD_FILTERING_OFF, dpi,1);
	buffer = text_buffer_new(800,480, render_surface);

	markup_t normal = {
		.language  = "en",
		.direction = HB_DIRECTION_LTR,
		.script    = HB_SCRIPT_LATIN,
		.family    = DejaVuSerif,
		.size      = 12.0, .bold    = 0,   .italic  = 0,
		.spacing   = 0.0,  //.gamma   = 2.,
		.foreground_color    = white, .background_color    = none,
		.underline           = 0,     .underline_color     = white,
		.overline            = 0,     .overline_color      = white,
		.strikethrough       = 0,     .strikethrough_color = white,
		.font = 0,
	};
	markup_t highlight = normal; highlight.background_color = grey;
	markup_t reverse   = normal; reverse.foreground_color = black;
								 reverse.background_color = white;
								 //reverse.gamma = 1.0;
	markup_t overline  = normal; overline.overline = true;
	markup_t underline = normal; underline.underline = true;
	markup_t small     = normal; small.size = 8.0;
								 small.outline = true; small.outline_color = (gfx_color_t){.argb8888.c = 0xff11aaff};
	markup_t big       = normal; big.size = 30.0;
								 big.italic = true;
								 big.strikethrough = true;
								 big.foreground_color = yellow;
								 big.overline = big.underline = true;
								 big.outline = true; big.outline_color = (gfx_color_t){.argb8888.c = 0xff11aaff};
	markup_t bold      = normal; bold.bold = true; bold.family = DejaVuSerif;
	markup_t italic    = normal; italic.italic = true; italic.family = DejaVuSerif;
	markup_t chinese   = normal; chinese.family = fireflysung_nano;
								 chinese.size = 15.0;
								 chinese.language = "cn";
								 chinese.direction = HB_DIRECTION_TTB;
								 chinese.script = HB_SCRIPT_HAN; /* fixme */
	markup_t chinese_big = chinese; chinese_big.family = fireflysung_nano;
									  chinese_big.size = 32.0;
	markup_t arabic    = normal; arabic.family = amiri_regular;
								 arabic.size = 15.0;
								 arabic.language = "ar";
								 arabic.direction = HB_DIRECTION_RTL;
								 arabic.script = HB_SCRIPT_ARABIC;
	markup_t math      = normal; math.family = DejaVuSerif;

	normal.font = font_manager_get_from_markup(font_manager, &normal);
	highlight.font = font_manager_get_from_markup(font_manager, &highlight);
	reverse.font = font_manager_get_from_markup(font_manager, &reverse);
	overline.font = font_manager_get_from_markup(font_manager, &overline);
	underline.font = font_manager_get_from_markup(font_manager, &underline);
	small.font = font_manager_get_from_markup(font_manager, &small);
	big.font = font_manager_get_from_markup(font_manager, &big);
	bold.font = font_manager_get_from_markup(font_manager, &bold);
	italic.font = font_manager_get_from_markup(font_manager, &italic);
	chinese.font = font_manager_get_from_markup(font_manager, &chinese);
	chinese_big.font = font_manager_get_from_markup(font_manager, &chinese_big);
	arabic.font = font_manager_get_from_markup(font_manager, &arabic);
	math.font = font_manager_get_from_markup(font_manager, &math);

//	vec2 penx = {{20, HEIGHT-20}};
//	text_buffer_printf( buffer, &penx, &japanese,"私", NULL);

	vec2 pen = {{20, 20}};
	text_buffer_printf( buffer, &pen,
						&underline, "The",
						&normal,    " Quick",
						&big,       " bRown ",
						&normal,    "\nThe Quick",
//						&reverse,   " fox \n",
//						&italic,    "jumps over ",
//						&bold,      "the lazy ",
//						&overline,  "dog.\n",
//						&small,     "Now is the time for all good people "
//									"to come to party.\n",
//						&italic,    "Ég get etið gler án þess að meiða mig.\n",
//						&math,      "ℕ ⊆ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂ\n",
////						&normal,    "<ttb>",
////						&chinese,  "私はガラスを食べられます。\nそれは私を傷つけません。\n",
////						&chinese,  "私はガラス",
////						&chinese_big,"を食べられ",
////						&chinese,  "ます。\n",
////						&chinese,  "bla\nblabla\nblablabla\n",
////						&normal,    "</ttb>\n",
//						&normal,    "<rtl>",
//						&arabic,    "كسول الز123نجبيل القط",
//						&normal,    "</rtl>\n",
						NULL );

//	/* display atlas */
//	atlas_surface.in.pixel.alpha_mode.color = 0xffffff;
//	dma2d_convert_copy__no_pxsrc_fix(
//			&atlas_surface, render_surface,
//			0,0, 0,0,
//			atlas_surface.width, atlas_surface.height
//		);
}
