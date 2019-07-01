/*
 * gfx_object.h
 *
 *  Created on: 6 Aug 2017
 *      Author: Oliver Meier
 */

#define GFX_OBJECT_H

#include "stdio.h"

#include "gfx_header.h"

#include <assert.h>

//#define PASTER(x,y) x ## y
//#define EVALUATOR(x,y)  PASTER(x,y)
//#define GFX_FCT(fun) EVALUATOR(GFXV, fun)

#define xstr(s) str(s)
#define str(s) #s

#if GFX_COLOR_SIZE==24
	typedef struct {
		uint32_t px12;
		uint32_t px23;
		uint32_t px34;
	} color4_t;
#endif


static gfx_state_t __gfx_state = {0};

#if GFX_DMA2D_FONTS
#include <libopencm3/stm32/ltdc.h>
#endif

/*
 * Make sure the surface is aligned for 32bit!
 */
void
GFX_FCT(init)(void *surface, int32_t width, int32_t height)
{
	__gfx_state.is_offscreen_rendering = 0;

	__gfx_state.width_orig  = width;
	__gfx_state.height_orig = height;
	__gfx_state.pixel_count = (uint32_t)width*(uint32_t)height;
	__gfx_state.width        = width;
	__gfx_state.height       = height;
	//	__gfx_state.surface      = surface;
	GFX_FCT(set_surface)(surface);

	__gfx_state.visible_area = (visible_area_t){0, 0, width, height};
	__gfx_state.rotation     = 0;
	__gfx_state.cursor_x     =
	__gfx_state.cursor_x_orig =
	__gfx_state.cursor_y     = 0;
	__gfx_state.font_scale   = 1;
	__gfx_state.font_color   = (gfx_color_t){.raw=0};
	__gfx_state.text_wrap    = true;
	__gfx_state.font         = NULL;
#if GFX_DMA2D_FONTS
	// font_pxbuf buffer and size have to be set before each char is drawn!
	__gfx_state.font_pxbuf.buffer = NULL;
	__gfx_state.font_pxbuf.width  =
	__gfx_state.font_pxbuf.height = 0;
	__gfx_state.font_pxbuf.in.pixel.bitsize = 4;
	__gfx_state.font_pxbuf.in.pixel.format  = DMA2D_xPFCCR_CM_A4;
	__gfx_state.font_pxbuf.in.pixel.alpha_mode.color = __gfx_state.font_color.raw;
	__gfx_state.font_pxbuf.out.pixel.bytesize = // out is not supported!
	__gfx_state.font_pxbuf.out.pixel.format = 0;
#endif
}

static gfx_state_t __gfx_state_bkp = {0};

gfx_state_t *GFX_FCT(get_state)() {
	if (__gfx_state.is_offscreen_rendering) {
		return &__gfx_state_bkp;
	} else {
		return &__gfx_state;
	}
}

void GFX_FCT(set_surface)(void *surface)
{
	GFX_FCT(get_state)()->surface = surface;
#if GFX_DMA2D_FONTS
//	if (ltdc_get_fbuffer_address(DISPLAY_LAYER_1) == (uint32_t)surface) {
//		dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &__gfx_state.font_pxbuf);
//	} else
//	if (ltdc_get_fbuffer_address(DISPLAY_LAYER_2) == (uint32_t)surface) {
//		dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &__gfx_state.font_pxbuf);
//	} else {
//		/* TODO change everything! */
		__gfx_state.surface_pxbuf.width  = __gfx_state.width;
		__gfx_state.surface_pxbuf.height = __gfx_state.height;
		__gfx_state.surface_pxbuf.buffer = surface;
		__gfx_state.surface_pxbuf.in.alpha_mod.mode  = 0;
		__gfx_state.surface_pxbuf.in.alpha_mod.alpha = 0xff;
#if   GFX_COLOR_MODE==GFX_COLOR_MODE_ARGB8888
		__gfx_state.surface_pxbuf.in.pixel.bitsize   = 32;
		__gfx_state.surface_pxbuf.in.pixel.format    = DMA2D_xPFCCR_CM_ARGB8888;
		__gfx_state.surface_pxbuf.out.pixel.bytesize = 4;
		__gfx_state.surface_pxbuf.out.pixel.format   = DMA2D_OPFCCR_CM_ARGB8888;
#elif GFX_COLOR_MODE==GFX_COLOR_MODE_RGB888
		__gfx_state.surface_pxbuf.in.pixel.bitsize   = 24;
		__gfx_state.surface_pxbuf.in.pixel.format    = DMA2D_xPFCCR_CM_RGB888;
		__gfx_state.surface_pxbuf.out.pixel.bytesize = 3;
		__gfx_state.surface_pxbuf.out.pixel.format   = DMA2D_OPFCCR_CM_RGB888;
#elif GFX_COLOR_MODE==GFX_COLOR_MODE_RGB565
		__gfx_state.surface_pxbuf.in.pixel.bitsize   = 16;
		__gfx_state.surface_pxbuf.in.pixel.format    = DMA2D_xPFCCR_CM_RGB565;
		__gfx_state.surface_pxbuf.out.pixel.bytesize = 2;
		__gfx_state.surface_pxbuf.out.pixel.format   = DMA2D_OPFCCR_CM_RGB565;
#elif GFX_COLOR_MODE==GFX_COLOR_MODE_MONOCHROME
#error "Monochrome color is not supported!"
#endif

//	}
#endif
//	if (__gfx_state.is_offscreen_rendering) {
//		__gfx_state_bkp.surface = surface;
//	} else {
//		__gfx_state.surface = surface;
//	}
}
void *
GFX_FCT(get_surface)()
{
	return GFX_FCT(get_state)()->surface;
//	if (__gfx_state.is_offscreen_rendering) {
//		return __gfx_state_bkp.surface;
//	} else {
//		return __gfx_state.surface;
//	}
}

void GFX_FCT(offscreen_rendering_begin)(
		void *surface,
		int32_t width,
		int32_t height
) {
	if (!__gfx_state.is_offscreen_rendering) {
		/*gfx_state_bkp = __gfx_state; ???*/
		memcpy(&__gfx_state_bkp, &__gfx_state, sizeof(gfx_state_t));
	}
	GFX_FCT(init)(surface, width, height);
	__gfx_state.is_offscreen_rendering = 1;
}
void GFX_FCT(offscreen_rendering_end)()
{
	if (__gfx_state.is_offscreen_rendering) {
		memcpy(&__gfx_state, &__gfx_state_bkp, sizeof(gfx_state_t));
	}
}

