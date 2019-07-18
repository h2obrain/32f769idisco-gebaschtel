/*
 * harfbuzz_test.h
 *
 *  Created on: 23 Jun 2019
 *      Author: h2obrain
 */

#ifndef HARFBUZZ_TEST_H_
#define HARFBUZZ_TEST_H_

#include "drivers/display.h"

dma2d_pixel_buffer_t *hbftgl_test_init(void);
void hbftgl_test_loop(dma2d_pixel_buffer_t *render_surface, uint32_t color);


#endif /* HARFBUZZ_TEST_H_ */
