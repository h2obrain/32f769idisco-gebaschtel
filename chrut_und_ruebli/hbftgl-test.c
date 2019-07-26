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

#include "hbftgl-test.h"
#include "freetype-gl.h"
//#include "vertex-buffer.h"
//#include "texture-font.h"

#include "data/fonts/DejaVuSerif_ttf.h"
#include "data/fonts/amiri-0.104/amiri-regular_ttf.h"
#include "data/fonts/fireflysung-nano_ttf.h"

#include "data/texts/lorem_ipsum_dolor.h"

static const char *text_ar  = "صِف خَلقَ خَودِ كَمِثلِ الشَمسِ إِذ بَزَغَت — يَحظى الضَجيعُ بِها نَجلاءَ مِعطارِ";
//static const char *font_filename      = "fonts/amiri-regular.ttf";
static const char *language_ar           = "ar";
static const hb_direction_t direction_ar = HB_DIRECTION_RTL;
static const hb_script_t script_ar       = HB_SCRIPT_ARABIC;

static const char *text_lat = "moin! ffi möh?";
static const char *language_lat           = "en";
static const hb_direction_t direction_lat = HB_DIRECTION_LTR;
static const hb_script_t script_lat       = HB_SCRIPT_LATIN;

static texture_atlas_t *atlas;
static dma2d_pixel_buffer_t atlas_surface;

#define NUM_FONTS (sizeof(fonts)/sizeof(texture_font_t *))
static texture_font_t *font_lat;
static texture_font_t *font_lat_huge;
static texture_font_t *fonts[15];

//#define PRELOAD_GLYPHS

dma2d_pixel_buffer_t *hbftgl_test_init() {
	atlas = texture_atlas_new( 300, 480, 1 );

	float dpi = 216; // 72 216 233.238

	float font_size_pt = 16;
	printf("Creating latin font size: %.1f\n", font_size_pt);
	font_lat = texture_font_new_from_memory(atlas, font_size_pt,dpi,100, DejaVuSerif_ttf,DejaVuSerif_ttf_size, language_lat);
	font_lat->rendermode = RENDER_OUTLINE_EDGE;
	font_lat->outline_thickness = 0.4f;
//	font_lat->hinting = false;
//	font_lat->filtering = false;
#ifdef PRELOAD_GLYPHS
	texture_font_load_glyphs(font_lat, text_lat);
#else
	// TODO: for some reason, a single char needs to be loaded before copying the font..
	texture_font_load_glyph(font_lat, text_lat);
#endif
	printf("Creating font size: %.1f\n", 50.0f);
	font_lat_huge = texture_font_clone(font_lat, 50.0f);
	texture_font_load_glyph(font_lat_huge, "@");

	font_size_pt = 1;
	printf("Creating font size: %.1f\n", font_size_pt);
	fonts[0] = texture_font_new_from_memory(atlas, font_size_pt,dpi,1, amiri_regular_ttf,amiri_regular_ttf_size, language_ar);
//	fonts[0] = texture_font_new_from_file(atlas, font_size_pt, font_filename, language);
	fonts[0]->rendermode = RENDER_NORMAL;
//	fonts[0]->hinting = false;
//	fonts[0]->filtering = false;
#ifdef PRELOAD_GLYPHS
	texture_font_load_glyphs(fonts[0], text_ar);
#else
	// TODO: for some reason, a single char needs to be loaded before copying the font..
	texture_font_load_glyph(fonts[0], text_ar);
#endif
	for (size_t i=1; i<NUM_FONTS; i++) {
		font_size_pt += 0.3f;
		printf("Creating font size: %.1f\n", font_size_pt);
//		fonts[i] = texture_font_new_from_file(atlas, 12+i, font_filename, language);
		fonts[i] = texture_font_clone(fonts[0], font_size_pt);
#ifdef PRELOAD_GLYPHS
		texture_font_load_glyphs(fonts[i], text_ar );
#endif
		}

//	dma2d_pixel_buffer_t *atlas_surface = malloc(sizeof(dma2d_pixel_buffer_t));
	atlas_surface = (dma2d_pixel_buffer_t) {
		.buffer = atlas->data,
		.width  = atlas->width,
		.height = atlas->height,
		.in = {
			.pixel = {
					.bitsize = 8,
					.format  = DMA2D_xPFCCR_CM_A8,
					.alpha_mode = {
							.color = 0xffffff,
					},
			},
		},
		.out = {{0}},// out is not supported!
	};
	return &atlas_surface;
}
static float draw_text(
		float y,
		const char *text,
		texture_font_t *font, const hb_direction_t direction, const hb_script_t script,
		hb_buffer_t *buffer,
		dma2d_pixel_buffer_t *render_surface
	);
