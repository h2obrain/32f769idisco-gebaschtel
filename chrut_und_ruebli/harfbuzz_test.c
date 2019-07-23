/* based on https://github.com/lxnt/ex-sdl-freetype-harfbuzz.git */

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include "harfbuzz_test.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <hb.h>
#include <hb-ft.h>

#include "data/fonts/DejaVuSerif_ttf.h"
#include "data/fonts/amiri-0.104/amiri-regular_ttf.h"
#include "data/fonts/fireflysung-nano_ttf.h"

#include "data/texts/lorem_ipsum_dolor.h"

typedef struct _spanner_baton_t {
	/* rendering part - assumes 8bpp surface */
	uint8_t *pixels; // set to the glyph's origin.
	uint8_t *first_pixel, *last_pixel; // bounds check
	int16_t pitch;

	/* sizing part */
	int min_span_x;
	int max_span_x;
	int min_y;
	int max_y;
} spanner_baton_t;

/* google this */
#ifndef unlikely
#define unlikely
#endif
#ifndef likely
#define likely
#endif

/* This spanner is write only, suitable for write-only mapped buffers,
   but can cause dark streaks where glyphs overlap, like in arabic scripts.

   Note how spanners don't clip against surface width - resize the window
   and see what it leads to. */
static
void spanner_wo(int y, int count, const FT_Span* spans, void *user) {
	spanner_baton_t *baton = (spanner_baton_t *) user;
	uint8_t *scanline = baton->pixels - y * (int) baton->pitch / sizeof(uint8_t);
	if (unlikely scanline < baton->first_pixel) return;
	for (int i = 0; i < count; i++) {
		short x = spans[i].x;
		int len = spans[i].len;
		/* clipping */
		if (unlikely x<0) { len += x; x = 0; }
		if (unlikely len<=0) continue;
		short d = baton->pitch / sizeof(uint8_t) - x;
		if (unlikely d<0) {
			continue;
		}
		d -= len;
		if (unlikely d<0) {
			len += d;
		}
		/* */
		uint8_t *start = scanline + x;
		if (unlikely start + len > baton->last_pixel) return;

		for (; len>0; len--) {
			*start++ = spans[i].coverage;
		}
	}
}

/* This spanner does read/modify/write, trading performance for accuracy.
   The color here is simply half coverage value in all channels,
   effectively mid-gray.
   Suitable for when artifacts mostly do come up and annoy.
   This might be optimized if one does rmw only for some values of x.
   But since the whole buffer has to be rw anyway, and the previous value
   is probably still in the cache, there's little point to. */
__attribute((unused))
static
void spanner_rw(int y, int count, const FT_Span* spans, void *user) {
	spanner_baton_t *baton = (spanner_baton_t *) user;
	uint8_t *scanline = baton->pixels - y * baton->pitch / sizeof(uint8_t);
	if (unlikely scanline < baton->first_pixel)
		return;

	for (int i = 0; i < count; i++) {
		uint8_t *start = scanline + spans[i].x;
		if (unlikely start + spans[i].len > baton->last_pixel)
			return;

		for (int x = 0; x < spans[i].len; x++) {
			//*start++ |= spans[i].coverage;
			*start = (uint8_t)(((uint32_t)spans[i].coverage + *start) / 2);
			start++;
		}
	}
}

/*  This spanner is for obtaining exact bounding box for the string.
	Unfortunately this can't be done without rendering it (or pretending to).
	After this runs, we get min and max values of coordinates used.
*/
static
void spanner_sizer(int y, int count, const FT_Span* spans, void *user) {
	spanner_baton_t *baton = (spanner_baton_t *) user;

	if (y < baton->min_y)
		baton->min_y = y;
	if (y > baton->max_y)
		baton->max_y = y;
	for (int i = 0 ; i < count; i++) {
		if (spans[i].x + spans[i].len > baton->max_span_x)
			baton->max_span_x = spans[i].x + spans[i].len;
		if (spans[i].x < baton->min_span_x)
			baton->min_span_x = spans[i].x;
	}
}

FT_SpanFunc spanner = spanner_wo;

