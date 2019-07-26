/*
 * Copyright (C) 2019 Oliver Meier <h2obrain@gmail.com>
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include <libopencmsis/core_cm3.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include "support/system.h"
#include <support/clock.h>
#include "drivers/sdram.h"
#include "drivers/display.h"


#include <stdlib.h>
#include <gfx/gfx.h>
#include <gfx/jpeg.h>


//#define DMA2D_SIMPLE
//#define WINDOWING
//#define SLED
//#define HARFBUZZ
//#define HBFTGL
#define HBFTGLMKP

#ifdef SLED
#include <sled.h>
#endif

#ifdef HARFBUZZ
#include "harfbuzz_test.h"
#endif

#ifdef HBFTGL
#include "hbftgl-test.h"
#endif

#ifdef HBFTGLMKP
#include "hbftgl-markup.h"
#endif

/**
 * Debugging
 */
void panic_mode(panic_mode_data_t *data) {
	print_panic_mode_data(data);
	while (1);
}

/**
 * Setup functions
 */
#define LED_LD1 GPIOJ,GPIO13
#define LED_LD2 GPIOJ,GPIO5
#define LED_LD3 GPIOA,GPIO12
#define BUTTON_BLUE GPIOA,GPIO0
#define BUTTON_BLUE_PRESSED() gpio_get(BUTTON_BLUE)
static void pin_setup(void) {
	/* pin-clocks */
	board_setup_clocks((uint32_t[]){
		RCC_GPIOA,
		RCC_GPIOJ,
		SDRAM_32F769IDISCOVERY_CLOCKS,
		DISPLAY_32F769IDISCOVERY_CLOCKS,
		JPEG_CLOCKS,
		0
	});
	/* pins */
	board_setup_pins((pin_setup_t[]) {
		/* outputs (LEDs 1-3) */
		{ GPIOJ,
			GPIO5  | GPIO13,
			GPIO_MODE_OUTPUT , GPIO_PUPD_NONE   ,
			.out_mode={ GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ  }, .af_mode ={0}},
		{ GPIOA,
			GPIO12,
			GPIO_MODE_OUTPUT , GPIO_PUPD_NONE   ,
			.out_mode={ GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ  }, .af_mode ={0}},
		/* inputs (blue button) */
		{ GPIOA,
			GPIO0,
			GPIO_MODE_INPUT  , GPIO_PUPD_NONE   ,
			.out_mode={0}, .af_mode ={0}},
		/* sdram */
		SDRAM_32F769IDISCOVERY_PINS,
		/* tft */
		DISPLAY_32F769IDISCOVERY_PINS,
		{0}
	});

	/* setup blue/user button */
	exti_select_source(EXTI0, GPIOA);
	exti_set_trigger(EXTI0, EXTI_TRIGGER_BOTH);
	exti_enable_request(EXTI0);
	nvic_enable_irq(NVIC_EXTI0_IRQ);
}

/**
 * Function definitions
 */
uint32_t update_led_counter(void);

/**
 * Interrupts
 */

/* Blue button interrupt (EXTI makes not too much sense here :)) */
static bool blue_button_state_changed = false;
void
exti0_isr()
{
	exti_reset_request(EXTI0);
	blue_button_state_changed = true;
	update_led_counter();

#ifdef SLED
	if (BUTTON_BLUE_PRESSED()) {
		matrix_next_animation();
	}
#endif
}


/**
 * Functions
 */
uint32_t update_led_counter() {
	/* little grey code blinker */
	static const uint32_t  gcl[] = {0,1,3,2,6,7,5,4};
	static const uint32_t *gc    = gcl;
	static const uint32_t *gcle  = gcl+sizeof(gcl)/sizeof(gcl[0])-1;
	uint32_t ret = 0;
	if (blue_button_state_changed) {
		blue_button_state_changed = false;
		gc = gcl;
		ret++;
	}
	if (BUTTON_BLUE_PRESSED()) {
		ret++;
		gpio_set(LED_LD1);
		gpio_set(LED_LD2);
		gpio_set(LED_LD3);
	} else {
		if (*gc&0b001) gpio_set(LED_LD1);
		else           gpio_clear(LED_LD1);
		if (*gc&0b010) gpio_set(LED_LD2);
		else           gpio_clear(LED_LD2);
		if (*gc&0b100) gpio_set(LED_LD3);
		else           gpio_clear(LED_LD3);
		if (gc==gcle) gc = gcl;
		else          gc++;
	}
	return ret;
}

