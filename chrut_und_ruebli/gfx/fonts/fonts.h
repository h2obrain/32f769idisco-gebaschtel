/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2016 Oliver Meier <h2obrain@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * Fonts library
 */
#ifndef FONTS_H
#define FONTS_H

#include <stdlib.h>
#include <stdint.h>

/*
 * the holds the offsets for the data relative to the default char size
 * and the data width/height
 */
typedef struct {
	int16_t x1, y1, x2, y2;
} char_data_bbox_t;

typedef struct {
			 uint32_t  utf8_value;
	 char_data_bbox_t  bbox;
	   const uint32_t *data; /* pointer to char_data_offset */
} char_t;

typedef struct {
	int16_t         fontsize;
	int16_t         lineheight;
	int16_t         ascender;
	int16_t         descender;
	int16_t         charwidth;
	uint32_t        char_count;
	/* pointer to chars array (sorted by utf8_value) */
	const char_t   *chars;
	/* pointer to chars_data array */
	const uint32_t *chars_data;
} font_t;

/* binary search for char */
static inline
const char_t*
font_get_char(uint32_t utf8_value, const font_t *font) {
	uint32_t s, e, i, j, cc;
	s = j = 0;
	e = font->char_count-1;
	while (1) {
		i = (s+e)>>1;
		cc = font->chars[i].utf8_value;
		if (utf8_value < cc) {
			e = i;
		} else
		if (utf8_value > cc) {
			s = i;
		} else {
			return &font->chars[i];
		}
		if (i == j) {
			return NULL;
		}
		j = i;
	}
}

#endif