void
GFX_FCT(set_clipping_area_max)()
{
	__gfx_state.visible_area =
			(visible_area_t){
				0, 0, __gfx_state.width, __gfx_state.height
			};
}
void
GFX_FCT(set_clipping_area)(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	if (x1 < 0) {
		x1 = 0;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (x2 < x1) {
		x2 = x1;
	}
	if (y2 < y1) {
		y2 = y1;
	}
	__gfx_state.visible_area = (visible_area_t){x1, y1, x2, y2};
}
visible_area_t
GFX_FCT(get_surface_visible_area)() {
	return __gfx_state.visible_area;
}


/*
 * ARGB8888
 * just write a ARGB8888 color to each pixel in the current buffer/surface
 * this function assumes 32bit aligned buffer!
 *
 * RGB888
 * desc..
 *
 * RGB565
 * just write a RGB565 color to each pixel in the current buffer/surface
 * this function assumes 32bit aligned buffer!
 *
 * MONO
 * desc..
 */
void GFX_FCT(fill_screen)(gfx_color_t color)
{
	// TODO use DMA2D
#if   GFX_COLOR_SIZE==32
	uint32_t i; uint32_t *pixel_addr;
	pixel_addr = (uint32_t *)__gfx_state.surface;
	for (i = 0; i < __gfx_state.pixel_count; i++) {
		*pixel_addr++ = color.argb8888.c;
	}

#elif GFX_COLOR_SIZE==24
	color4_t color4 = {
		color.rgb888.c | ((color.rgb888.c & 0xff) << 24),
		((color.rgb888.c >> 8) & 0xffff) | ((color.rgb888.c & 0xffff) << 16),
		((color.rgb888.c >> 16) & 0xff) | ((color.rgb888.c) << 8)
	};

	uint32_t i; color4_t *pixel_addr;
	pixel_addr = (color4_t *)__gfx_state.surface;
	for (i = 0; i < __gfx_state.pixel_count/4; i++) {
		*pixel_addr++ = color4;
	}
	/* copy the rest.. */
	memcpy(pixel_addr, &color4, (__gfx_state.pixel_count & 3) * 3);

#elif GFX_COLOR_SIZE==16
	uint32_t color2 = (color.rgb565.c<<16)|color.rgb565.c;

	uint32_t i; uint32_t *pixel_addr;
	pixel_addr = (uint32_t *)__gfx_state.surface;
	for (i = 0; i < __gfx_state.pixel_count/2; i++) {
		*pixel_addr++ = color2;
	}
	if (__gfx_state.pixel_count & 1) {
		*((uint16_t *)__gfx_state.surface+(__gfx_state.pixel_count-1)) = color.rgb565.c;
	}

	/*
	uint32_t i; void *pixel_addr;
	pixel_addr = __gfx_state.surface;
	for (i = 0; i < __gfx_state.pixel_count&1; i++) {
		*pixel_addr++ = color;
	}
	*/
#elif GFX_COLOR_SIZE==1
//#warning "if the buffer is not aligned to 32bits this will lead to troubles!"
	assert(!((uint32_t)__gfx_state.surface%4));
	memset(__gfx_state.surface, color.mono ? 0xff:0, 1+(__gfx_state.width*__gfx_state.height-1)/32);
#endif

}



/* change the rotation and flip width and height accordingly */
void GFX_FCT(set_rotation)(gfx_rotation_t rotation)
{
	if ((rotation == GFX_ROTATION_0_DEGREES)
	 || (rotation == GFX_ROTATION_180_DEGREES)
	) {
		__gfx_state.width  = __gfx_state.width_orig;
		__gfx_state.height = __gfx_state.height_orig;
	} else {
		__gfx_state.width  = __gfx_state.height_orig;
		__gfx_state.height = __gfx_state.width_orig;
	}
	__gfx_state.rotation = rotation;
	GFX_FCT(set_clipping_area_max)();
}
gfx_rotation_t GFX_FCT(get_rotation)(void)
{
	return __gfx_state.rotation;
}
/* Return the size of the display (per current rotation) */
uint16_t GFX_FCT(width)(void)
{
	return __gfx_state.width;
}

uint16_t GFX_FCT(height)(void)
{
	return __gfx_state.height;
}


/* GFX_GLOBAL_DISPLAY_SCALE can be  used as a zoom function
 * while debugging code..
 *
 * NOTE: This currently breaks offscreen rendering!!
 */
#define GFX_GLOBAL_DISPLAY_SCALE 1

typedef struct {
#if   GFX_COLOR_SIZE == 32
	uint32_t *p;
#elif GFX_COLOR_SIZE == 24
	uint8_t  *p;
//	uint32_t  b; /* byte offset */
#elif GFX_COLOR_SIZE == 16
	uint16_t *p;
#elif GFX_COLOR_SIZE == 1
	uint32_t *p;
	uint32_t  b;
#endif
} gfx_pixeladdress_t;

static inline uint32_t GFX_FCT(get_pixel_offset)(int16_t x, int16_t y)
{
	switch (__gfx_state.rotation) {
	case GFX_ROTATION_0_DEGREES:
		return                           (x + y * (__gfx_state.width_orig));
	case GFX_ROTATION_270_DEGREES:
		return                           (x * __gfx_state.width_orig + __gfx_state.width_orig - y)         - GFX_GLOBAL_DISPLAY_SCALE;
	case GFX_ROTATION_180_DEGREES:
		return __gfx_state.pixel_count - (x + (y + (GFX_GLOBAL_DISPLAY_SCALE-1)) * __gfx_state.width_orig) - GFX_GLOBAL_DISPLAY_SCALE;
	case GFX_ROTATION_90_DEGREES:
		return __gfx_state.pixel_count - ((x + (GFX_GLOBAL_DISPLAY_SCALE-1))     * __gfx_state.width_orig + __gfx_state.width_orig - y);
	default :
		assert(0);
		return 0;
	}
}

static inline gfx_pixeladdress_t GFX_FCT(get_pixel_address)(int16_t x, int16_t y)
{
#if   GFX_COLOR_SIZE == 32
	return (gfx_pixeladdress_t){.p=((uint32_t *)__gfx_state.surface)+GFX_FCT(get_pixel_offset)(x,y)};
#elif GFX_COLOR_SIZE == 24
	return (gfx_pixeladdress_t){.p=((uint8_t *) __gfx_state.surface)+3*GFX_FCT(get_pixel_offset)(x,y)};
#elif GFX_COLOR_SIZE == 16
	return (gfx_pixeladdress_t){.p=((uint16_t *)__gfx_state.surface)+GFX_FCT(get_pixel_offset)(x,y)};
#elif GFX_COLOR_SIZE == 1
	uint32_t offset = GFX_FCT(get_pixel_offset)(x,y);
	gfx_pixeladdress_t addr;
	addr.p = ((uint32_t *)__gfx_state.surface) + offset/32;
	addr.b =                                     offset%32;
	return addr;
#endif
}


#if GFX_GLOBAL_DISPLAY_SCALE > 1
#define GFX_GLOBAL_DISPLAY_SCALE_OFFSET_X -10
#define GFX_GLOBAL_DISPLAY_SCALE_OFFSET_Y -40

#if (GFX_COLOR_SIZE == 1) || (GFX_COLOR_SIZE == 24) || (GFX_COLOR_SIZE == 32)
#error "not implemented!"
#endif

/*
 * draw a single pixel
 * changes buffer addressing (surface) according to orientation
 */
void GFX_FCT(draw_pixel)(int16_t x, int16_t y, gfx_color_t color)
{
	x += GFX_GLOBAL_DISPLAY_SCALE_OFFSET_X;
	y += GFX_GLOBAL_DISPLAY_SCALE_OFFSET_Y;
	x *= GFX_GLOBAL_DISPLAY_SCALE;
	y *= GFX_GLOBAL_DISPLAY_SCALE;

	if (
		x <  __gfx_state.visible_area.x1
	 || x >= __gfx_state.visible_area.x2 - GFX_GLOBAL_DISPLAY_SCALE+1
	 || y <  __gfx_state.visible_area.y1
	 || y >= __gfx_state.visible_area.y2 - GFX_GLOBAL_DISPLAY_SCALE+1
	) {
		return;
	}

	void *pa = GFX_FCT(get_pixel_address)(x, y);
	for (uint32_t sa = 0; sa < GFX_GLOBAL_DISPLAY_SCALE; sa++) {
		for (uint32_t sb = 0; sb < GFX_GLOBAL_DISPLAY_SCALE; sb++) {
			*(pa+sb) = color;
		}
		pa += __gfx_state.width_orig;
	}

	return;
}

/*
 * get a single pixel
 */
int32_t GFX_FCT(get_pixel)(int16_t x, int16_t y)
{
	x += GFX_GLOBAL_DISPLAY_SCALE_OFFSET_X;
	y += GFX_GLOBAL_DISPLAY_SCALE_OFFSET_Y;
	x *= GFX_GLOBAL_DISPLAY_SCALE;
	y *= GFX_GLOBAL_DISPLAY_SCALE;

	if (
		x <  __gfx_state.visible_area.x1
	 || x >= __gfx_state.visible_area.x2 - GFX_GLOBAL_DISPLAY_SCALE+1
	 || y <  __gfx_state.visible_area.y1
	 || y >= __gfx_state.visible_area.y2 - GFX_GLOBAL_DISPLAY_SCALE+1
	) {
		return -1;
	}

	return *GFX_FCT(get_pixel_address)(x, y);
}

#else

/*
 * draw a single pixel
 * changes buffer addressing (surface) according to orientation
 */
void GFX_FCT(draw_pixel)(int16_t x, int16_t y, gfx_color_t color)
{
	if (
		x <  __gfx_state.visible_area.x1
	 || x >= __gfx_state.visible_area.x2
	 || y <  __gfx_state.visible_area.y1
	 || y >= __gfx_state.visible_area.y2
	) {
		return;
	}

#if   GFX_COLOR_SIZE == 32
	*GFX_FCT(get_pixel_address)(x, y).p = color.argb8888.c;
#elif GFX_COLOR_SIZE == 24
	memcpy(GFX_FCT(get_pixel_address)(x, y).p, &color.rgb888.c, 3);
#elif GFX_COLOR_SIZE == 16
	*GFX_FCT(get_pixel_address)(x, y).p = color.rgb565.c;
#elif GFX_COLOR_SIZE == 1
	gfx_pixeladdress_t addr = GFX_FCT(get_pixel_address)(x, y);
	if (color.mono) {
		*addr.p |=  (1<<addr.b);
	} else {
		*addr.p &= ~(1<<addr.b);
	}
#endif
	return;
}

/*
 * get a single pixel
 */
gfx_color_t GFX_FCT(get_pixel)(int16_t x, int16_t y)
{
	if (
		x <  __gfx_state.visible_area.x1
	 || x >= __gfx_state.visible_area.x2
	 || y <  __gfx_state.visible_area.y1
	 || y >= __gfx_state.visible_area.y2
	) {
		return (gfx_color_t){.raw=0};
	}

#if   GFX_COLOR_SIZE == 32
	return (gfx_color_t){.argb8888.c=*GFX_FCT(get_pixel_address)(x, y).p};
#elif GFX_COLOR_SIZE == 24
	gfx_color_t ret;
	memcpy(&ret, GFX_FCT(get_pixel_address)(x, y).p, 3);
	return ret;
#elif GFX_COLOR_SIZE == 16
	return (gfx_color_t){.rgb565.c=*GFX_FCT(get_pixel_address)(x, y).p};
#elif GFX_COLOR_SIZE == 1
	gfx_pixeladdress_t addr = GFX_FCT(get_pixel_address)(x, y);
	return (gfx_color_t){.mono=*addr.p & ~(1<<addr.b)};
#endif
}
#endif

typedef struct {
	int16_t ud_dir;
	int16_t x0, x1;
	int16_t y; /* actually y-ud_dir.. */
} fill_segment_t;
typedef struct {
	size_t size;
	fill_segment_t *data;
	size_t count;
	fill_segment_queue_statistics_t stats;
} fill_segment_queue_t;

//#define SHOW_FILLING
#ifdef SHOW_FILLING
#include "lcd_ili9341.h"
#include <assert.h>
#include <support/clock.h>
#endif


static inline void GFX_FCT(flood_fill4_add_if_found)(
		fill_segment_queue_t *queue,
		int16_t ud_dir,
		int16_t xa0, int16_t xa1,
		int16_t y,
		gfx_color_t old_color
) {
	y += ud_dir;
	while ((xa0 <= xa1)) {
		if (GFX_FCT(get_pixel)(xa0, y).raw == old_color.raw) {
			if (queue->count < queue->size) {
				queue->data[queue->count++] =
						(fill_segment_t) {
							.ud_dir = ud_dir,
							.x0     = xa0,
							.x1     = xa1,
							.y      = y-ud_dir
						};
			} else {
				queue->stats.overflows++;
			}
			return;
		}
		xa0++;
	}
}

/* warning! this function might be very slow and fail:) */
fill_segment_queue_statistics_t
GFX_FCT(flood_fill4)(
		int16_t x, int16_t y,
		gfx_color_t old_color, gfx_color_t new_color,
		uint8_t fill_segment_buf[], size_t fill_segment_buf_size
) {
	if (old_color.raw == new_color.raw) {
		return (fill_segment_queue_statistics_t){0};
	}

	if (GFX_FCT(get_pixel)(x, y).raw != old_color.raw) {
		return (fill_segment_queue_statistics_t){0};
	}

	fill_segment_queue_t queue = {
		.size = fill_segment_buf_size/sizeof(fill_segment_t),
		.data = (fill_segment_t *)fill_segment_buf,
		.count = 0,
		.stats = (fill_segment_queue_statistics_t) {
				.count_max = 0,
				.count_total = 0,
				.overflows = 0,
		}
	};


	int16_t sx, sy;

	int16_t ud_dir_init = 1;
	int16_t ud_dir = 1;

	int16_t x0, x1, x0l, x1l, x0ll, x1ll;

	bool pixel_drawn = false;

	sx = x;
	sy = y;

	/* process first line */
	x--; /* x is always increased first! */
	while (GFX_FCT(get_pixel)(++x, y).raw == old_color.raw) {
		GFX_FCT(draw_pixel)(x, y, new_color);
		pixel_drawn = true;
	}
	x1l = x1ll = x-1;
	x  = sx;
	while (GFX_FCT(get_pixel)(--x, y).raw == old_color.raw) {
		GFX_FCT(draw_pixel)(x, y, new_color);
		pixel_drawn = true;
	}
	x0l = x0ll = x+1;

#ifdef SHOW_FILLING
	ltdc_set_fbuffer_address(LTDC_LAYER_2, (uint32_t)__gfx_state.surface);
#endif

	while (pixel_drawn) {
		pixel_drawn = false;

#ifdef SHOW_FILLING
		if (!LTDC_SRCR_IS_RELOADING()) {
			ltdc_reload(LTDC_SRCR_RELOAD_VBR);
		}
#endif
		y += ud_dir;

		/* find x start point (must be between x0 and x1 of the last iteration) */
		sx  = x0l;
		while ((sx < x1l) && (GFX_FCT(get_pixel)(sx, y).raw != old_color.raw)) {
			sx++;
		}

		bool sx_was_oc = GFX_FCT(get_pixel)(sx, y).raw == old_color.raw;

		/** middle to right */
		x  = sx-1;
		/* fill additional adjacent old-colored pixels */
		while (GFX_FCT(get_pixel)(++x, y).raw == old_color.raw) {
			GFX_FCT(draw_pixel)(x, y, new_color);
#ifdef SHOW_FILLING
		msleep_loop(1);
#endif
			pixel_drawn = true;
		}
		x1 = x-1;
		/* check if x1 is bigger compared to the last line */
		if (x1 > x1l+1) {
			if (queue.count < queue.size) {
				queue.data[queue.count++] =
						(fill_segment_t) {
							.ud_dir = -ud_dir,
							.x0     = x1l,
							.x1     = x1,
							.y      = y
						};
			} else {
				queue.stats.overflows++;
			}
		} else {
			/* fill all lastline-adjacent old-colored pixels */
			bool adjacent_pixel_drawn = false;
			int16_t xa0=0, xa1;
			while ((x < x1l) || adjacent_pixel_drawn) {
				if (GFX_FCT(get_pixel)(++x, y).raw == old_color.raw) {
					GFX_FCT(draw_pixel)(x, y, new_color);
#ifdef SHOW_FILLING
		msleep_loop(1);
#endif
					if (!adjacent_pixel_drawn) {
						adjacent_pixel_drawn = true;
						xa0 = x;
					}
				} else
				if (adjacent_pixel_drawn) {
					adjacent_pixel_drawn = false;
					xa1 = x-1;

					GFX_FCT(flood_fill4_add_if_found)(&queue, ud_dir, xa0, xa1, y, old_color);
					if (xa1 > x1l+1) {
						GFX_FCT(flood_fill4_add_if_found)(&queue, -ud_dir, x1l+1, xa1, y, old_color);
					}
				}
			}
		}
		x1l = x1;


		/** middle-1 to left */
		x  = sx;
		if ((sx > x0l) || sx_was_oc) {
			/* fill additional adjacent old-colored pixels */
			while (GFX_FCT(get_pixel)(--x, y).raw == old_color.raw) {
				GFX_FCT(draw_pixel)(x, y, new_color);
#ifdef SHOW_FILLING
		msleep_loop(1);
#endif
				pixel_drawn = true;
			}
			x0 = x+1;
		} else {
			x0 = sx;
			x--;
		}
		/* check if x0 is smaller compared to the last line */
		if (x0 < x0l-1) {
			if (queue.count < queue.size) {
				queue.data[queue.count++] =
						(fill_segment_t) {
							.ud_dir = -ud_dir,
							.x0     = x0,
							.x1     = x0l,
							.y      = y
						};
			} else {
				queue.stats.overflows++;
			}
		} else {
			/* fill all lastline-adjacent old-colored pixels */
			bool adjacent_pixel_drawn = false;
			int16_t xa0, xa1 = 0;
			while ((x > x0l) || adjacent_pixel_drawn) {
				if (GFX_FCT(get_pixel)(--x, y).raw == old_color.raw) {
					GFX_FCT(draw_pixel)(x, y, new_color);
#ifdef SHOW_FILLING
		msleep_loop(1);
#endif
					if (!adjacent_pixel_drawn) {
						adjacent_pixel_drawn = true;
						xa1 = x;
					}
				} else
				if (adjacent_pixel_drawn) {
					adjacent_pixel_drawn = false;
					xa0 = x+1;

					GFX_FCT(flood_fill4_add_if_found)(&queue, ud_dir, xa0, xa1, y, old_color);
					if (xa0 < x0l-1) {
						GFX_FCT(flood_fill4_add_if_found)(&queue, -ud_dir, xa0, x0l-1, y, old_color);
					}
				}
			}
		}
		x0l = x0;

		/** no pixel draw, check if we're already filling upwards */
		if (!pixel_drawn) {
			if (ud_dir == ud_dir_init) {
				pixel_drawn = true;
				ud_dir = -1;
				y = sy;
				x0l = x0ll;
				x1l = x1ll;
			} else {
				ud_dir_init = 2; /* filling saved segments */

				queue.stats.count_total++;
				if (queue.stats.count_max < queue.count) {
					queue.stats.count_max = queue.count;
				}

				if (queue.count) {
					queue.count--;
					ud_dir = queue.data[queue.count].ud_dir;
					x0l = queue.data[queue.count].x0;
					x1l = queue.data[queue.count].x1;
					/* sx  = (x0l+x1l)/2; */
					y   = queue.data[queue.count].y;
					pixel_drawn = true;

#ifdef SHOW_FILLING
					assert(x0l <= x1l);
#endif
				}
			}
		}
	}

#ifdef SHOW_FILLING
	msleep_loop(5000);
#endif

	return queue.stats;
}


/* Bresenham's algorithm - thx wikpedia */
void GFX_FCT(draw_line)(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    gfx_color_t fg) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap_i16(x0, y0);
		swap_i16(x1, y1);
	}

	if (x0 > x1) {
		swap_i16(x0, x1);
		swap_i16(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			GFX_FCT(draw_pixel)(y0, x0, fg);
		} else {
			GFX_FCT(draw_pixel)(x0, y0, fg);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}
void GFX_FCT(draw_hline)(int16_t x, int16_t y, int16_t length, gfx_color_t color)
{
	if (length < 0) {
		length = -length;
		x     -=  length;
	}
	while (length--) {
		GFX_FCT(draw_pixel)(x++, y, color);
	}
}
void GFX_FCT(draw_vline)(int16_t x, int16_t y, int16_t length, gfx_color_t color)
{
	if (length < 0) {
		length = -length;
		y     -=  length;
	}
	while (length--) {
		GFX_FCT(draw_pixel)(x, y++, color);
	}
}

void GFX_FCT(draw_rect)(int16_t x, int16_t y, int16_t w, int16_t h, gfx_color_t color)
{
	int16_t x_s, y_e;
	if (w < 0) {
		w = -w;
		x -= w;
	}
	if (h < 0) {
		h = -h;
		x -= h;
	}
	x_s = x;
	y_e = y+h-1;
	while (w-- != 0) {
		GFX_FCT(draw_pixel)(x, y  , color);
		GFX_FCT(draw_pixel)(x, y_e, color);
		x++;
	}
	x--;
	while (h-- != 0) {
		GFX_FCT(draw_pixel)(x_s, y, color);
		GFX_FCT(draw_pixel)(x  , y, color);
		y++;
	}
}

void GFX_FCT(fill_rect)(int16_t x, int16_t y, int16_t w, int16_t h,
			    gfx_color_t fg) {

	int16_t i;
	for (i = x; i < x+w; i++) {
		GFX_FCT(draw_vline)(i, y, h, fg);
	}
}




/* Draw RGB565 data (TODO maybe remove this!) */
static inline
bool rgb565_to_mono(uint16_t c) {
	return 256*3/2 <
		  ((((c) & 0xF800) >> 11) * 2
		 + (((c) & 0x07E0) >>  5) * 1
		 +  ((c) & 0x001F)        * 2);
}

void GFX_FCT(draw_raw_rbg565_buffer)(
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint16_t *img
) {
	// dma2d this!
	while (h--) {
		int16_t x_cp, w_cp;
		x_cp = x;
		w_cp = w;
		while (w_cp--) {
#if   GFX_COLOR_SIZE == 32
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.argb8888.c=0xff|(rgb888_from_rgb565(*img++)<<8)});
#elif GFX_COLOR_SIZE == 24
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.rgb888.c=rgb888_from_rgb565(*img++)});
#elif GFX_COLOR_SIZE == 16
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.rgb565.c=*img++});
#elif GFX_COLOR_SIZE == 1
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.mono=rgb565_to_mono(*img++)});
#endif
			x_cp++;
		}
		y++;
	}
}