//#include "drivers/dsi_helper_functions.h"
//void *align_pointer(uint32_t alignment, void *pointer);
//void *align_pointer(uint32_t alignment, void *pointer) {
//	return (void *)align_up_to(alignment, (uint32_t)pointer);
//}
#define SDRAM_SIZE  (0x1000000U/4) // only the first bank accessible?

#ifdef DMA2D_SIMPLE
#include "data/gebaschtel_jpg.h"
uint8_t gebaschtel_out[100*100*4]={0};
#endif

#ifdef WINDOWING
static inline
int16_t bounce_add(int16_t v,int16_t *d,int16_t w) {
	v += *d;
	if (*d>0) {
		if (v>w) {
			*d = -*d;
			v  = w*2-v;
		}
	} else {
		if (v<0) {
			*d = -*d;
			v  = -v;
		}
	}
	return v;
}
#endif


#include <libopencm3/cm3/mpu.h>
void st_setup(void);

static
void preinit(void) {
	/* init timers. */
	clock_setup();
	/* setup pins */
	pin_setup();
	/* low level setup */
	system_setup();
	/* setup mpu to enable unaligned memory accesses in external sdram */
	asm("dmb");
	MPU_RNR  = 0; /* config region 0 */
	uint32_t rasr = 0;
	rasr |= MPU_RASR_ATTR_XN; /* instruction fetches/access disable */
	rasr |= MPU_RASR_ATTR_AP_PRW_URW; /* full access */
	rasr |= 1 << 19; /* TEX level 1 */
//	rasr |= MPU_RASR_ATTR_C; /* cacheable */
//	rasr |= MPU_RASR_ATTR_B; /* bufferable */
//	rasr |= MPU_RASR_ATTR_S; /* shareable */
//	rasr |= (x&0xff) << MPU_RASR_SRD_LSB; /* subregion disable */
	uint32_t n = 24; /* protection area size = 2^(size+1) = 16MB */
	rasr |= ((n-1)&0x1f)<<MPU_RASR_SIZE_LSB;
	rasr |= MPU_RASR_ENABLE; /* enable region */
	MPU_RASR = rasr;
	uint32_t addr_mask = 0;
	for (uint32_t i = n; i<32; i++) {
		addr_mask |= 1<<i;
	}
//	MPU_RBAR = ((SDRAM1_BASE_ADDRESS&addr_mask) << n) | MPU_RBAR_VALID; /* set address */
	MPU_RBAR = (SDRAM1_BASE_ADDRESS&addr_mask); /* set address */
	// MPU_CTRL_PRIVDEFENA /* */
	// MPU_CTRL_HFNMIENA   /* enable mpu during hard-fault */
	MPU_CTRL = MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE; /* enable mpu */
	SCB_SHCSR |= SCB_SHCSR_MEMFAULTENA; /* enable exceptions */
	/* ensure MPU setting take effects */
	asm("dsb");
	asm("isb");

	msleep(1);

	/* setup sdram */
//	st_setup();
//	sdram_init();
	sdram_init_custom(
			SDRAM_BANK1,
			32, 12,8,
			(struct sdram_timing) {
				.trcd = 2,  /* RCD Delay */
				.trp  = 2,  /* RP Delay */
				.twr  = 2,  /* Write Recovery Time */
				.trc  = 7,  /* Row Cycle Delay */
				.tras = 4,  /* Self Refresh Time */
				.txsr = 7,  /* Exit Self Refresh Time */
				.tmrd = 2,  /* Load to Active Delay */
			},
			3,
			true, false, 1
		);

	uint64_t ram_clear_time = mtime();
	memset((void *)SDRAM1_BASE_ADDRESS, 0x00, SDRAM_SIZE);
	ram_clear_time = mtime() - ram_clear_time;
}

__attribute__((section(".preinit_array"),externally_visible))
void (*app_preinit_array[])(void) = { &preinit };


/**
 * Main loop
 */