void hbftgl_test_loop(dma2d_pixel_buffer_t *render_surface, uint32_t color) {
	/* set color */
	atlas_surface.in.pixel.alpha_mode.color = color;

	/* Create a buffer for harfbuzz to use */
	hb_buffer_t *buffer = hb_buffer_create();

	/* draw text */
	float y;
	y = 0;
	for (size_t i=0; i<NUM_FONTS; ++i) {
		/* render */
		y = draw_text(y,text_ar,fonts[i],direction_ar,script_ar,buffer,render_surface);
		/* clean up the buffer, but don't kill it just yet */
		hb_buffer_reset(buffer);
	}
	/* render */
	y = draw_text(y,text_lat,font_lat,direction_lat,script_lat,buffer,render_surface);
	/* clean up the buffer, but don't kill it just yet */
	hb_buffer_reset(buffer);

	static uint64_t last_time = 0;
	static double fps = 0;
	uint64_t time = mtime();
	fps = /*0.9*fps + 0.1**/(1000.0/(time-last_time));
	last_time = time;
	char fps_buf[64];
	sprintf(fps_buf, "%.1f fps", fps);
	/* render */
	y = draw_text(y,fps_buf,font_lat,direction_lat,script_lat,buffer,render_surface);
	/* clean up the buffer, but don't kill it just yet */
	hb_buffer_reset(buffer);

	/* destroy the buffer */
	hb_buffer_destroy(buffer);

	/* display atlas */
	atlas_surface.in.pixel.alpha_mode.color = 0xffffff;
	dma2d_convert_copy__no_pxsrc_fix(
			&atlas_surface, render_surface,
			0,0, 0,0,
			atlas_surface.width, atlas_surface.height
		);
}
static float draw_text(
		float y,
		const char *text,
		texture_font_t *font, const hb_direction_t direction, const hb_script_t script,
		hb_buffer_t *buffer,
		dma2d_pixel_buffer_t *render_surface
) {
	hb_buffer_add_utf8( buffer, text, strlen(text), 0, strlen(text) );
	hb_buffer_set_language( buffer, font->language );
//	hb_buffer_guess_segment_properties( buffer );
	hb_buffer_set_script( buffer, script );
	hb_buffer_set_direction( buffer, direction );
	hb_shape( font->hb_ft_font, buffer, NULL, 0 );

	unsigned int         glyph_count;
	hb_glyph_info_t     *glyph_info;
	hb_glyph_position_t *glyph_pos;
	glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
	glyph_pos  = hb_buffer_get_glyph_positions(buffer, &glyph_count);

	texture_font_load_glyphs( font, text );

	float width = 0.0;
	float hres = font->hres;
	for (size_t i = 0; i < glyph_count; ++i) {
		int codepoint = glyph_info[i].codepoint;
		float x_advance = glyph_pos[i].x_advance/(float)(hres*64);
		float x_offset = glyph_pos[i].x_offset/(float)(hres*64);
		texture_glyph_t *glyph = texture_font_get_glyph32(font, codepoint);
		if ( i < (glyph_count-1) )
			width += x_advance + x_offset;
		else
			width += glyph->offset_x + glyph->width;
	}
	float x;
	x = render_surface->width - width - 10 ;
//		y = render_surface->height - i * (10+i) - 15;
//	printf("asc=%.3f desc=%.3f\n",font->ascender,font->descender);
	y += font->ascender;

	for (size_t j = 0; j < glyph_count; ++j) {
		texture_glyph_t *glyph = texture_font_get_glyph32(font, glyph_info[j].codepoint);
		if (glyph == NULL) continue;

		// because of vhinting trick we need the extra 64 (hres)
		float x_advance = glyph_pos[j].x_advance/(float)(hres*64);
		float x_offset = glyph_pos[j].x_offset/(float)(hres*64);
//		float y_advance = glyph_pos[j].y_advance/(float)(64);
		float y_offset = glyph_pos[j].y_offset/(float)(64);

		float x0 = x + x_offset + glyph->offset_x;
//		float x1 = x0 + glyph->width;
		float y0 = nearbyint(y - y_offset - glyph->offset_y);
//			float y1 = floor(y0 - glyph->height);

		dma2d_convert_copy__no_pxsrc_fix(
				&atlas_surface, render_surface,
				glyph->tex_region.x, glyph->tex_region.y,
				(int)x0,(int)y0,
				glyph->tex_region.width, glyph->tex_region.height
			);

		x += x_advance;
		//y += y_advance;
	}

	y -= font->descender;

	return y;
}
