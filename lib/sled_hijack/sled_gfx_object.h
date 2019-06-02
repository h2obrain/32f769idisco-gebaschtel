/*
 * gfx_private_obj.h
 *
 *  Created on: May 31, 2019
 *      Author: h2obrain
 */

#include <stdlib.h>
#include "sled_gfx_private.h"

static int init_(uint32_t modno) {
	return init(modno, NULL);
}
static void reset_(uint32_t modno) {
	reset(modno);
}
static int draw_(uint32_t modno) {
	return draw(modno, 0, NULL);
}
static void deinit_(uint32_t modno) {
	deinit(modno);
}