int main(void)
{
#if defined(VIDEO_MEMORY_ADDRESS)
	assert(VIDEO_MEMORY_SIZE == 2*800*480*sizeof(uint32_t));
	uint32_t (*layers)[480][800] = (void*)VIDEO_MEMORY_ADDRESS;
#elif defined(BETTER_LD_VERSION) || (defined(MALLOC_AREA_START) && defined(MALLOC_AREA_SIZE))
	uint32_t (*layers)[480][800] = malloc(2*800*480*sizeof(uint32_t));
#else
	uint32_t (*layers)[480][800] = (void*)SDRAM1_BASE_ADDRESS;
#endif

//#ifdef HARFBUZZ
//	srand(systick_get_value());
//	dma2d_pixel_buffer_t hb_buf = {
//		.buffer = malloc(800*480),
//		.width  = 800,
//		.height = 480,
//		.in.pixel.bitsize = 8,
//		.in.pixel.format  = DMA2D_xPFCCR_CM_A8,
//		.in.pixel.alpha_mode.color = 0xffffff,
//		.out.pixel.bytesize = 1,// out is not supported!
//		.out.pixel.format = 0,
//	};
//	harfbuzz_test(&hb_buf);
//#endif

	/* setup tft */
//	uint8_t *video_data = malloc(2 * 800*480*4+1024);
//	uint32_t (*layers)[800][480] = align_pointer(1024, video_data);
	// the whole 1st rambank is reserved for video
	display_init(
//			DSI_MODE_VIDEO_BURST,
//			DSI_MODE_VIDEO_SYNC_PULSES,
//			DSI_MODE_VIDEO_SYNC_EVENTS,
//			DSI_MODE_VIDEO_PATTERN_BER,
//			DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL,
//			DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL,
			DSI_MODE_ADAPTED_COMMAND_MODE,

			DISPLAY_COLOR_CODING_ARGB8888,

			DISPLAY_ORIENTATION_LANDSCAPE,
//			DISPLAY_ORIENTATION_PORTRAIT,

			(uint8_t*)layers[0],
			(uint8_t*)layers[1]
		);

	/* very verbose pixel set (all data) */
//	uint32_t alpha = 0xff000000;
//	for (uint32_t l = 0; l<2; l++) {
//		for (uint32_t x=0; x<800; x++) {
//			for (uint32_t y=0; y<480; y++) {
//				layers[l][y][x] = alpha | (0xff<<(l*8)); // ARGB
////				wait_cycles(10000); // give the ram some time to breath
////				wait_cycles(100000); // give the ram some time to breath
//			}
//		}
//		alpha = (alpha / 2 + 1) & 0xff000000;
//	}

#ifdef HBFTGLMKP
	display_ltdc_config_access_begin();
	display_ltdc_config_layer(DISPLAY_LAYER_1, false);
	//display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 0,0,640,480);
	display_ltdc_config_access_end();
	/* Give ltdc time to update its shadow registers! */
	while (!display_ltdc_config_ready());
	display_update();

	/* Read back the window settings.. */
	dma2d_pixel_buffer_t pxdst;
	display_ltdc_config_access_begin();
	display_ltdc_set_background_color(0x22,0x22,0x22);
//	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &pxdst_layer1);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &pxdst);
	display_ltdc_config_access_end();

	gfx_init(layers[1], pxdst.width,pxdst.height);

	dma2d_fill(
			&pxdst,
			0, //0xffffffff,
			0,0, pxdst.width,pxdst.height
		);
	while (!display_ltdc_config_ready());
	display_update();

	hbftgl_markup_init(&pxdst);
#define REFRESH_RATE 30
	uint64_t timeout = mtime();
	while (1) {
		if (display_ready()) {
			uint64_t time = mtime();
			if (timeout<=time) {
				timeout += 1000/REFRESH_RATE;
				if (timeout<=time) timeout = time + 1000/REFRESH_RATE;
				update_led_counter();
//				dma2d_fill(&pxdst, 0, 0,0, pxdst.width,pxdst.height);
//				hbftgl_markup_loop(&pxdst);
				dma2d_wait_complete();
				display_update();
			}
		}
	}
#endif


#ifdef HBFTGL
	display_ltdc_config_access_begin();
	display_ltdc_config_layer(DISPLAY_LAYER_1, false);
	//display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 0,0,640,480);
	display_ltdc_config_access_end();
	/* Give ltdc time to update its shadow registers! */
	while (!display_ltdc_config_ready());
	display_update();

	/* Read back the window settings.. */
	dma2d_pixel_buffer_t pxdst;
	display_ltdc_config_access_begin();
	display_ltdc_set_background_color(0x22,0x22,0x22);
//	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &pxdst_layer1);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &pxdst);
	display_ltdc_config_access_end();

	gfx_init(layers[1], pxdst.width,pxdst.height);

	dma2d_fill(
			&pxdst,
			0, //0xffffffff,
			0,0, pxdst.width,pxdst.height
		);
	while (!display_ltdc_config_ready());
	display_update();

	hbftgl_test_init();
#define REFRESH_RATE 100
	uint64_t timeout = mtime();
	while (1) {
		if (display_ready()) {
			uint64_t time = mtime();
			if (timeout<=time) {
				timeout += 1000/REFRESH_RATE;
				if (timeout<=time) timeout = time + 1000/REFRESH_RATE;
				update_led_counter();
				static uint32_t c = 0;
				static const uint32_t col[] = {
					0xffc5d9,
					0xc2f2d0,
					0xfdf5c9,
					0xffcb85,
					0x6b3e26
				};
//				if (!c) {
				dma2d_fill(&pxdst, 0, 0,0, pxdst.width,pxdst.height);
				hbftgl_test_loop(&pxdst, col[c]);
				dma2d_wait_complete();
//				}
				if (c==sizeof(col)/sizeof(col[0])-1) {c=0;} else {c++;}
				display_update();
			}
		}
	}
#endif


#ifdef HARFBUZZ
	display_ltdc_config_access_begin();
	display_ltdc_config_layer(DISPLAY_LAYER_1, false);
	//display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 0,0,640,480);
	display_ltdc_config_access_end();
	/* Give ltdc time to update its shadow registers! */
	while (!display_ltdc_config_ready());
	display_update();

	/* Read back the window settings.. */
	dma2d_pixel_buffer_t pxdst;
	display_ltdc_config_access_begin();
	display_ltdc_set_background_color(0x22,0x22,0x22);
//	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &pxdst_layer1);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &pxdst);
	display_ltdc_config_access_end();

	gfx_init(layers[1], pxdst.width,pxdst.height);

	dma2d_fill(
			&pxdst,
			0, //0xffffffff,
			0,0, pxdst.width,pxdst.height
		);
	while (!display_ltdc_config_ready());
	display_update();

	dma2d_pixel_buffer_t hb_buf = {
		.buffer = malloc(pxdst.width*pxdst.height),
		.width  = pxdst.width,
		.height = pxdst.height,
		.in.pixel.bitsize = 8,
		.in.pixel.format  = DMA2D_xPFCCR_CM_A8,
		.in.pixel.alpha_mode.color = 0xffffff,
		.out.pixel.bytesize = 0,// out is not supported!
		.out.pixel.format = 0,
	};
	harfbuzz_test(&hb_buf);

//	uint32_t *ptr = pxdst.buffer;
//	for (uint32_t i=10;i<400;i++) {
//		ptr[i] = 0xffffffff;
//	}
//	gfx_draw_line(10,10,400,400,(gfx_color_t){.argb8888={.a=0xff,.r=0,.g=0xff,.b=0xff}});
	dma2d_convert_copy__no_pxsrc_fix(&hb_buf,&pxdst, 0,0, 0,0, pxdst.width,pxdst.height);

#define REFRESH_RATE 10
	uint64_t timeout = mtime();
	while (1) {
		if (display_ready()) {
			uint64_t time = mtime();
			if (timeout<=time) {
				timeout += 1000/REFRESH_RATE;
				if (timeout<=time) timeout = time + 1000/REFRESH_RATE;
				update_led_counter();
				display_update();
			}
		}
	}
#endif


#ifdef SLED
	/*
	 * DMA2D debugging
	 */
	display_ltdc_config_access_begin();
	display_ltdc_config_layer(DISPLAY_LAYER_1, false);
	//display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 0,0,640,480);
	display_ltdc_config_access_end();

	/* Give ltdc time to update its shadow registers! */
	while (!display_ltdc_config_ready());
	display_update();

	/* Read back the window settings.. */
	dma2d_pixel_buffer_t pxdst;
	display_ltdc_config_access_begin();
	display_ltdc_set_background_color(0x22,0x22,0x22);
//	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &pxdst_layer1);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &pxdst);
	display_ltdc_config_access_end();

	gfx_init(layers[1], pxdst.width,pxdst.height);

	dma2d_fill(
			&pxdst,
			0, //0xffffffff,
			0,0, pxdst.width,pxdst.height
		);
	while (!display_ltdc_config_ready());
	display_update();

	srand(systick_get_value());

	matrix_run_main_loop();

#endif

#ifdef DMA2D_SIMPLE
	/*
	 * DMA2D debugging
	 */
	display_ltdc_config_access_begin();
	display_ltdc_config_layer(DISPLAY_LAYER_1, false);
//	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_1, 0,0,100,100);
	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 2,2,796,476);
	display_ltdc_config_access_end();

	/* Give ltdc time to update its shadow registers! */
	while (!display_ltdc_config_ready());
	display_update();

	/* Read back the window settings.. */
	dma2d_pixel_buffer_t pxsrc_fg, pxsrc_bg, pxdst, pxdst_layer1;
	display_ltdc_config_access_begin();
	display_ltdc_set_background_color(0xff,0xff,0xff);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &pxdst_layer1);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &pxdst);
	display_ltdc_config_access_end();

	gfx_init(layers[1], pxdst.width,pxdst.height);


	dma2d_fill(&pxdst_layer1, 0xff0000ff, 0,0,pxdst_layer1.width+5,pxdst_layer1.height+5);

	pxsrc_fg = pxsrc_bg = pxdst;
	dma2d_fill(
			&pxdst,
			0xffff0000,
			0,0, pxdst.width,pxdst.height
		);
	dma2d_fill(
			&pxdst,
			0, //0xffffffff,
			4,4, pxdst.width/2-6,pxdst.height-8
		);
	dma2d_fill(
			&pxdst,
			0, //0xffffffff,
			pxdst.width/2+2,4, pxdst.width/2-6,pxdst.height-8
		);