/* Draw RGB565 data, do not draw ignored_color */
void GFX_FCT(draw_raw_rbg565_buffer_ignore_color)(
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint16_t *img,
		uint16_t ignored_color
) {
	// dma2d this!
	while (h--) {
		int16_t x_cp, w_cp;
		x_cp = x;
		w_cp = w;
		while (w_cp--) {
			uint16_t c = *img++;
			if (c != ignored_color) {
#if   GFX_COLOR_SIZE == 32
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.argb8888.c=0xff|(rgb888_from_rgb565(c)<<8)});
#elif GFX_COLOR_SIZE == 24
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.rgb888.c=rgb888_from_rgb565(c)});
#elif GFX_COLOR_SIZE == 16
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.rgb565.c=c});
#elif GFX_COLOR_SIZE == 1
				GFX_FCT(draw_pixel)(x_cp, y, (gfx_color_t){.mono=rgb565_to_mono(c)});
#endif
			}
			x_cp++;
		}
		y++;
	}
}

/* Draw a circle outline */
void GFX_FCT(draw_circle)(int16_t x0, int16_t y0, int16_t r, gfx_color_t fg)
{
	int16_t f = 1 - r;
	int16_t dd_f_x = 1;
	int16_t dd_f_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	GFX_FCT(draw_pixel)(x0  , y0+r, fg);
	GFX_FCT(draw_pixel)(x0  , y0-r, fg);
	GFX_FCT(draw_pixel)(x0+r, y0  , fg);
	GFX_FCT(draw_pixel)(x0-r, y0  , fg);

	while (x < y) {
		if (f >= 0) {
			y--;
			dd_f_y += 2;
			f += dd_f_y;
		}
		x++;
		dd_f_x += 2;
		f += dd_f_x;

		GFX_FCT(draw_pixel)(x0 + x, y0 + y, fg);
		GFX_FCT(draw_pixel)(x0 - x, y0 + y, fg);
		GFX_FCT(draw_pixel)(x0 + x, y0 - y, fg);
		GFX_FCT(draw_pixel)(x0 - x, y0 - y, fg);
		GFX_FCT(draw_pixel)(x0 + y, y0 + x, fg);
		GFX_FCT(draw_pixel)(x0 - y, y0 + x, fg);
		GFX_FCT(draw_pixel)(x0 + y, y0 - x, fg);
		GFX_FCT(draw_pixel)(x0 - y, y0 - x, fg);
	}
}

