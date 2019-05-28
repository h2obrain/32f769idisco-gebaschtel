/*
 * gfx_config.h
 *
 *  Created on: 27 May 2019
 *      Author: Oliver Meier
 */

#ifndef GFX_GFX_CONFIG_H_
#define GFX_GFX_CONFIG_H_

/*
 * GFX config types
 */
#define GFX_COLOR_MODE_ARGB8888   0
#define GFX_COLOR_MODE_RGB888     1
#define GFX_COLOR_MODE_RGB565     2
#define GFX_COLOR_MODE_MONOCHROME 3

/*
 * GFX config
 */

#define GFX_DEFAULT_COLOR_MODE GFX_COLOR_MODE_ARGB8888
#define GFX_DMA2D_FONTS 1

#endif /* GFX_GFX_CONFIG_H_ */
