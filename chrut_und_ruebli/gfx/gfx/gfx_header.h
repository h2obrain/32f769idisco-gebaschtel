/*
 * gfx-header.h
 *
 *  Created on: 6 Aug 2017
 *      Author: Oliver Meier
 */


#ifndef GFX_COLOR_MODE
#error "Do not include this file directly!"
#endif

#ifndef swap_i16
#define swap_i16(a, b) { int16_t t = a; a = b; b = t; }
#endif

#define PASTER(x,y) x ## y
#define EVALUATOR(x,y)  PASTER(x,y)
#define GFX_FCT(fun) EVALUATOR(GFXV, fun)

#if   GFX_COLOR_MODE==GFX_COLOR_MODE_ARGB8888
#define GFX_COLOR_SIZE 32
#define GFXV gfx_argb8888_

#elif GFX_COLOR_MODE==GFX_COLOR_MODE_RGB888
#define GFX_COLOR_SIZE 24
#define GFXV gfx_rgb888_

#elif GFX_COLOR_MODE==GFX_COLOR_MODE_RGB565
#define GFX_COLOR_SIZE 16
#define GFXV gfx_rgb565_

#elif GFX_COLOR_MODE==GFX_COLOR_MODE_MONOCHROME
#define GFX_COLOR_SIZE 1
#define GFXV gfx_mono_
#define GFX_MONO_COLOR_ON  (gfx_color_t){.mono=true}
#define GFX_MONO_COLOR_OFF (gfx_color_t){.mono=false}
#undef GFX_DMA2D_FONTS
#define GFX_DMA2D_FONTS 0

#endif

#if GFX_COLOR_MODE==GFX_DEFAULT_COLOR_MODE
#undef GFXV
#define GFXV gfx_
#endif

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../gfx_config.h"
#include "../utf8.h"
#include "../fonts/fonts.h"

#if GFX_DMA2D_FONTS
#include "../../drivers/dma2d_helper_functions.h"
#endif

#ifndef GFX_HEADER
#define GFX_HEADER

typedef enum {
	GFX_ROTATION_0_DEGREES,
	GFX_ROTATION_90_DEGREES,
	GFX_ROTATION_180_DEGREES,
	GFX_ROTATION_270_DEGREES
} gfx_rotation_t;

typedef enum {
	GFX_ALIGNMENT_TOP    = 1<<0,
	GFX_ALIGNMENT_BOTTOM = 1<<1,
	GFX_ALIGNMENT_LEFT   = 1<<2,
	GFX_ALIGNMENT_RIGHT  = 1<<3,
	GFX_ALIGNMENT_CENTER = 1<<4,
} gfx_alignment_t;

typedef struct {
	int16_t x1, y1, x2, y2;
} visible_area_t;

// meh.. this ignore does not work
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked-bitfield-compat"
typedef union {
	bool mono;
	union {
		uint16_t c;
		struct __attribute__((packed)) {
			uint8_t b:5;
			uint8_t g:6;
			uint8_t r:5;
		};
	} rgb565;
//	union {
//		uint32_t c;
//		struct {
//			uint8_t a:8;
//			uint8_t b:8;
//			uint8_t g:8;
//			uint8_t r:8;
//		};
//	} rgba8888;
	union {
		uint32_t c;
		struct {
			uint8_t b:8;
			uint8_t g:8;
			uint8_t r:8;
			uint8_t a:8;
		};
	} argb8888;
	union {
		uint32_t c;
		struct {
			uint8_t b:8;
			uint8_t g:8;
			uint8_t r:8;
		};
	} rgb888;
	uint32_t raw;
} gfx_color_t;
#pragma GCC diagnostic pop

typedef struct {
	uint32_t is_offscreen_rendering;
	const int16_t width_orig, height_orig;
	const uint32_t pixel_count;
	int16_t width, height;
	visible_area_t visible_area;
	int16_t cursor_x, cursor_y, cursor_x_orig;
	gfx_color_t font_color;
	uint8_t font_scale;
	gfx_rotation_t rotation;
	bool text_wrap;
	const font_t *font;
	void *surface; /* current pixel buffer */
#if GFX_DMA2D_FONTS
	dma2d_pixel_buffer_t surface_pxbuf, font_pxbuf;
#endif
} gfx_state_t;

