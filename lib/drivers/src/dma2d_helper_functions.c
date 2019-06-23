/*
 * dma2d_helper_functions.c
 *
 *  Created on: 19 May 2019
 *      Author: h2obrain
 */

#include <stddef.h>
#include <assert.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/ltdc.h>
#include <drivers/dma2d_helper_functions.h>
#include <support/clock.h>

void dma2d_init() {
	rcc_periph_reset_pulse(RST_DMA2D);
	nvic_disable_irq(NVIC_DMA2D_IRQ);
//	nvic_set_priority(NVIC_DMA2D_IRQ,10);
	DMA2D_CR &= ~(DMA2D_CR_CEIE | DMA2D_CR_CTCIE | DMA2D_CR_CAEIE
			    | DMA2D_CR_TWIE | DMA2D_CR_TCIE | DMA2D_CR_TEIE);
	DMA2D_CR |= DMA2D_CR_CEIE | DMA2D_CR_CAEIE | DMA2D_CR_TEIE;
	nvic_enable_irq(NVIC_DMA2D_IRQ);
}
/**
 * Wait num cycles between ahb accesses
 * @param num_cycles
 */
void dma2d_set_ahb_master_timer(uint8_t num_cycles) {
	DMA2D_AMTCR |= ~(DMA2D_AMTCR_DT_MASK << DMA2D_AMTCR_DT_SHIFT);
	DMA2D_AMTCR |= num_cycles << DMA2D_AMTCR_DT_SHIFT;
	DMA2D_AMTCR |= DMA2D_AMTCR_EN;
}

void dma2d_isr(void) {
//	uint32_t isr = DMA2D_ISR;
//	DMA2D_IFCR = isr;
	wait_cycles(3); // wait a bit for the isr to be cleared
	while (1);
}


/* ltdc interoperability */
void dma2d_setup_ltdc_pixel_buffer(display_layer_t layer, dma2d_pixel_buffer_t *pxbuf) {
	pxbuf->buffer = (void *)ltdc_get_fbuffer_address(layer);

	uint32_t window_x0,window_x1;
	uint32_t window_y0,window_y1;
	window_x1 = ((LTDC_LxWHPCR(layer) >> LTDC_LxWHPCR_WHSPPOS_SHIFT) & LTDC_LxWHPCR_WHSPPOS_MASK);
	window_y1 = ((LTDC_LxWVPCR(layer) >> LTDC_LxWVPCR_WVSPPOS_SHIFT) & LTDC_LxWVPCR_WVSPPOS_MASK);

	window_x0 = ((LTDC_LxWHPCR(layer) >> LTDC_LxWHPCR_WHSTPOS_SHIFT) & LTDC_LxWHPCR_WHSTPOS_MASK);
	window_y0 = ((LTDC_LxWVPCR(layer) >> LTDC_LxWVPCR_WVSTPOS_SHIFT) & LTDC_LxWVPCR_WVSTPOS_MASK);

	pxbuf->width  = window_x1-window_x0 + 1;
	pxbuf->height = window_y1-window_y0 + 1;

	/* TODO maybe remove the next two switches? */

	/* input color */
	pxbuf->in.alpha_mod.mode  = 0;
	pxbuf->in.alpha_mod.alpha = 0xff;
	switch (LTDC_LxPFCR(layer)) {
		case LTDC_LxPFCR_ARGB8888 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_ARGB8888;
			pxbuf->in.pixel.bitsize = 4*8;
			break;
		case LTDC_LxPFCR_RGB888 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_RGB888;
			pxbuf->in.pixel.bitsize = 3*8;
			break;
		case LTDC_LxPFCR_RGB565 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_RGB565;
			pxbuf->in.pixel.bitsize = 2*8;
			break;
		case LTDC_LxPFCR_ARGB1555 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_ARGB1555;
			pxbuf->in.pixel.bitsize = 2*8;
			break;
		case LTDC_LxPFCR_ARGB4444 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_ARGB4444;
			pxbuf->in.pixel.bitsize = 2*8;
			break;
		case LTDC_LxPFCR_L8 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_L8;
			pxbuf->in.pixel.bitsize = 1*8;
			/* TODO bitch about color */
			//pxbuf->in.alpha_mode.color = ???;
			break;
		case LTDC_LxPFCR_AL44 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_AL44;
			pxbuf->in.pixel.bitsize = 1*8;
			/* TODO bitch about clut (cannot be retrieved from ltdc) */
			//pxbuf->in.clut_mode.clut = ???;
			//pxbuf->in.clut_mode.clut_size = ???;
			//pxbuf->in.clut_mode.clut_is_24bit = ???;
			break;
		case LTDC_LxPFCR_AL88 :
			pxbuf->in.pixel.format  = DMA2D_xPFCCR_CM_AL88;
			pxbuf->in.pixel.bitsize = 1*8;
			break;
		default:
			assert("Unsupported destination color format");
			break;
	}
	/* output color */
	switch (LTDC_LxPFCR(layer)) {
		case LTDC_LxPFCR_ARGB8888 :
			pxbuf->out.pixel.format   = DMA2D_OPFCCR_CM_ARGB8888;
			pxbuf->out.pixel.bytesize = 4;
			break;
		case LTDC_LxPFCR_RGB888 :
			pxbuf->out.pixel.format   = DMA2D_OPFCCR_CM_RGB888;
			pxbuf->out.pixel.bytesize = 3;
			break;
		case LTDC_LxPFCR_RGB565 :
			pxbuf->out.pixel.format   = DMA2D_OPFCCR_CM_RGB565;
			pxbuf->out.pixel.bytesize = 2;
			break;
		case LTDC_LxPFCR_ARGB1555 :
			pxbuf->out.pixel.format   = DMA2D_OPFCCR_CM_ARGB1555;
			pxbuf->out.pixel.bytesize = 2;
			break;
		case LTDC_LxPFCR_ARGB4444 :
			pxbuf->out.pixel.format   = DMA2D_OPFCCR_CM_ARGB4444;
			pxbuf->out.pixel.bytesize = 2;
			break;
		case LTDC_LxPFCR_L8 :
		case LTDC_LxPFCR_AL44 :
		case LTDC_LxPFCR_AL88 :
		default:
			assert("Unsupported destination color format");
			break;
	}
}

