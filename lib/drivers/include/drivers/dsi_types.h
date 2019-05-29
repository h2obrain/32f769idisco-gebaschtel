/*
 * dsi_types.h
 *
 *  Created on: 19 May 2019
 *      Author: h2obrain
 */

#ifndef DSI_TYPES_H_
#define DSI_TYPES_H_

typedef enum {
	DSI_MODE_VIDEO_SYNC_PULSES,
	DSI_MODE_VIDEO_SYNC_EVENTS,
	DSI_MODE_VIDEO_BURST,

	DSI_MODE_VIDEO_PATTERN_BER,
	DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL,
	DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL,

	DSI_MODE_ADAPTED_COMMAND_MODE,
} display_dsi_mode_t;
typedef enum {
	DISPLAY_COLOR_CODING_ARGB8888 = 0,
	DISPLAY_COLOR_CODING_RGB888   = 1,
	DISPLAY_COLOR_CODING_RGB565   = 2
} display_color_coding_t;
typedef enum {
	DISPLAY_ORIENTATION_PORTRAIT,
	DISPLAY_ORIENTATION_LANDSCAPE
} display_orientation_t;
/* compatible with ltdc */
typedef enum {
	DISPLAY_LAYER_1 = 1,
	DISPLAY_LAYER_2 = 2
} display_layer_t;

#endif /* DSI_TYPES_H_ */