void GFX_FCT(draw_circle_helper)(
		int16_t x0, int16_t y0,
		int16_t r, uint8_t cornername,
		gfx_color_t fg
) {
	int16_t f     = 1 - r;
	int16_t dd_f_x = 1;
	int16_t dd_f_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			dd_f_y += 2;
			f     += dd_f_y;
		}
		x++;
		dd_f_x += 2;
		f     += dd_f_x;
		if (cornername & 0x4) {
			GFX_FCT(draw_pixel)(x0 + x, y0 + y, fg);
			GFX_FCT(draw_pixel)(x0 + y, y0 + x, fg);
		}
		if (cornername & 0x2) {
			GFX_FCT(draw_pixel)(x0 + x, y0 - y, fg);
			GFX_FCT(draw_pixel)(x0 + y, y0 - x, fg);
		}
		if (cornername & 0x8) {
			GFX_FCT(draw_pixel)(x0 - y, y0 + x, fg);
			GFX_FCT(draw_pixel)(x0 - x, y0 + y, fg);
		}
		if (cornername & 0x1) {
			GFX_FCT(draw_pixel)(x0 - y, y0 - x, fg);
			GFX_FCT(draw_pixel)(x0 - x, y0 - y, fg);
		}
	}
}

void GFX_FCT(fill_circle)(
		int16_t x0, int16_t y0,
		int16_t r,
		gfx_color_t fg
) {
	GFX_FCT(draw_vline)(x0, y0-r, 2*r+1, fg);
	GFX_FCT(fill_circle_helper)(x0, y0, r, 3, 0, fg);
}