static
void ftfdump(FT_Face ftf) {
	for(int i=0; i<ftf->num_charmaps; i++) {
		printf("%d: %s %s %c%c%c%c plat=%hu id=%hu\n", i,
			ftf->family_name,
			ftf->style_name,
			ftf->charmaps[i]->encoding >>24,
		   (ftf->charmaps[i]->encoding >>16 ) & 0xff,
		   (ftf->charmaps[i]->encoding >>8) & 0xff,
		   (ftf->charmaps[i]->encoding) & 0xff,
			ftf->charmaps[i]->platform_id,
			ftf->charmaps[i]->encoding_id
		);
	}
}

/*  See http://www.microsoft.com/typography/otspec/name.htm
	for a list of some possible platform-encoding pairs.
	We're interested in 0-3 aka 3-1 - UCS-2.
	Otherwise, fail. If a font has some unicode map, but lacks
	UCS-2 - it is a broken or irrelevant font. What exactly
	Freetype will select on face load (it promises most wide
	unicode, and if that will be slower that UCS-2 - left as
	an excercise to check. */
static
int force_ucs2_charmap(FT_Face ftf) {
	for(int i = 0; i < ftf->num_charmaps; i++)
		if ((  (ftf->charmaps[i]->platform_id == 0)
			&& (ftf->charmaps[i]->encoding_id == 3))
		   || ((ftf->charmaps[i]->platform_id == 3)
			&& (ftf->charmaps[i]->encoding_id == 1)))
				return FT_Set_Charmap(ftf, ftf->charmaps[i]);
	return -1;
}

static
void hline(dma2d_pixel_buffer_t *s, int min_x, int max_x, int y, uint8_t color) {
	if ((y<0)||(y>(int)s->height-1)) return;
	if (min_x<0) { min_x=0; } else if (min_x>(int)s->width-1) { min_x=(int)s->width-1; }
	if (max_x<0) { max_x=0; } else if (max_x>(int)s->width-1) { max_x=(int)s->width-1; }
	if (max_x<=min_x) return;

	uint8_t *pix = (uint8_t *)s->buffer + y * s->width + min_x;
	uint8_t *end = (uint8_t *)s->buffer + y * s->width + max_x;

	while (pix - 1 != end) {
		*pix++ = color;
	}
}

static
void vline(dma2d_pixel_buffer_t *s, int min_y, int max_y, int x, uint8_t color) {
	if ((x<0)||(x>(int)s->width-1)) return;
	if (min_y<0) { min_y=0; } else if (min_y>(int)s->height-1) { min_y=(int)s->height-1; }
	if (max_y<0) { max_y=0; } else if (max_y>(int)s->height-1) { max_y=(int)s->height-1; }
	if (max_y<=min_y) return;

	uint8_t *pix = (uint8_t *)s->buffer + min_y * s->width + x;
	uint8_t *end = (uint8_t *)s->buffer + max_y * s->width + x;

	while (pix - s->width != end) {
		*pix = color;
		pix += s->width;
	}
}

static
int harfbuzz_testx(
		dma2d_pixel_buffer_t *surface,
		int ptSize, int device_hdpi, int device_vdpi,
		FT_Library ft_library,
		const uint8_t font[], size_t font_size,
		const char *text,
		const int direction, const char *language, const hb_script_t script
	);

int harfbuzz_test(dma2d_pixel_buffer_t *surface) {
   /* Clear our surface */
	memset((uint8_t *)surface->buffer, 0, surface->width*surface->height);

	/* Font size and DPI */
	int ptSize = 25*64;
	int device_hdpi = 72;
	int device_vdpi = 72;

	assert(surface->in.pixel.format==DMA2D_xPFCCR_CM_A8);
	assert(surface->in.pixel.bitsize==8);

	/* Init freetype */
	FT_Library ft_library;
	assert(!FT_Init_FreeType(&ft_library));

	assert(!harfbuzz_testx(
			surface, ptSize, device_hdpi, device_vdpi,
			ft_library,
			DejaVuSerif_ttf,DejaVuSerif_ttf_size,
			"Ленивый рыжий кот-hâãä?fi",
			HB_DIRECTION_LTR, "en", HB_SCRIPT_LATIN
		));
	assert(!harfbuzz_testx(
			surface, ptSize, device_hdpi, device_vdpi,
			ft_library,
			amiri_regular_ttf,amiri_regular_ttf_size,
			"كسول الزنجبيل القط",
			HB_DIRECTION_RTL, "ar", HB_SCRIPT_ARABIC
		));
	assert(!harfbuzz_testx(
			surface, ptSize, device_hdpi, device_vdpi,
			ft_library,
			fireflysung_nano_ttf,fireflysung_nano_ttf_size,
			"懶惰的姜貓",
			HB_DIRECTION_TTB, "cn", HB_SCRIPT_HAN
		));

	/* Cleanup */
	FT_Done_FreeType(ft_library);

//    /* Write picture */
//    FILE *F = fopen("test.pbm","w");
//	fprintf(F, "P2\n");
//	fprintf(F, "%u %u\n", map->w,map->h);
//	fprintf(F, "%u\n", 255);
//	uint8_t *p = map->pixels;
//	for (int16_t y=0; y<map->h; y++) {
//		for (int16_t x=0; x<map->w; x++) {
//			fprintf(F, "%3u ", 255-*p++);
//		}
//		fprintf(F, "\n");
//	}

	return 0;
}