/* color and blending stuff */
static inline
void dsi_dma2d_set_input_color_format(
		dma2d_pixel_buffer_t *pxbuf,
		volatile uint32_t *xpfccr,
		volatile uint32_t *xcmar,
		volatile uint32_t *xcolr
) {
	*xpfccr &= ~(
				(DMA2D_xPFCCR_CM_MASK << DMA2D_xPFCCR_CM_SHIFT)
			  | (DMA2D_xPFCCR_AM_MASK << DMA2D_xPFCCR_AM_SHIFT)
			  | (DMA2D_xPFCCR_ALPHA_MASK << DMA2D_xPFCCR_ALPHA_SHIFT)
			  | (DMA2D_xPFCCR_CS_MASK << DMA2D_xPFCCR_CS_SHIFT)
			);
	*xpfccr |= pxbuf->in.pixel.format << DMA2D_xPFCCR_CM_SHIFT;
	*xpfccr |= pxbuf->in.alpha_mod.mode << DMA2D_xPFCCR_AM_SHIFT;
	*xpfccr |= pxbuf->in.alpha_mod.alpha << DMA2D_xPFCCR_ALPHA_SHIFT;
	switch (pxbuf->in.pixel.format) {
		case DMA2D_xPFCCR_CM_ARGB8888 :
		case DMA2D_xPFCCR_CM_RGB888 :
		case DMA2D_xPFCCR_CM_RGB565 :
		case DMA2D_xPFCCR_CM_ARGB1555 :
		case DMA2D_xPFCCR_CM_ARGB4444 :
			break;
		case DMA2D_xPFCCR_CM_L4 :
		case DMA2D_xPFCCR_CM_L8 :
		case DMA2D_xPFCCR_CM_AL44 :
		case DMA2D_xPFCCR_CM_AL88 :
			/* Setup/load CLUT */
			if (*xcmar != (uint32_t)pxbuf->in.pixel.clut_mode.clut) { // this test is not 100% safe..
				*xcmar  = (uint32_t)pxbuf->in.pixel.clut_mode.clut;
				*xpfccr |= (uint32_t)pxbuf->in.pixel.clut_mode.clut_size << DMA2D_xPFCCR_CS_SHIFT;
				if (pxbuf->in.pixel.clut_mode.clut_is_24bit) {
					*xpfccr |= DMA2D_xPFCCR_CCM_RGB888;
				} else {
					*xpfccr &= ~DMA2D_xPFCCR_CCM_RGB888;
				}
				*xpfccr |= DMA2D_xPFCCR_START;
				//while (*xpfccr & DMA2D_xPFCCR_START); // done before start..
			}
			break;
		case DMA2D_xPFCCR_CM_A8 :
		case DMA2D_xPFCCR_CM_A4 :
			/* Set color */
			*xcolr = pxbuf->in.pixel.alpha_mode.color;
			break;
		default:
			assert("Unsupported destination color format");
			break;
	}
}
static inline
void dsi_dma2d_set_output_color_format(
		dma2d_pixel_buffer_t *pxbuf
) {
	DMA2D_OPFCCR &= ~(DMA2D_OPFCCR_CM_MASK << DMA2D_OPFCCR_CM_SHIFT);
	DMA2D_OPFCCR |= pxbuf->out.pixel.format << DMA2D_OPFCCR_CM_SHIFT;
}
/* positional stuff */
static inline
void dma2d_fix_pos_smaller_zero(
		int16_t *p, int16_t *po1,int16_t *po2, int16_t *s
) {
	if (*p<0) {
		*s   += *p;
		if (po1) *po1 -= *p;
		if (po2) *po2 -= *p;
		*p    = 0;
	}
}
static inline __attribute__((always_inline))
bool dma2d_fix_pos_size(
		int16_t *sx_fg, int16_t *sy_fg, int16_t W_fg, int16_t H_fg,
		int16_t *sx_bg, int16_t *sy_bg, int16_t W_bg, int16_t H_bg,
		int16_t *dx,    int16_t *dy,    int16_t W_d,  int16_t H_d,
		int16_t *w, int16_t *h
) {
	/* x<0 */
	           dma2d_fix_pos_smaller_zero(dx, sx_fg, sx_bg, w);
	if (sx_fg) dma2d_fix_pos_smaller_zero(sx_fg, sx_bg, dx, w);
	if (sx_bg) dma2d_fix_pos_smaller_zero(sx_bg, dx, sx_fg, w);
	/* y<0 */
	           dma2d_fix_pos_smaller_zero(dy, sy_fg, sy_bg, h);
	if (sy_fg) dma2d_fix_pos_smaller_zero(sy_fg, sy_bg, dy, h);
	if (sy_bg) dma2d_fix_pos_smaller_zero(sy_bg, dy, sy_fg, h);
	/* w<0 */
	if (*w<=0) return false;
	/* h<0 */
	if (*h<=0) return false;
	/* x>=W */
	             if (*dx>=W_d)     return false;
	if (sx_fg) { if (*sx_fg>=W_fg) return false; }
	if (sx_bg) { if (*sx_bg>=W_bg) return false; }
	/* y>=H */
	             if (*dy>=H_d)     return false;
	if (sy_fg) { if (*sy_fg>=H_fg) return false; }
	if (sy_bg) { if (*sy_bg>=H_bg) return false; }
	/* x+w>W */
	             if (*dx+*w>W_d)     *w = W_d - *dx;
	if (sx_fg) { if (*sx_fg+*w>W_fg) *w = W_fg - *sx_fg; }
	if (sx_bg) { if (*sx_bg+*w>W_bg) *w = W_bg - *sx_bg; }
	/* y+h>H */
	             if (*dy+*h>H_d)     *h = H_d - *dy;
	if (sy_fg) { if (*sy_fg+*h>H_fg) *h = H_fg - *sy_fg; }
	if (sy_bg) { if (*sy_bg+*h>H_bg) *h = H_bg - *sy_bg; }

	return true;
}