/* Used to do circles and roundrects */
void GFX_FCT(fill_circle_helper)(
		int16_t x0, int16_t y0,
		int16_t r, uint8_t cornername, int16_t delta,
		gfx_color_t fg
) {
	int16_t f     = 1 - r;
	int16_t dd_f_x = 1;
	int16_t dd_f_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			dd_f_y += 2;
			f     += dd_f_y;
		}
		x++;
		dd_f_x += 2;
		f     += dd_f_x;

		if (cornername & 0x1) {
			GFX_FCT(draw_vline)(x0+x, y0-y, 2*y+1+delta, fg);
			GFX_FCT(draw_vline)(x0+y, y0-x, 2*x+1+delta, fg);
		}
		if (cornername & 0x2) {
			GFX_FCT(draw_vline)(x0-x, y0-y, 2*y+1+delta, fg);
			GFX_FCT(draw_vline)(x0-y, y0-x, 2*x+1+delta, fg);
		}
	}
}


/* Draw a rounded rectangle */
void GFX_FCT(draw_round_rect)(
		int16_t x, int16_t y, int16_t w, int16_t h,
		int16_t r,
		gfx_color_t fg
) {
	/* smarter version */
	GFX_FCT(draw_hline)(x+r  , y    , w-2*r, fg); /* Top */
	GFX_FCT(draw_hline)(x+r  , y+h-1, w-2*r, fg); /* Bottom */
	GFX_FCT(draw_vline)(x    , y+r  , h-2*r, fg); /* Left */
	GFX_FCT(draw_vline)(x+w-1, y+r  , h-2*r, fg); /* Right */
	/* draw four corners */
	GFX_FCT(draw_circle_helper)(x+r    , y+r    , r, 1, fg);
	GFX_FCT(draw_circle_helper)(x+w-r-1, y+r    , r, 2, fg);
	GFX_FCT(draw_circle_helper)(x+w-r-1, y+h-r-1, r, 4, fg);
	GFX_FCT(draw_circle_helper)(x+r    , y+h-r-1, r, 8, fg);
}

