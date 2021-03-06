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

#include <libopencmsis/core_cm3.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include "support/system.h"
#include <support/clock.h>
#include "drivers/sdram.h"
#include "drivers/display.h"

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
static void update_led_counter(void);

/**
 * Interrupts
 */

/* Blue button interrupt (EXTI makes not too much sense here :)) */
static uint32_t mode = 3;
static bool blue_button_state_changed = false;
void
exti0_isr()
{
	exti_reset_request(EXTI0);
	blue_button_state_changed = true;
	update_led_counter();

	if (BUTTON_BLUE_PRESSED()) {
		mode = (mode+1)%4;
	}
}


/**
 * Functions
 */
void update_led_counter() {
	/* little grey code blinker */
	static const uint32_t  gcl[] = {0,1,3,2,6,7,5,4};
	static const uint32_t *gc    = gcl;
	static const uint32_t *gcle  = gcl+sizeof(gcl)/sizeof(gcl[0])-1;
	if (blue_button_state_changed) {
		blue_button_state_changed = false;
		gc = gcl;
	}
	if (BUTTON_BLUE_PRESSED()) {
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
}

#include <stdlib.h>
#include "gfx/gfx.h"
//#include "drivers/dsi_helper_functions.h"
//void *align_pointer(uint32_t alignment, void *pointer);
//void *align_pointer(uint32_t alignment, void *pointer) {
//	return (void *)align_up_to(alignment, (uint32_t)pointer);
//}
#define SDRAM_SIZE  (0x1000000U/4) // only the first bank accessible?

#define DMA2D_SIMPLE

#ifndef DMA2D_SIMPLE
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

/**
 * Main loop
 */
int main(void)
{
	/* init timers. */
	clock_setup();
	/* setup pins */
	pin_setup();
	/* low level setup */
	system_setup();
	/* setup sdram */
	sdram_init();
	uint64_t ram_clear_time = mtime();
	memset((void *)SDRAM1_BASE_ADDRESS, 0x00, SDRAM_SIZE);
	ram_clear_time = mtime() - ram_clear_time;
	/* setup tft */
//	uint8_t *video_data = malloc(2 * 800*480*4+1024);
//	uint32_t (*layers)[800][480] = align_pointer(1024, video_data);
	// the whole 1st rambank is reserved for video
	uint32_t (*layers)[480][800] = (void*)SDRAM1_BASE_ADDRESS;
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

#ifdef DMA2D_SIMPLE
	/*
	 * DMA2D debugging
	 */
	display_ltdc_config_begin();
	display_ltdc_config_layer(DISPLAY_LAYER_1, false);
//	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_1, 0,0,100,100);
	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 2,2,796,476);
	display_ltdc_config_end();

	/* Give ltdc time to update its shadow registers! */
	while (!display_ltdc_config_ready());
	display_update();

	/* Read back the window settings.. */
	dma2d_pixel_buffer_t pxsrc_fg, pxsrc_bg, pxdst, pxdst_layer1;
	display_ltdc_config_begin();
	display_ltdc_set_background_color(0xff,0xff,0xff);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_1, &pxdst_layer1);
	dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &pxdst);
	display_ltdc_config_end();

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