static inline
void dma2d_set_source_area(
		dma2d_pixel_buffer_t *pxbuf,
		volatile uint32_t *xxmar, volatile uint32_t *xxor,
		uint32_t sx, uint32_t sy, uint32_t w
) {
//	*xxmar = (uint32_t)pxbuf->buffer + ((sx + sy * pxbuf->width) * pxbuf->in.pixel.bitsize + 7)/8;
	uint32_t bit_offset = (sx + sy * pxbuf->width) * pxbuf->in.pixel.bitsize;
	assert("Offset has to be byte-aligned" && (bit_offset % 8 == 0));
	*xxmar = (uint32_t)pxbuf->buffer + bit_offset / 8;
	*xxor  = pxbuf->width - w;
}
static inline
void dma2d_set_destination_area(
		dma2d_pixel_buffer_t *pxbuf,
		uint32_t dx,uint32_t dy,uint32_t w,uint32_t h
) {
	DMA2D_OMAR = (uint32_t)pxbuf->buffer + (dx + dy*pxbuf->width)*pxbuf->out.pixel.bytesize;
	DMA2D_OOR  = pxbuf->width-w;
	DMA2D_NLR  = (w << DMA2D_NLR_PL_SHIFT) | (h << DMA2D_NLR_NL_SHIFT);
}