/* Fill a rounded rectangle */
void GFX_FCT(fill_round_rect)(
		int16_t x, int16_t y, int16_t w, int16_t h,
		int16_t r,
		gfx_color_t fg
) {
	/* smarter version */
	GFX_FCT(fill_rect)(x+r, y, w-2*r, h, fg);

	/* draw four corners */
	GFX_FCT(fill_circle_helper)(x+w-r-1, y+r, r, 1, h-2*r-1, fg);
	GFX_FCT(fill_circle_helper)(x+r    , y+r, r, 2, h-2*r-1, fg);
}


/* Draw a triangle */
void GFX_FCT(draw_triangle)(
		int16_t x0, int16_t y0,
		int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		gfx_color_t fg
) {
	GFX_FCT(draw_line)(x0, y0, x1, y1, fg);
	GFX_FCT(draw_line)(x1, y1, x2, y2, fg);
	GFX_FCT(draw_line)(x2, y2, x0, y0, fg);
}

/* Fill a triangle */
void GFX_FCT(fill_triangle)(
		int16_t x0, int16_t y0,
		int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		gfx_color_t fg
) {
	int16_t a, b, y, last;

	/* Sort coordinates by Y order (y2 >= y1 >= y0) */
	if (y0 > y1) {
		swap_i16(y0, y1); swap_i16(x0, x1);
	}
	if (y1 > y2) {
		swap_i16(y2, y1); swap_i16(x2, x1);
	}
	if (y0 > y1) {
		swap_i16(y0, y1); swap_i16(x0, x1);
	}

	/* Handle awkward all-on-same-line case as its own thing */
	if (y0 == y2) {
		a = b = x0;
		if (x1 < a) {
			a = x1;
		} else
		if (x1 > b) {
			b = x1;
		}
		if (x2 < a) {
			a = x2;
		} else
		if (x2 > b) {
			b = x2;
		}
		GFX_FCT(draw_hline)(a, y0, b-a+1, fg);
		return;
	}

	int16_t
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1,
	sa   = 0,
	sb   = 0;

	/* For upper part of triangle, find scanline crossings for segments
	 * 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	 * is included here (and second loop will be skipped, avoiding a /0
	 * error there), otherwise scanline y1 is skipped here and handled
	 * in the second loop...which also avoids a /0 error here if y0=y1
	 * (flat-topped triangle).
	 */
	if (y1 == y2) {
		last = y1;   /* Include y1 scanline */
	} else {
		last = y1-1; /* Skip it */
	}

	for (y = y0; y <= last; y++) {
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b) {
			swap_i16(a, b);
		}
		GFX_FCT(draw_hline)(a, y, b-a+1, fg);
	}

	/* For lower part of triangle, find scanline crossings for segments
	 * 0-2 and 1-2.  This loop is skipped if y1=y2.
	 */
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++) {
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b) {
			swap_i16(a, b);
		}
		GFX_FCT(draw_hline)(a, y, b-a+1, fg);
	}
}