//	dma2d_fill_area(
//			&layer2,
//			0xff000000|rand()/(RAND_MAX/0xffffff),
//			10,10, 10,10
//		);
	while (!display_ltdc_config_ready());
	display_update();

//	while(1) { if (display_ready()) { display_update(); } }

	srand(systick_get_value());
#endif

#ifdef WINDOWING

	/*
	 * Windowing
	 */

	/* Ram is too slow for 2 layers in 60Hz video mode (see wait_cycles below) */
	if (display_get_dsi_mode()!=DSI_MODE_ADAPTED_COMMAND_MODE) {
		while (!display_ltdc_config_ready());
		display_ltdc_config_access_begin();
		display_ltdc_config_layer(DISPLAY_LAYER_1, false);
		display_ltdc_config_access_end();
	}

	/* play with windows */
	uint32_t c=300;
	int16_t w,h,x,y,xd,yd;
	w=120;
	h=120;
	x=y=0;
	xd=3;
	yd=5;
	uint32_t (*layer1_data)[w] = (void *)layers[0];
	uint32_t (*layer2_data)[w] = (void *)layers[1];
	display_ltdc_config_access_begin();
	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_1, x,y, w,h);
	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 800-w-x,480-h-y, w,h);
	/* Ram is fast enough for two small windowed layers */
	display_ltdc_config_layer(DISPLAY_LAYER_1, true);
	display_ltdc_config_access_end();

