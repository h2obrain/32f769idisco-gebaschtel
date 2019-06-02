/*
 * jpeg.h
 *
 *  Created on: May 30, 2019
 *      Author: Oliver Meier
 */

#ifndef LIB_GFX_INCLUDE_GFX_JPEG_H_
#define LIB_GFX_INCLUDE_GFX_JPEG_H_

#include <libopencm3/stm32/rcc.h>

#define JPEG_CLOCKS RCC_JPEG, RCC_DMA2, RCC_DMA2D

void jpeg_decode(void *in, uint32_t in_size, void *out, uint32_t out_size);

#endif /* LIB_GFX_INCLUDE_GFX_JPEG_H_ */