/**
 * Text functions
 */

/* Write utf8 text */
void GFX_FCT(write)(const uint32_t c)
{
	if (!__gfx_state.font) {
		return;
	}
	if (c == '\n') {
		__gfx_state.cursor_y += GFX_FCT(get_line_height)();
		__gfx_state.cursor_x  = __gfx_state.cursor_x_orig;
	} else if (c == '\r') {
		__gfx_state.cursor_x  = __gfx_state.cursor_x_orig;
	} else {
		if (__gfx_state.text_wrap
		 && (
			__gfx_state.cursor_x
		  > (
				__gfx_state.visible_area.x2
			  - GFX_FCT(get_char_width)()
		))) {
			__gfx_state.cursor_x = __gfx_state.cursor_x_orig;
			__gfx_state.cursor_y += GFX_FCT(get_line_height());
		}
		GFX_FCT(draw_char)(
				__gfx_state.cursor_x, __gfx_state.cursor_y,
				c,
				__gfx_state.font_color, __gfx_state.font_scale
			);
		__gfx_state.cursor_x += GFX_FCT(get_char_width)();
	}
}

void GFX_FCT(puts)(const char *s) {
	while (*s) {
		int32_t value;
		s = utf8_read_value(s, &value);
		GFX_FCT(write)(value);
	}
}
void GFX_FCT(puts4)(const char *s, uint32_t max_len) {
	while (*s && max_len--) {
		int32_t value;
		s = utf8_read_value(s, &value);
		GFX_FCT(write)(value);
	}
}
void GFX_FCT(puts2)(
		int16_t x, int16_t y,
		const char *s,
		const font_t *font,
		gfx_color_t col
) {
	GFX_FCT(set_cursor)(x, y);
	GFX_FCT(set_font)(font);
	GFX_FCT(set_font_color)(col);
	GFX_FCT(puts)(s);
}
/* this is not utf8 right now.. */
void GFX_FCT(puts3)(
		int16_t x, int16_t y,
		const char *s,
		const gfx_alignment_t alignment
) {
	const char *s_end;
	const char *next_nl; //, *last_nl;

	switch (alignment) {
	case GFX_ALIGNMENT_TOP:
	case GFX_ALIGNMENT_BOTTOM:
	case GFX_ALIGNMENT_LEFT:
		GFX_FCT(set_cursor)(x, y);
		GFX_FCT(puts)(s);
		return;

	case GFX_ALIGNMENT_RIGHT:
		s_end = utf8_find_character_in_string(0, s, s+1024);
		next_nl = utf8_find_character_in_string('\n', s, s_end);
		if (next_nl == s_end) {
			GFX_FCT(set_cursor)(
					x
					- utf8_find_pointer_diff(s, s_end)
					* GFX_FCT(get_char_width()),
					y
				);
			GFX_FCT(puts)(s);
		} else {
			uint32_t line_count = 0;
			while (s < s_end) {
				next_nl = utf8_find_character_in_string('\n', s, s_end);
				GFX_FCT(set_cursor)(
						x
						- utf8_find_pointer_diff(s, next_nl)
						* GFX_FCT(get_char_width)(),
						y
						+ line_count
						* GFX_FCT(get_line_height())
					);
				do {
					int32_t value;
					s = utf8_read_value(s, &value);
					GFX_FCT(write)(value);
				} while (s != next_nl);
				s++; line_count++;
			}
		}
		break;

	case GFX_ALIGNMENT_CENTER:
		/* TODO correct rounding on /2 */
		s_end = utf8_find_character_in_string(0, s, s+1024);
		next_nl = utf8_find_character_in_string('\n', s, s_end);
		if (0) { //next_nl == s_end) {
			GFX_FCT(set_cursor)(
					x-(utf8_find_pointer_diff(s, s_end)
					* GFX_FCT(get_char_width)())/2,
				y
			);
			GFX_FCT(puts)(s);
		} else {
//			/* find longest line */
//			uint32_t line_length, longest_line;
//			longest_line = utf8_find_pointer_diff(s, next_nl);
//			last_nl = next_nl+1;
//			while (last_nl < s_end) {
//				next_nl = utf8_find_character_in_string('\n', last_nl, s_end);
//				line_length = utf8_find_pointer_diff(last_nl, next_nl);
//				if (longest_line < line_length) {
//					longest_line = line_length;
//				}
//				last_nl = next_nl+1;
//			}

			/* print lines */
			uint32_t line_count = 0;
			while (s < s_end) {
				next_nl = utf8_find_character_in_string('\n', s, s_end);
				GFX_FCT(set_cursor)(
						x
						-(utf8_find_pointer_diff(s, next_nl)
						* GFX_FCT(get_char_width)())/2,
						//- (longest_line*gfx_get_char_width())
//						- ((longest_line
//							- utf8_find_pointer_diff(s, next_nl)
//						) * gfx_get_char_width())/2,
						y
						+ line_count*GFX_FCT(get_line_height)());
				do {
					int32_t value;
					s = utf8_read_value(s, &value);
					GFX_FCT(write)(value);
				} while (s != next_nl);
				s++; line_count++;
			}
		}
		break;
	}
}

