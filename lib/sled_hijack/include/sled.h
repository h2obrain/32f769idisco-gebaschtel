#include <stdint.h>
#include <stdlib.h>

#ifndef SLED_H
#define SLED_H

typedef struct {
	const char *name;
	int  (*init)(uint32_t modno);
	void (*reset)(uint32_t modno);
	int  (*draw)(uint32_t modno);
	void (*deinit)(uint32_t modno);
} module_t;

#include "../modules/sled_modules.h"
#include "../src/matrix.h"

#endif /* SLED_H */