int harfbuzz_testx(
		dma2d_pixel_buffer_t *surface,
		int ptSize, int device_hdpi, int device_vdpi,
		FT_Library ft_library,
		const uint8_t font[], size_t font_size,
		const char *text,
		const int direction, const char *language, const hb_script_t script
) {

	/* Load our fonts */
	FT_Face ft_face;
	assert(!FT_New_Memory_Face(ft_library, font,font_size, 0, &ft_face));
	assert(!FT_Set_Char_Size(ft_face, 0, ptSize, device_hdpi, device_vdpi ));
	ftfdump(ft_face); // wonderful world of encodings ...
	force_ucs2_charmap(ft_face); // which we ignore.

	/* Get our harfbuzz font structs */
	hb_font_t *hb_ft_font;
	hb_ft_font = hb_ft_font_create(ft_face, NULL);

	/* Create a buffer for harfbuzz to use */
	hb_buffer_t *buf = hb_buffer_create();

	/* Our main event/draw loop */
//    int resized = 1;
	hb_buffer_set_direction(buf, direction); /* or LTR */
	hb_buffer_set_script(buf, script); /* see hb-unicode.h */
	hb_buffer_set_language(buf, hb_language_from_string(language, strlen(language)));

	/* Layout the text */
	hb_buffer_add_utf8(buf, text, strlen(text), 0, strlen(text));
	hb_shape(hb_ft_font, buf, NULL, 0);

	unsigned int         glyph_count;
	hb_glyph_info_t     *glyph_info   = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t *glyph_pos    = hb_buffer_get_glyph_positions(buf, &glyph_count);

	/* set up rendering via spanners */
	spanner_baton_t stuffbaton;

//	/* set up rendering via buffer */
//	assert((surface->in.pixel.format == DMA2D_xPFCCR_CM_A8)||(surface->in.pixel.format == DMA2D_xPFCCR_CM_A4));
//	const FT_Bitmap bitmap = surface->in.pixel.format == DMA2D_xPFCCR_CM_A8 ? (FT_Bitmap){
//			.rows = surface->height,
//			.width = surface->width,
//			.pitch = surface->width * 1,
//			.buffer = surface->buffer,
//			.num_grays = 256,
//			.pixel_mode = FT_PIXEL_MODE_GRAY,
//			.palette_mode = 0, .palette = 0
//		} : (FT_Bitmap){
//			.rows = surface->height,
//			.width = surface->width,
//			.pitch = surface->width / 2,
//			.buffer = surface->buffer,
//			.num_grays = 16,
//			.pixel_mode = FT_PIXEL_MODE_GRAY4,
//			.palette_mode = 0, .palette = 0
//		};

	FT_Raster_Params ftr_params;
	ftr_params.target = 0;
	ftr_params.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA;
//	ftr_params.source = NULL;
//	ftr_params.target = &bitmap;
////	ftr_params.flags = FT_RASTER_FLAG_DEFAULT;
//	ftr_params.flags = FT_RASTER_FLAG_AA; // | FT_RASTER_FLAG_CLIP;
	ftr_params.user = &stuffbaton;
	ftr_params.bit_set = 0;
	ftr_params.bit_test = 0;
	ftr_params.clip_box = (FT_BBox){ .xMin=0, .yMin=0, .xMax=(FT_Pos)surface->width, .yMax=(FT_Pos)surface->height };

	/* Calculate string bounding box in pixels */
	ftr_params.black_spans = NULL;
//	ftr_params.gray_spans = NULL;
	ftr_params.gray_spans = spanner_sizer;

	/* See http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html */

	int16_t max_x = INT16_MIN; // largest coordinate a pixel has been set at, or the pen was advanced to.
	int16_t min_x = INT16_MAX; // smallest coordinate a pixel has been set at, or the pen was advanced to.
	int16_t max_y = INT16_MIN; // this is max topside bearing along the string.
	int16_t min_y = INT16_MAX; // this is max value of (height - topbearing) along the string.
	/*  Naturally, the above comments swap their meaning between horizontal and vertical scripts,
		since the pen changes the axis it is advanced along.
		However, their differences still make up the bounding box for the string.
		Also note that all this is in FT coordinate system where y axis points upwards.
	 */

	int sizer_x = 0;
	int sizer_y = 0; /* in FT coordinate system. */

	FT_Error fterr;
	for (unsigned j = 0; j < glyph_count; ++j) {
		if (glyph_info[j].codepoint==13) printf("Newline\n");
		if ((fterr = FT_Load_Glyph(ft_face, glyph_info[j].codepoint, 0))) {
			printf("load %08"PRIx32" failed fterr=%d.\n",  glyph_info[j].codepoint, fterr);
		} else {
			if (ft_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
				printf("glyph->format = %4s\n", (char *)&ft_face->glyph->format);
			} else {
				int gx = sizer_x + (glyph_pos[j].x_offset/64);
				int gy = sizer_y + (glyph_pos[j].y_offset/64); // note how the sign differs from the rendering pass

				stuffbaton.min_span_x = INT16_MAX;
				stuffbaton.max_span_x = INT16_MIN;
				stuffbaton.min_y = INT16_MAX;
				stuffbaton.max_y = INT16_MIN;

				if ((fterr = FT_Outline_Render(ft_library, &ft_face->glyph->outline, &ftr_params)))
					printf("FT_Outline_Render() failed err=%d\n", fterr);

				if (stuffbaton.min_span_x != INT16_MAX) {
				/* Update values if the spanner was actually called. */
					if (min_x > stuffbaton.min_span_x + gx)
						min_x = stuffbaton.min_span_x + gx;

					if (max_x < stuffbaton.max_span_x + gx)
						max_x = stuffbaton.max_span_x + gx;

					if (min_y > stuffbaton.min_y + gy)
						min_y = stuffbaton.min_y + gy;

					if (max_y < stuffbaton.max_y + gy)
						max_y = stuffbaton.max_y + gy;
				} else {
				/* The spanner wasn't called at all - an empty glyph, like space. */
					if (min_x > gx) min_x = gx;
					if (max_x < gx) max_x = gx;
					if (min_y > gy) min_y = gy;
					if (max_y < gy) max_y = gy;
				}
			}
		}

		sizer_x += glyph_pos[j].x_advance/64;
		sizer_y += glyph_pos[j].y_advance/64; // note how the sign differs from the rendering pass
	}
	/* Still have to take into account last glyph's advance. Or not? */
	if (min_x > sizer_x) min_x = sizer_x;
	if (max_x < sizer_x) max_x = sizer_x;
	if (min_y > sizer_y) min_y = sizer_y;
	if (max_y < sizer_y) max_y = sizer_y;

	/* The bounding box */
	int bbox_w = max_x - min_x;
	int bbox_h = max_y - min_y;

	/* Two offsets below position the bounding box with respect to the 'origin',
		which is sort of origin of string's first glyph.

		baseline_offset - offset perpendecular to the baseline to the topmost (horizontal),
						  or leftmost (vertical) pixel drawn.

		baseline_shift  - offset along the baseline, from the first drawn glyph's origin
						  to the leftmost (horizontal), or topmost (vertical) pixel drawn.

		Thus those offsets allow positioning the bounding box to fit the rendered string,
		as they are in fact offsets from the point given to the renderer, to the top left
		corner of the bounding box.

		NB: baseline is defined as y==0 for horizontal and x==0 for vertical scripts.
		(0,0) here is where the first glyph's origin ended up after shaping, not taking
		into account glyph_pos[0].xy_offset (yeah, my head hurts too).
	*/

	int baseline_offset;
	int baseline_shift;

	if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(buf))) {
		baseline_offset = max_y;
		baseline_shift  = min_x;
	} else
	if (HB_DIRECTION_IS_VERTICAL(hb_buffer_get_direction(buf))) {
		baseline_offset = min_x;
		baseline_shift  = max_y;
	} else {
		assert("Invalid direction" && 0);
		baseline_offset = baseline_shift  = 0;
	}

	printf("ex '%s' string min_x=%d max_x=%d min_y=%d max_y=%d bbox %dx%d boffs %d,%d\n",
		language, min_x, max_x, min_y, max_y, bbox_w, bbox_h, baseline_offset, baseline_shift);

	/* The pen/baseline start coordinates in window coordinate system
		- with those text placement in the window is controlled.
		- note that for RTL scripts pen still goes LTR */
	int x, y;
	switch (direction) {
		case HB_DIRECTION_LTR:
			/* left justify */
			x = 20;
			y = 50;
			break;
		case HB_DIRECTION_RTL:
			/* right justify */
			x = surface->width - bbox_w - 20;
			y = 50 + ptSize/64;
			break;
		case HB_DIRECTION_TTB:
		case HB_DIRECTION_BTT:
			/* center, and for TTB script h_advance is half-width. */
			x = surface->width/2 - bbox_w/2;
			y = 50 + 2*ptSize/64;
			break;
		default :
			assert(0);
			x=y=0;
			break;
	}

	/* Draw baseline and the bounding box */
	/* The below is complicated since we simultaneously
	   convert to the window coordinate system. */
	int left, right, top, bottom;

	if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(buf))) {
		/* bounding box in window coordinates without offsets */
		left   = x;
		right  = x + bbox_w;
		top    = y - bbox_h;
		bottom = y;

		/* apply offsets */
		left   +=  baseline_shift;
		right  +=  baseline_shift;
		top    -=  baseline_offset - bbox_h;
		bottom -=  baseline_offset - bbox_h;

		/* draw the baseline */
		hline(surface, x, x + bbox_w, y, 0xff);
	} else
	if (HB_DIRECTION_IS_VERTICAL(hb_buffer_get_direction(buf))) {
		left   = x;
		right  = x + bbox_w;
		top    = y;
		bottom = y + bbox_h;

		left   += baseline_offset;
		right  += baseline_offset;
		top    -= baseline_shift;
		bottom -= baseline_shift;

		vline(surface, y, y + bbox_h, x, 0xff);
	} else {
		assert("Invalid direction" && 0);
		left = right = top = bottom = 0;
	}
	printf("ex '%s' origin %d,%d bbox l=%d r=%d t=%d b=%d\n",
			language, x, y, left, right, top, bottom);

	/* +1/-1 are for the bbox borders be the next pixel outside the bbox itself */
	hline(surface, left - 1, right + 1, top - 1, 0xff);
	hline(surface, left - 1, right + 1, bottom + 1, 0xff);
	vline(surface, top - 1, bottom + 1, left - 1, 0xff);
	vline(surface, top - 1, bottom + 1, right + 1, 0xff);

	/* set rendering spanner */
	ftr_params.gray_spans = spanner;

	/* initialize rendering part of the baton */
	stuffbaton.pixels = NULL;
	stuffbaton.first_pixel = (uint8_t *)surface->buffer;
	stuffbaton.last_pixel  = (uint8_t *)surface->buffer + surface->width*surface->height;
	stuffbaton.pitch = surface->width * sizeof(uint8_t);

	/* render */
	for (unsigned j=0; (j < glyph_count)&&(x<(int)surface->width)&&(y<(int)surface->height); ++j) {
		if ((fterr = FT_Load_Glyph(ft_face, glyph_info[j].codepoint, 0))) {
			printf("load %08"PRIx32" failed fterr=%d.\n",  glyph_info[j].codepoint, fterr);
		} else {
			if (ft_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
				printf("glyph->format = %4s\n", (char *)&ft_face->glyph->format);
			} else {
				int gx = x + (glyph_pos[j].x_offset/64);
				int gy = y - (glyph_pos[j].y_offset/64);

				stuffbaton.pixels = (uint8_t *)surface->buffer + gy * surface->width + gx;

				if ((fterr = FT_Outline_Render(ft_library, &ft_face->glyph->outline, &ftr_params)))
					printf("FT_Outline_Render() failed err=%d\n", fterr);
			}
		}

		x += glyph_pos[j].x_advance/64;
		y -= glyph_pos[j].y_advance/64;
	}

	/* clean up the buffer, but don't kill it just yet */
	hb_buffer_clear_contents(buf);


	/* Cleanup */
	hb_font_destroy(hb_ft_font);
	hb_buffer_destroy(buf);

	return 0;
}