#define REFRESH_RATE 1000
	uint64_t timeout = mtime();
	while (1) {
		if (display_ready()) {
			uint64_t time = mtime();
			if (timeout<=time) {
				timeout += 1000/REFRESH_RATE;
				if (timeout<=time) timeout = time + 1000/REFRESH_RATE;

#define FPS_SAMPLE_TIME 1000
				static double fps = 0;
				static uint64_t fps_timeout = FPS_SAMPLE_TIME;
				static uint32_t c = 0;
				c++;
				if (fps_timeout<=time) {
					fps = (double)(1000*c)/(time+FPS_SAMPLE_TIME-fps_timeout);
					c = 0;
					fps_timeout = time+FPS_SAMPLE_TIME;
				}

				update_led_counter();

				int16_t cx,cy,w2,h2;
				cx = rand()/(RAND_MAX/(pxdst.width/2-6-20+1)); // +1 because we are rounding down
				cy = rand()/(RAND_MAX/(pxdst.height-8-20+1));
				w2 = 5+rand()/(RAND_MAX/(5+1));
				h2 = 5+rand()/(RAND_MAX/(5+1));
				dma2d_fill(
						&pxdst,
						//0xff000000,
						0xff000000|rand()/(RAND_MAX/0xffffff),
						4+10+cx-w2,4+10+cy-h2, w2*2,h2*2
					);

				int16_t sx_fg,sy_fg, sx_bg,sy_bg, dx,dy, w,h;
				static double angle = 0;
				double d_dx,d_dy;
				angle += 1.0/180*M_PI;
				d_dx = 50*sin(angle);
				d_dy = 60*cos(angle);

				pxsrc_fg.in.alpha_mod.mode  = DMA2D_xPFCCR_AM_PRODUCT;
				pxsrc_fg.in.alpha_mod.alpha = 0x22;
//				pxsrc_fg.in.alpha_mod.alpha = (uint8_t)lround((sin(angle*M_PI)+1)/2 * 0x66);// + 0x11;
				pxdst.in.alpha_mod.mode  = DMA2D_xPFCCR_AM_NONE;
//				pxdst.in.alpha_mod.alpha = 0xff;

				sx_fg=4+50;
				sy_fg=4+60;
				sx_bg=pxdst.width/2+2+50-(int16_t)lround(d_dx);
				sy_bg=4+60-(int16_t)lround(d_dy);
				dx=pxdst.width/2+2+50+(int16_t)lround(d_dx);
				dy=4+60+(int16_t)lround(d_dy);
				w=pxdst.width/2-100-6;
				h=pxdst.height-120-8;

				switch (mode) {
					case 0:
						dma2d_copy(&pxsrc_fg,&pxdst,sx_fg,sy_fg,dx,dy,w,h);
						break;
					case 1:
						pxsrc_fg.in.alpha_mod.alpha = 0xff;
						dma2d_convert_copy(&pxsrc_fg,&pxdst,sx_fg,sy_fg,dx,dy,w,h);
						break;
					case 2:
						dma2d_convert_blenddst(&pxsrc_fg,&pxdst,sx_fg,sy_fg,dx,dy,w,h);
						break;
					case 3:
						dma2d_convert_blend_copy(&pxsrc_fg,&pxsrc_bg,&pxdst,sx_fg,sy_fg,sx_bg,sy_bg,dx,dy,w,h);
						break;
				}

				char buf[256];
				sprintf(buf, "% 7.03f fps", fps);
				//gfx_set_font_scale(2);
//#define font font_Tamsyn10x20b_20
#define font font_DejaVuSansMono_36
				dma2d_fill(&pxdst, 0, 10,10,strnlen(buf,256)*font.charwidth,font.lineheight);
//				dma2d_wait_complete();
				gfx_set_font_blending(false);
				gfx_puts2(10,10,buf,&font,(gfx_color_t){.argb8888.c=0x000000});

				display_update();
//				while(1);
			}
		}
	}

#else
	/*
	 * Windowing
	 */

	/* Ram is too slow for 2 layers in 60Hz video mode (see wait_cycles below) */
	if (display_get_dsi_mode()!=DSI_MODE_ADAPTED_COMMAND_MODE) {
		while (!display_ltdc_config_ready());
		display_ltdc_config_begin();
		display_ltdc_config_layer(DISPLAY_LAYER_1, false);
		display_ltdc_config_end();
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
	display_ltdc_config_begin();
	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_1, x,y, w,h);
	display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 800-w-x,480-h-y, w,h);
	/* Ram is fast enough for two small windowed layers */
	display_ltdc_config_layer(DISPLAY_LAYER_1, true);
	display_ltdc_config_end();

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

					display_ltdc_config_begin();
					display_ltdc_config_windowing_xywh(DISPLAY_LAYER_1, x,y, w,h);
					display_ltdc_config_windowing_xywh(DISPLAY_LAYER_2, 800-w-x,480-h-y, w,h);
					display_ltdc_config_end();

					display_update();
				}
			}
		}
	}
#endif
}