/**
 * This function gets the index of the char for the font data
 * converts it's bit array address to a 32bit and mask
 * and draws a point of size to it's location
 *
 * @param x    x position
 * @param y    y position
 * @param c    utf8 char id
 * @param col  color
 * @param size Size in multiples of 1
 */
void GFX_FCT(draw_char)(
		int16_t x, int16_t y,
		uint32_t c,
		gfx_color_t col,
		uint8_t size
) {
	(void)size;
	if (!__gfx_state.font) {
		return;
	}
	/* get a pointer to the char id from the lookup table */
	const char_t   *cp;
	cp = font_get_char(c, __gfx_state.font);
	if (cp == NULL) {
		return;
	}
	if (cp->data == NULL) {
		return;
	}


#if GFX_DMA2D_FONTS
	__gfx_state.font_pxbuf.buffer = (uint8_t *)cp->data;
	__gfx_state.font_pxbuf.width  = cp->bbox.x2-cp->bbox.x1;
	__gfx_state.font_pxbuf.height = cp->bbox.y2-cp->bbox.y1;
	__gfx_state.font_pxbuf.in.pixel.alpha_mode.color = col.raw;
	if (__gfx_state.font_blending) {
		dma2d_convert_blenddst__no_pxsrc_fix(
				&__gfx_state.font_pxbuf,
				&__gfx_state.surface_pxbuf,
				0,0,
				x+cp->bbox.x1,y+cp->bbox.y1,
				__gfx_state.font_pxbuf.width,
				__gfx_state.font_pxbuf.height);

	} else {
		dma2d_convert_copy__no_pxsrc_fix(
				&__gfx_state.font_pxbuf,
				&__gfx_state.surface_pxbuf,
				0,0,
				x+cp->bbox.x1,y+cp->bbox.y1,
				__gfx_state.font_pxbuf.width,
				__gfx_state.font_pxbuf.height);

	}

#else
	const uint32_t *cp_data_p;
	cp_data_p = cp->data;
	int16_t i, j;
	uint32_t bm;
	bm = 1; /* bit_mask */
	for (j = cp->bbox.y1; j < cp->bbox.y2; j++) {
		for (i = cp->bbox.x1; i < cp->bbox.x2; i++) {
			if (*cp_data_p & bm) {
				/* default size */
				if (size == 1) {
					GFX_FCT(draw_pixel)(x+i, y+j, col);
				} else {
				/* big size */
					GFX_FCT(fill_rect)(x+i*size, y+j*size, size, size, col);
				}
			}
			/* overflow */
			if (bm == 0x80000000) {
				bm = 1;
				cp_data_p++;
			} else {
				bm <<= 1;
			}
		}
	}
#endif
}

void GFX_FCT(set_font)(const font_t *font)
{
	__gfx_state.font = font;
}
void GFX_FCT(set_font_color)(gfx_color_t col)
{
	__gfx_state.font_color   = col;
#if GFX_DMA2D_FONTS
	__gfx_state.font_pxbuf.in.pixel.alpha_mode.color = col.raw;
#endif
}

#if GFX_DMA2D_FONTS
void GFX_FCT(set_font_blending)(bool enable) {
	__gfx_state.font_blending = enable;
}
bool GFX_FCT(get_font_blending)() {
	return __gfx_state.font_blending;
}
#else
void GFX_FCT(set_font_scale)(uint8_t s)
{
	__gfx_state.font_scale = (s > 0) ? s : 1;
}
uint8_t GFX_FCT(get_font_scale)()
{
	return __gfx_state.font_scale;
}
#endif

void GFX_FCT(set_text_wrap)(bool w)
{
	__gfx_state.text_wrap = w;
}

void GFX_FCT(set_cursor)(int16_t x, int16_t y)
{
	__gfx_state.cursor_x_orig = x;
	__gfx_state.cursor_x = x;
	__gfx_state.cursor_y = y;
}
int16_t GFX_FCT(get_cursor_x)()
{
	return __gfx_state.cursor_x;
}
int16_t GFX_FCT(get_cursor_y)()
{
	return __gfx_state.cursor_y;
}

gfx_color_t
GFX_FCT(get_font_color)()
{
	return __gfx_state.font_color;
}
const font_t *
GFX_FCT(get_font)()
{
	return __gfx_state.font;
}
uint8_t
GFX_FCT(get_text_wrap)()
{
	return __gfx_state.text_wrap;
}

uint16_t
GFX_FCT(get_char_width)()
{
	if (!__gfx_state.font) {
		return 0;
	} else {
		return __gfx_state.font->charwidth*__gfx_state.font_scale;
	}
}
uint16_t
GFX_FCT(get_line_height)()
{
	if (!__gfx_state.font) {
		return 0;
	}
	return __gfx_state.font->lineheight*__gfx_state.font_scale;
}
static inline
const char *
GFX_FCT(next_line)(const char *s) {
	while ((*s != 0) && (*s != '\n') && (*s != '\r')) {
		s++;
	}
	return s;
}
uint16_t
GFX_FCT(get_string_width)(const char *s)
{
	uint16_t cnt, cnt_max;
	cnt_max = 0;
	while (1) {
		const char *sn = GFX_FCT(next_line)(s);
		cnt = (uint16_t)(sn-s); s = sn;
		if ((*s == 0) || (*s == '\n') || (*s == '\r')) {
			if (cnt_max < cnt) {
				cnt_max = cnt;
			}
			if (*s == 0) {
				break;
			}
		}
		s++;
	}
	return cnt_max * GFX_FCT(get_char_width)();
}
uint16_t
GFX_FCT(get_string_height)(const char *s)
{
	uint16_t cnt;
	cnt = 1;
	while (*s) {
		if (*s == '\n') {
			cnt++;
		}
		s++;
	}
	return cnt * GFX_FCT(get_line_height)();
}
uint16_t
GFX_FCT(get_string_height2)(const char *s, int16_t max_width) {
	uint16_t cnt;
	cnt = 1;
	uint16_t mw = 1+(max_width-GFX_FCT(get_char_width)()+1)/GFX_FCT(get_char_width)();
	while (1) {
		const char *sn = GFX_FCT(next_line)(s);
		uint16_t w = (uint16_t)(sn-s);
		cnt += w/mw;
		s = sn;
		if (!*s) break;
		cnt++;
		s++;
	}
	return cnt * GFX_FCT(get_line_height)();
}

