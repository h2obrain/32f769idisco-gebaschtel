/*
 * dma2d_helper.h
 *
 *  Created on: 1 May 2019
 *      Author: h2obrain
 */

#ifndef DMA2D_HELPER_FUNCTIONS_H_
#define DMA2D_HELPER_FUNCTIONS_H_

#include <stdint.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma2d.h>
#include <drivers/dsi_types.h>

#define DMA2D_CLOCKS \
	RCC_DMA2D

typedef struct {
	const void *buffer;
	uint32_t width, height;
	struct {
		struct {
			uint32_t bitsize, format;
			union {
				struct {
					void *clut;
					uint8_t clut_size;
					bool clut_is_24bit;
				} clut_mode;
				struct {
					uint32_t color;
				} alpha_mode;
			};
		} pixel;
		struct {
			/** alpha modification mode DMA2D_xPFCCR_AM_xxx */
			uint8_t mode;
			/** alpha modification value 0-255 */
			uint8_t alpha;
		} alpha_mod;
	} in;
	struct {
		struct {
			uint32_t bytesize, format;
		} pixel;
	} out;
} dma2d_pixel_buffer_t;



/*
 * Functions
 */

/* */
void dma2d_init(void);
/* ltdc interoperability */
void dma2d_setup_ltdc_pixel_buffer(display_layer_t layer, dma2d_pixel_buffer_t *pxbuf);
/* */
void dma2d_wait_complete(void);
void dma2d_fill(
		dma2d_pixel_buffer_t *pxdst,
		uint32_t color,
		int16_t dx,int16_t dy,
		int16_t w,int16_t h
	);
void dma2d_copy(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
	);
void dma2d_convert_copy(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
	);
void dma2d_convert_blenddst_copy(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
	);
void dma2d_convert_blenddst_copy_no_pxsrc_fix(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
	);
void dma2d_convert_blend_copy(
		dma2d_pixel_buffer_t *pxsrcfg,
		dma2d_pixel_buffer_t *pxsrcbg,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx_fg, int16_t sy_fg,
		int16_t sx_bg, int16_t sy_bg,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
	);

#endif /* DMA2D_HELPER_FUNCTIONS_H_ */