#define REFRESH_RATE 30
	uint64_t timeout = mtime();
	while (1) {
//		layers[1][rand()/(RAND_MAX/480)][rand()/(RAND_MAX/800)] = 0xff000000|rand()/(RAND_MAX/0xffffff);
//		wait_cycles(100000); // give the ram some time to breath
		layer1_data[10+rand()/(RAND_MAX/(h-20))][10+rand()/(RAND_MAX/(w-20))] = 0xcf000000|rand()/(RAND_MAX/0xffffff);
		layer2_data[10+rand()/(RAND_MAX/(h-20))][10+rand()/(RAND_MAX/(w-20))] = 0xcf000000|rand()/(RAND_MAX/0xffffff);
//		wait_cycles(1000); // give the ram some time to breath
		if (display_ready()) {
			uint64_t time = mtime();
			if (timeout<=time) {
				timeout += 1000/REFRESH_RATE;
				if (timeout<=time) timeout = time + 1000/REFRESH_RATE;
				update_led_counter();
				if (c) {
					//c--;
//					x = 360;
//					y = 200;
					x = bounce_add(x,&xd,800-w);
					y = bounce_add(y,&yd,480-h);

					display_ltdc_config_access_begin();
					display_ltdc_config_windowing_xywh(DISPLAY_LAYER_1, x,y, w,h);
					display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 800-w-x,480-h-y, w,h);
					display_ltdc_config_access_end();

					display_update();
				}
			}
		}
	}
#endif
}


