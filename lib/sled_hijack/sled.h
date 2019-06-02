#include <stdint.h>

typedef struct {
	int  (*init)(uint32_t modno);
	void (*reset)(uint32_t modno);
	int  (*draw)(uint32_t modno);
	void (*deinit)(uint32_t modno);
} module_t;

extern module_t gfx_balls;
