// The things actually doing the matrix manipulation.
// Also contains the buffers.
//
// Copyright (c) 2019, Adrian "vifino" Pistol <vifino@tty.sh>
// 
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "types.h"
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <drivers/display.h>
#include <gfx/gfx.h>
#include <drivers/dma2d_helper_functions.h>
#include "sled/src/matrix.h"
#include "sled/src/random.h"
#include "sled/src/timers.h"
unsigned int randn(unsigned int n) {
	return (uint32_t)((uint64_t)rand()*n/RAND_MAX);
}

// This is where the matrix functions send output.
// It is the root of the output chain.
//static int mod_out_no;

int matrix_init(int outmodno) {
	(void)outmodno;
	return 0;
}

#define PX_SIZE 20
int matrix_getx(void) {
//	return 300;
	return gfx_width()/PX_SIZE;
//	return 100;
}
int matrix_gety(void) {
//	return 300;
	return gfx_height()/PX_SIZE;
//	return 100;
}

static inline
gfx_color_t RGB_to_argb8888(RGB color) {
	return (gfx_color_t){ .argb8888 = { .a=color.alpha, .r=color.red, .g=color.green, .b=color.blue} };
}
static inline
RGB RGB_from_argb8888(gfx_color_t c) {
	return (RGB){ .alpha=c.argb8888.a, .red=c.argb8888.r, .green=c.argb8888.g, .blue=c.argb8888.b };
}
static dma2d_pixel_buffer_t *pxdst;
static dma2d_pixel_buffer_t _pxdst;
static inline void init_pxdst(void) {
	if (!pxdst) {
		pxdst = &_pxdst;
		dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, pxdst);
	}
}

#define ABSI(i) (i<0?-i:i)
#define MAXI(a,b) (a>b?a:b)
#define MINI(a,b) (a>b?b:a)
static dma2d_pixel_buffer_t *pxsrc;
static dma2d_pixel_buffer_t _pxsrc;
static inline void init_pxsrc(void) {
	if (!pxsrc) {
		init_pxdst();
		_pxsrc.buffer = malloc(PX_SIZE*PX_SIZE);
		_pxsrc.width  =
		_pxsrc.height = PX_SIZE;
		_pxsrc.in.pixel.bitsize = 8;
		_pxsrc.in.pixel.format  = DMA2D_xPFCCR_CM_A8;
		_pxsrc.in.pixel.alpha_mode.color = 0;
		_pxsrc.out.pixel.bytesize = // out is not supported!
		_pxsrc.out.pixel.format = 0;
//		double max_v = PX_SIZE * pow(2,0.5)/2;
		for (int16_t y=0;y<PX_SIZE; y++) {
			for (int16_t x=0;x<PX_SIZE; x++) {
				double xd  = (double)x-(double)PX_SIZE/2;
				double yd  = (double)y-(double)PX_SIZE/2;
				double d   = 255*pow(pow(xd*xd+yd*yd,0.5)/(PX_SIZE/2),2); //max_v;
				uint32_t a = (uint32_t)lround(d);
				((uint8_t (*)[PX_SIZE])_pxsrc.buffer)[y][x] = 255-(uint8_t)MINI(a,255);
//				((uint8_t (*)[PX_SIZE])_pxsrc.buffer)[y][x] = 255*ABSI((x+y)/2+1-PX_SIZE)/PX_SIZE;
			}
		}
		pxsrc = & _pxsrc;
	}
}
int matrix_set(int x, int y, RGB color) {
//	gfx_draw_pixel((int16_t)x,(int16_t)y,RGB_to_arPX_SIZEgb8888(color));
//	init_pxdst();
//	dma2d_fill(pxdst, RGB_to_argb8888(color).raw, x*PX_SIZE,y*PX_SIZE, PX_SIZE,PX_SIZE);
	init_pxsrc();
	_pxsrc.in.pixel.alpha_mode.color = RGB_to_argb8888(color).raw;
	dma2d_convert_copy__no_pxsrc_fix(pxsrc,pxdst, 0,0, x*PX_SIZE,y*PX_SIZE, PX_SIZE,PX_SIZE);
	return 0;
}

RGB matrix_get(int x, int y) {
	return RGB_from_argb8888(gfx_get_pixel((int16_t)x*PX_SIZE+PX_SIZE/2,(int16_t)y*PX_SIZE+PX_SIZE/2));
}

// Fills part of the matrix with jo-- a single color.
int matrix_fill(int start_x, int start_y, int end_x, int end_y, RGB color) {
	if (start_x > end_x)
		return 1;
	if (start_y > end_y)
		return 2;

	init_pxdst();

	dma2d_fill(pxdst, RGB_to_argb8888(color).raw, start_x,start_y, end_x-start_x,end_y-start_y);
//	int x;
//	int y;
//	for (y = MAX(start_y, 0); y <= MIN(end_y, matrix_gety()); y++)
//		for (x = MAX(start_x, 0); x <= MIN(end_x, matrix_getx()); x++) {
//			matrix_set(x, y, color);
//		}
	return 0;
}

// Zeroes the stuff.
int matrix_clear(void) {
	init_pxdst();
	dma2d_fill(pxdst,0,0,0,matrix_getx(),matrix_gety());
	return 0;
}

int matrix_render(void) {
	while (!display_ready());

	uint64_t time = mtime();
#define FPS_SAMPLE_TIME 1000
	static double fps = 0;
	static uint64_t fps_timeout = FPS_SAMPLE_TIME;
	static uint32_t c = 0;
	static char buf[256]={0};
	c++;
	if (fps_timeout<=time) {
		fps = (double)(1000*c)/(time+FPS_SAMPLE_TIME-fps_timeout);
		c = 0;
		fps_timeout = time+FPS_SAMPLE_TIME;
		sprintf(buf, "% 7.03f fps", fps);
	}
	//gfx_set_font_scale(2);
//#define font font_Tamsyn10x20b_20
#define font (&font_DejaVuSansMono_36)
	init_pxdst();
//	dma2d_fill(pxdst, 0, 10,10,strnlen(buf,256)*font->charwidth,font->lineheight);
//				dma2d_wait_complete();
//	gfx_set_font_blending(false);
	gfx_puts2(10,10,buf,font,(gfx_color_t){.argb8888.c=0xffffffff});

	display_update();
	return 0;
}

int matrix_deinit(void) {
	//~ ..
	return 0;
}
