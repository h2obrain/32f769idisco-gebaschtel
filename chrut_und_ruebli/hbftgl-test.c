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

#include "hbftgl-test.h"
#include "freetype-gl.h"
//#include "vertex-buffer.h"
//#include "texture-font.h"

#include "data/fonts/DejaVuSerif_ttf.h"
#include "data/fonts/amiri-0.104/amiri-regular_ttf.h"
#include "data/fonts/fireflysung-nano_ttf.h"

#include "data/texts/lorem_ipsum_dolor.h"

const char *text = "صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت — يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ";
//const char *font_filename      = "fonts/amiri-regular.ttf";
const hb_direction_t direction = HB_DIRECTION_RTL;
const hb_script_t script       = HB_SCRIPT_ARABIC;
const char *language           = "ar";


texture_atlas_t *atlas;
dma2d_pixel_buffer_t atlas_surface;

#define NUM_FONTS (sizeof(fonts)/sizeof(texture_font_t *))
static texture_font_t *fonts[15];

dma2d_pixel_buffer_t *hbftgl_test_init() {
	atlas = texture_atlas_new( 300, 330, 1 );
	printf("Creating font size: %zu\n", 8);
	fonts[0] = texture_font_new_from_memory(atlas, 8, amiri_regular_ttf,amiri_regular_ttf_size, language);
//	fonts[0] = texture_font_new_from_file(atlas, 8, font_filename, language);
	texture_font_load_glyphs(fonts[0], text );
	for (size_t i=1; i<NUM_FONTS; i++) {
		printf("Creating font size: %zu\n", 8+i);
//		fonts[i] = texture_font_new_from_file(atlas, 12+i, font_filename, language);
		fonts[i] = texture_font_clone(fonts[0], 8+i);
		texture_font_load_glyphs(fonts[i], text );
	}

//	dma2d_pixel_buffer_t *atlas_surface = malloc(sizeof(dma2d_pixel_buffer_t));
	atlas_surface = (dma2d_pixel_buffer_t) {
		.buffer = atlas->data,
		.width  = atlas->width,
		.height = atlas->height,
		.in.pixel.bitsize = 8,
		.in.pixel.format  = DMA2D_xPFCCR_CM_A8,
		.in.pixel.alpha_mode.color = 0xffffff,
		.out.pixel.bytesize = 0,// out is not supported!
		.out.pixel.format = 0,
	};
	return &atlas_surface;
}
void hbftgl_test_loop(dma2d_pixel_buffer_t *render_surface, uint32_t color) {
	/* Create a buffer for harfbuzz to use */
	hb_buffer_t *buffer = hb_buffer_create();

	atlas_surface.in.pixel.alpha_mode.color = color;

	/* display atlas */
	dma2d_convert_copy__no_pxsrc_fix(
			&atlas_surface, render_surface,
			0,0, 0,0,
			atlas_surface.width, atlas_surface.height
		);

	/* draw text */
	float x,y;
	y = 0;
	for (size_t i=0; i<NUM_FONTS; ++i) {
		hb_buffer_add_utf8( buffer, text, strlen(text), 0, strlen(text) );
		hb_buffer_set_language( buffer, fonts[i]->language );
//		hb_buffer_guess_segment_properties( buffer );
		hb_buffer_set_script( buffer, script );
		hb_buffer_set_direction( buffer, direction );
		hb_shape( fonts[i]->hb_ft_font, buffer, NULL, 0 );

		unsigned int         glyph_count;
		hb_glyph_info_t     *glyph_info;
		hb_glyph_position_t *glyph_pos;
		glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
		glyph_pos  = hb_buffer_get_glyph_positions(buffer, &glyph_count);

		texture_font_load_glyphs( fonts[i], text );

		float width = 0.0;
		float hres = fonts[i]->hres;
		for (size_t j = 0; j < glyph_count; ++j) {
			int codepoint = glyph_info[j].codepoint;
			float x_advance = glyph_pos[j].x_advance/(float)(hres*64);
			float x_offset = glyph_pos[j].x_offset/(float)(hres*64);
			texture_glyph_t *glyph = texture_font_get_glyph32(fonts[i], codepoint);
			if ( i < (glyph_count-1) )
				width += x_advance + x_offset;
			else
				width += glyph->offset_x + glyph->width;
		}
		x = render_surface->width - width - 10 ;
//		y = render_surface->height - i * (10+i) - 15;
		printf("asc=%.3f desc=%.3f\n",fonts[i]->ascender,fonts[i]->descender);
		y += fonts[i]->ascender;

		for (size_t j = 0; j < glyph_count; ++j) {
			texture_glyph_t *glyph = texture_font_get_glyph32(fonts[i], glyph_info[j].codepoint);
			if (glyph == NULL) continue;

			// because of vhinting trick we need the extra 64 (hres)
			float x_advance = glyph_pos[j].x_advance/(float)(hres*64);
			float x_offset = glyph_pos[j].x_offset/(float)(hres*64);
			float y_advance = glyph_pos[j].y_advance/(float)(64);
			float y_offset = glyph_pos[j].y_offset/(float)(64);

			float x0 = x + x_offset + glyph->offset_x;
//			float x1 = x0 + glyph->width;
			float y0 = floor(y + y_offset + glyph->offset_y);
//			float y1 = floor(y0 - glyph->height);

			dma2d_convert_copy__no_pxsrc_fix(
					&atlas_surface, render_surface,
					glyph->tex_region.x, glyph->tex_region.y,
					(int)x0,(int)y0,
					glyph->tex_region.width, glyph->tex_region.height
				);

			x += x_advance;
			y += y_advance;
		}

		y -= fonts[i]->descender;

		/* clean up the buffer, but don't kill it just yet */
		hb_buffer_reset(buffer);
	}

	/* destroy the buffer */
	hb_buffer_destroy(buffer);
}