//extern gfx_state_t __gfx_state;

typedef struct {
	size_t count_max;
	size_t count_total;
	size_t overflows;
} fill_segment_queue_statistics_t;


#ifndef _GFX_COLOR_CONVERSIONS
#define _GFX_COLOR_CONVERSIONS

static inline uint32_t rgb888_from_rgb565(uint16_t rgb565) {
	uint32_t rgb565_32 = (uint32_t)rgb565;
	return ((((rgb565_32) & 0xF800)   >> (11-8))/31)<<16
	       | ((((rgb565_32) & 0x07E0) <<  (8-5))/63)<<8
	       | ((((rgb565_32) & 0x001F) <<  (8-0))/31)<<0;
}
static inline uint16_t rgb565_from_rgb888(uint32_t rgb888) {
	return (uint16_t)(
			((rgb888&0xf80000)>>8)
		  | ((rgb888&0x00fc00)>>5)
		  | ((rgb888&0x0000f8)>>3)
		);
}

#endif

/*
 * Python
 * def get_rgb565(x) :
 *  return ((x&0xf80000)>>8) | ((x&0xfc00)>>5) | ((x&0xf8)>>3)
 *
 * print print "0x%X"%get_rgb565(some_rgb_value)
 */
#define GFX_COLOR_WHITE          (gfx_color_t){.rgb565.c=0xFFFF}
#define GFX_COLOR_BLACK          (gfx_color_t){.rgb565.c=0x0000}
#define GFX_COLOR_DARKGREY       (gfx_color_t){.rgb565.c=0x4228}
#define GFX_COLOR_GREY           (gfx_color_t){.rgb565.c=0xF7DE}
#define GFX_COLOR_GREY2          (gfx_color_t){.rgb565.c=0x7BEF} /*1111 0111 1101 1110*/
#define GFX_COLOR_BLUE           (gfx_color_t){.rgb565.c=0x001F}
#define GFX_COLOR_BLUE2          (gfx_color_t){.rgb565.c=0x051F}
#define GFX_COLOR_RED            (gfx_color_t){.rgb565.c=0xF800}
#define GFX_COLOR_MAGENTA        (gfx_color_t){.rgb565.c=0xF81F}
#define GFX_COLOR_GREEN          (gfx_color_t){.rgb565.c=0x07E0}
#define GFX_COLOR_GREEN2         (gfx_color_t){.rgb565.c=0xB723}
#define GFX_COLOR_CYAN           (gfx_color_t){.rgb565.c=0x7FFF}
#define GFX_COLOR_YELLOW         (gfx_color_t){.rgb565.c=0xFFE0}
#define GFX_COLOR_ORANGE         (gfx_color_t){.rgb565.c=0xFBE4}
#define GFX_COLOR_BROWN          (gfx_color_t){.rgb565.c=0xBBCA}

#endif

void GFX_FCT(init)(void *surface, int32_t w, int32_t h);
gfx_state_t *GFX_FCT(get_state)(void);
/**
 * set pixel buffer address to render graphics on..
 * @param surface pixel buffer address
 */
void GFX_FCT(set_surface)(void *surface);
void *GFX_FCT(get_surface)(void);


void GFX_FCT(offscreen_rendering_begin)(
		void *surface,
		int32_t width, int32_t height
	);
void GFX_FCT(offscreen_rendering_end)(void);

