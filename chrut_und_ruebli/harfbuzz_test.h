/*
 * harfbuzz_test.h
 *
 *  Created on: 23 Jun 2019
 *      Author: h2obrain
 */

#ifndef HARFBUZZ_TEST_H_
#define HARFBUZZ_TEST_H_

#include "drivers/display.h"

int harfbuzz_test(
		dma2d_pixel_buffer_t *surface,
		void *DejaVuSerif,uint32_t DejaVuSerif_size,
		void *amiri_regular_ttf,uint32_t amiri_regular_ttf_size,
		void *fireflysung_ttf,uint32_t fireflysung_ttf_size
	);

#endif /* HARFBUZZ_TEST_H_ */