/* exported functions */
void dma2d_wait_complete() {
	while (DMA2D_CR & DMA2D_CR_START);
}
void dma2d_fill(
		dma2d_pixel_buffer_t *pxdst,
		uint32_t color,
		int16_t dx,int16_t dy,
		int16_t w,int16_t h
) {
	if (!dma2d_fix_pos_size(
			NULL,NULL,0,0,
			NULL,NULL,0,0,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;

	dma2d_wait_complete();

	dsi_dma2d_set_output_color_format(pxdst);

	DMA2D_OCOLR = color;

	dma2d_set_destination_area(pxdst, dx,dy,w,h);

	DMA2D_CR &= ~(DMA2D_CR_MODE_MASK << DMA2D_CR_MODE_SHIFT);
	DMA2D_CR |= DMA2D_CR_MODE_R2M << DMA2D_CR_MODE_SHIFT;

	DMA2D_CR |= DMA2D_CR_START;
}
void dma2d_copy(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	if (!dma2d_fix_pos_size(
			&sx,&sy,(int16_t)pxsrc->width,(int16_t)pxsrc->height,
			NULL,NULL,0,0,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;

	dma2d_wait_complete();

	dsi_dma2d_set_input_color_format(pxsrc, &DMA2D_FGPFCCR,&DMA2D_FGCMAR,&DMA2D_FGCOLR); // << check why this is needed!
	dsi_dma2d_set_output_color_format(pxdst);

	dma2d_set_source_area(pxsrc, &DMA2D_FGMAR,&DMA2D_FGOR, sx,sy,w);
	dma2d_set_destination_area(pxdst, dx,dy,w,h);

	DMA2D_CR &= ~(DMA2D_CR_MODE_MASK << DMA2D_CR_MODE_SHIFT);
	DMA2D_CR |= DMA2D_CR_MODE_M2M << DMA2D_CR_MODE_SHIFT;

	DMA2D_CR |= DMA2D_CR_START;
}
static inline
void dma2d_convert_copy_internal(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	dma2d_wait_complete();

	dsi_dma2d_set_input_color_format(pxsrc, &DMA2D_FGPFCCR,&DMA2D_FGCMAR,&DMA2D_FGCOLR);
	dsi_dma2d_set_output_color_format(pxdst);

	dma2d_set_source_area(pxsrc, &DMA2D_FGMAR,&DMA2D_FGOR, sx,sy,w);
	dma2d_set_destination_area(pxdst, dx,dy,w,h);

	DMA2D_CR &= ~(DMA2D_CR_MODE_MASK << DMA2D_CR_MODE_SHIFT);
	DMA2D_CR |= DMA2D_CR_MODE_M2MWPFC << DMA2D_CR_MODE_SHIFT;

	/* wait for CLUTs to be loaded */
	while (DMA2D_FGPFCCR & DMA2D_xPFCCR_START);

	DMA2D_CR |= DMA2D_CR_START;
}
void dma2d_convert_copy(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	if (!dma2d_fix_pos_size(
			&sx,&sy,(int16_t)pxsrc->width,(int16_t)pxsrc->height,
			NULL,NULL,0,0,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;
	dma2d_convert_copy_internal(pxsrc,pxdst, sx,sy, dx,dy, w,h);
}
void dma2d_convert_copy__no_pxsrc_fix(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	if (!dma2d_fix_pos_size(
			NULL,NULL,0,0,
			NULL,NULL,0,0,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;
	dma2d_convert_copy_internal(pxsrc,pxdst, sx,sy, dx,dy, w,h);
}
static inline
void dma2d_convert_blend_copy_internal(
		dma2d_pixel_buffer_t *pxsrc_fg,
		dma2d_pixel_buffer_t *pxsrc_bg,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx_fg, int16_t sy_fg,
		int16_t sx_bg, int16_t sy_bg,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	assert(pxsrc_fg->buffer);

	dma2d_wait_complete();

	dsi_dma2d_set_input_color_format(pxsrc_fg, &DMA2D_FGPFCCR,&DMA2D_FGCMAR,&DMA2D_FGCOLR);
	dsi_dma2d_set_input_color_format(pxsrc_bg, &DMA2D_BGPFCCR,&DMA2D_BGCMAR,&DMA2D_BGCOLR);
	dsi_dma2d_set_output_color_format(pxdst);

	dma2d_set_source_area(pxsrc_fg, &DMA2D_FGMAR,&DMA2D_FGOR, sx_fg,sy_fg,w);
	dma2d_set_source_area(pxsrc_bg, &DMA2D_BGMAR,&DMA2D_BGOR, sx_bg,sy_bg,w);
	dma2d_set_destination_area(pxdst, dx,dy,w,h);

	DMA2D_CR &= ~(DMA2D_CR_MODE_MASK << DMA2D_CR_MODE_SHIFT);
	DMA2D_CR |= DMA2D_CR_MODE_M2MWB << DMA2D_CR_MODE_SHIFT;

	/* wait for CLUTs to be loaded */
	while (DMA2D_BGPFCCR & DMA2D_xPFCCR_START);
	while (DMA2D_FGPFCCR & DMA2D_xPFCCR_START);

	DMA2D_CR |= DMA2D_CR_START;
}

void dma2d_convert_blenddst(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	if (!dma2d_fix_pos_size(
			&sx,&sy,(int16_t)pxsrc->width,(int16_t)pxsrc->height,
			NULL,NULL,0,0,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;
	dma2d_convert_blend_copy_internal(pxsrc,pxdst,pxdst, sx,sy, dx,dy, dx,dy, w,h);
}
void dma2d_convert_blenddst__no_pxsrc_fix(
		dma2d_pixel_buffer_t *pxsrc,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx, int16_t sy,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	if (!dma2d_fix_pos_size(
			NULL,NULL,0,0,
			NULL,NULL,0,0,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;
	dma2d_convert_blend_copy_internal(pxsrc,pxdst,pxdst, sx,sy, dx,dy, dx,dy, w,h);
}
void dma2d_convert_blend_copy(
		dma2d_pixel_buffer_t *pxsrc_fg,
		dma2d_pixel_buffer_t *pxsrc_bg,
		dma2d_pixel_buffer_t *pxdst,
		int16_t sx_fg, int16_t sy_fg,
		int16_t sx_bg, int16_t sy_bg,
		int16_t dx, int16_t dy,
		int16_t w, int16_t h
) {
	if (!dma2d_fix_pos_size(
			&sx_fg,&sy_fg,(int16_t)pxsrc_fg->width,(int16_t)pxsrc_fg->height,
			&sx_bg,&sy_bg,(int16_t)pxsrc_bg->width,(int16_t)pxsrc_bg->height,
			&dx,&dy,(int16_t)pxdst->width,(int16_t)pxdst->height,
			&w,&h
		)
	) return;
	dma2d_convert_blend_copy_internal(pxsrc_fg,pxsrc_bg,pxdst, sx_fg,sy_fg, sx_bg,sy_bg, dx,dy, w,h);
}