void
GFX_FCT(set_clipping_area_max)(void);
void
GFX_FCT(set_clipping_area)(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
visible_area_t
GFX_FCT(get_surface_visible_area)(void);


void GFX_FCT(fill_screen)(gfx_color_t color);

void GFX_FCT(set_rotation)(gfx_rotation_t rotation);
uint16_t GFX_FCT(width)(void);
uint16_t GFX_FCT(height)(void);
gfx_rotation_t GFX_FCT(get_rotation)(void);

void GFX_FCT(draw_pixel)(int16_t rgb888, int16_t y, gfx_color_t color);
gfx_color_t GFX_FCT(get_pixel)(int16_t rgb888, int16_t y);
fill_segment_queue_statistics_t
GFX_FCT(flood_fill4)(
		int16_t rgb888, int16_t y,
		gfx_color_t old_color, gfx_color_t new_color,
		uint8_t fill_segment_buf[], size_t fill_segment_buf_size
	);

void GFX_FCT(draw_line)(
		int16_t x0, int16_t y0, int16_t x1, int16_t y1,
		gfx_color_t color
	);
void GFX_FCT(draw_hline)(int16_t rgb888, int16_t y, int16_t length, gfx_color_t color);
void GFX_FCT(draw_vline)(int16_t rgb888, int16_t y, int16_t length, gfx_color_t color);
void GFX_FCT(draw_rect)(int16_t rgb888, int16_t y, int16_t w, int16_t h, gfx_color_t color);
void GFX_FCT(fill_rect)(int16_t rgb888, int16_t y, int16_t w, int16_t h, gfx_color_t color);
void GFX_FCT(draw_raw_rbg565_buffer)(
		int16_t rgb888, int16_t y, uint16_t w, uint16_t h,
		const uint16_t *img
	);
void GFX_FCT(draw_raw_rbg565_buffer_ignore_color)(
		int16_t rgb888, int16_t y, uint16_t w, uint16_t h,
		const uint16_t *img,
		uint16_t ignored_color
	);

void GFX_FCT(draw_circle)(int16_t x0, int16_t y0, int16_t r, gfx_color_t color);
void GFX_FCT(draw_circle_helper)(
		int16_t x0, int16_t y0,
		int16_t r, uint8_t cornername,
		gfx_color_t color
	);
void GFX_FCT(fill_circle)(int16_t x0, int16_t y0, int16_t r, gfx_color_t color);
void GFX_FCT(fill_circle_helper)(
		int16_t x0, int16_t y0,
		int16_t r, uint8_t cornername, int16_t delta,
		gfx_color_t color
	);

void GFX_FCT(draw_round_rect)(
		int16_t x0, int16_t y0, int16_t w, int16_t h,
		int16_t radius,
		gfx_color_t color
	);
void GFX_FCT(fill_round_rect)(
		int16_t x0, int16_t y0, int16_t w, int16_t h,
		int16_t radius,
		gfx_color_t color
	);

void GFX_FCT(draw_triangle)(
		int16_t x0, int16_t y0,
		int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		gfx_color_t color
	);
void GFX_FCT(fill_triangle)(
		int16_t x0, int16_t y0,
		int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		gfx_color_t color
	);


void GFX_FCT(draw_char)(
		int16_t rgb888, int16_t y,
		uint32_t c,
		gfx_color_t color,
		uint8_t size
	);
void GFX_FCT(set_font)(const font_t *font);
void GFX_FCT(set_font_color)(gfx_color_t col);
#if !GFX_DMA2D_FONTS
void GFX_FCT(set_font_scale)(uint8_t s);
uint8_t GFX_FCT(get_font_scale)(void);
#endif
void GFX_FCT(set_cursor)(int16_t rgb888, int16_t y);
int16_t GFX_FCT(get_cursor_x)(void);
int16_t GFX_FCT(get_cursor_y)(void);
void GFX_FCT(set_text_wrap)(bool w);
void GFX_FCT(puts)(const char *);
void GFX_FCT(puts4)(const char *s, uint32_t max_len);
void GFX_FCT(puts2)(
		int16_t rgb888, int16_t y,
		const char *s,
		const font_t *font,
		gfx_color_t col
	);
void GFX_FCT(puts3)(
		int16_t rgb888, int16_t y,
		const char *s,
		const gfx_alignment_t alignment
	);
void GFX_FCT(write)(const uint32_t);

const font_t *
GFX_FCT(get_font)(void);
gfx_color_t
GFX_FCT(get_font_color)(void);
uint8_t
GFX_FCT(get_text_wrap)(void);

uint16_t
GFX_FCT(get_char_width)(void);
uint16_t
GFX_FCT(get_line_height)(void);
uint16_t
GFX_FCT(get_string_width)(const char *);
uint16_t
GFX_FCT(get_string_height)(const char *s);
uint16_t
GFX_FCT(get_string_height2)(const char *s, int16_t max_width);

#ifndef GFX_OBJECT_H
#undef PASTER
#undef EVALUATOR
#undef GFX_FCT
#undef GFXV
#undef GFX_COLOR_SIZE
#undef swap_i16
#endif
