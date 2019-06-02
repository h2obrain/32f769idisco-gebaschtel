#include "sled_gfx_object.h"
#include "sled/src/modules/gfx_balls.c"
module_t gfx_balls = {
		.init = init_,
		.reset = reset_,
		.draw = draw_,
		.deinit = deinit_
	};
