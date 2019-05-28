/*
 * tft_impl.c
 *
 *  Created on: 21 Apr 2019
 *      Author: h2obrain
 */


#include "display.h"
#include "otm8009a.h"

#include "../support/system.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/ltdc.h>
#include <libopencm3/stm32/dsi.h>
#include "dsi_helper_functions.h"
//#include "../gfx/gfx_locm3.h"
#include "../clock.h"

static inline uint32_t display_ltdc_config(
		dsi_lpcr_flags_t signal_polarity,
		uint32_t HSA, uint32_t HBP, uint32_t HACT, uint32_t HFP,
		uint32_t VSA, uint32_t VBP, uint32_t VACT, uint32_t VFP,
		display_color_coding_t color_coding,
		bool enable_color_keying_in_rgb565_mode, uint16_t color_key,
		uint8_t layer1_frame_buffer[], uint8_t layer2_frame_buffer[]
	);

#define ENABLE_TEARING 1

void display_init(
		display_dsi_mode_t mode,
		display_color_coding_t color_coding,
		display_orientation_t orientation,
		uint8_t layer1_frame_buffer[],
		uint8_t layer2_frame_buffer[]
) {
	/* Disable interrupts */
	nvic_disable_irq(NVIC_LCD_TFT_IRQ);
	nvic_disable_irq(NVIC_LCD_TFT_ERR_IRQ);
	nvic_disable_irq(NVIC_DSIHOST_IRQ);

	/* Reset TFT */
	dsi_disable_all();
	gpio_clear(TFT_NOT_RESET_PIN);

	/* Reset LTDC/DSI */
	rcc_periph_reset_pulse(RST_LTDC);
	rcc_periph_reset_pulse(RST_DSI);

	/* Init dma2d */
	dma2d_init();

	/* Setup LTDC clock (PLLSAI)
	 * Setup Pixel Clock:
	 *  PLLSAI_VCO_Input  = HSE_CLK/PLL_M = 1 Mhz (make sure this is true!)
	 *  PLLSAI_VCO Output = PLLSAI_VCO_Input * PLLSAI_N   = 384 Mhz
	 *  PLL_LCD_CLK       = PLLSAI_VCO_Output / PLLSAI_R  = 54.857 Mhz
	 *  lcd_clk           = PLL_LCD_CLK / RCC_PLLSAIDIV_R = 54.857/2 = 27.429 Mhz
	 * Other outputs:
	 *   48MHz_clock, PLLSAI48 = 48 MHz = PLLSAI_VCO_Output / PLLSAI_P
	 *   SAI_clock  , PLLSAI   = 48 MHz = PLLSAI_VCO_Output / PLLSAI_Q / PLLSAIDIV_Q
	 */
	uint32_t pllsai_n, pllsai_r;
	switch (mode) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			/* 27.429MHz (AN4860 example for video burst mode) */
			pllsai_n = 384;
			pllsai_r = 7;
			break;
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			/* 27.429MHz (ram is fast enough so layer1/2 can be blended together) */
			pllsai_n = 384;
			pllsai_r = 7;
//			/* 38.4MHz (max with PLLSAI48 48MHz output possible) */
//			pllsai_n = 384;
//			pllsai_r = 5;
//			/* 41.7MHz (max for rgb888) (AN4860 example for adapted command mode) */
//			pllsai_n = 417;
//			pllsai_r = 5;
			break;
		default :
			assert("Unsupported video mode" && 0);
			return;
	}
	/*
	 * Fdisplay = lcd_clk / ((HSA+HBP+HACT+HFP)*(VSA+VBP+VACT+VFP))
	 */
//	uint32_t lcd_clk = (uint32_t)((uint64_t)1000000 * pllsai_n / (pllsai_r * 4));
//	rcc_pllsai_config(pllsai_n, RCC_PLLSAICFGR_PLLSAIP_DIV8, 8, pllsai_r);
//	rcc_pllsai_postscalers(0, RCC_DCKCFGR1_PLLSAIDIVR_DIVR_4);

	/* 27.429MHz / ((800+2+34+34)×(480+1+15+16)) ~= 61.6Hz */
	// 2=RCC_DCKCFGR1_PLLSAIDIVR_DIVR_2
	uint32_t lcd_clk = (uint32_t)((uint64_t)1000000 * pllsai_n / (pllsai_r * 2));
	rcc_pllsai_config(pllsai_n, RCC_PLLSAICFGR_PLLSAIP_DIV8, 8, pllsai_r);
	rcc_pllsai_postscalers(0, RCC_DCKCFGR1_PLLSAIDIVR_DIVR_2);

	rcc_pllsai_enable();
	while (!rcc_pllsai_ready());

	/*
	 * DSI host config
	 *
	 */

	/*
	 * Config DSI pll
	 *  lane_byte_clk = 62.5Mhz = HSE(25Mhz) / idf * ndiv / odf / 8
	 *
	 */
	uint32_t lane_byte_clk, tx_escape_clk;
	uint32_t idf,ndiv,odf;
	idf  = 5;
	ndiv = 100;
	odf  = 1;
	tx_escape_clk = 15625000;
	switch (mode) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			/* Config DSI PHY */
			lane_byte_clk = dsi_phy_config(
								idf,ndiv,odf,
								tx_escape_clk,
								2,
								/* clock lane config */
								DSI_CLCR_DPCC, // | DSI_CLCR_ACR
								/* LP<->HS transition times (TBD calculate those values!) */
								28,36, 18,17, 0, 0, // values taken from an4860
//								50,50, 50,50, 0, 0,
//								36,36, 36,36, 0, 10, // 10 is for dsi host, might be higher for display
								0 //DSI_PCR_BTAE
				);
			break;
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			/* Config DSI PHY */
			lane_byte_clk = dsi_phy_config(
								idf,ndiv,odf,
								tx_escape_clk,
								2,
								/* clock lane config */
								DSI_CLCR_DPCC, // | DSI_CLCR_ACR
								/* LP<->HS transition times (TBD calculate those values!) */
								/* 10 is for dsi host, might be higher for display */
								0,0, 0,0, 0, 10, // values taken from an4860
								0
//								DSI_PCR_BTAE
							);
			break;
	}

	/* Config DSI video mode */
	dsi_color_coding_t dsi_color_coding;
	dsi_lpcr_flags_t signal_polarity;
	uint32_t HSA, HBP, HACT, HFP;
	uint32_t VSA, VBP, VACT, VFP;

	// rgb888 (actually argb8888 for ltdc)
	switch (color_coding) {
		case DISPLAY_COLOR_CODING_RGB565 :
			dsi_color_coding = DSI_COLOR_CODING_RGB565;
			break;
		case DISPLAY_COLOR_CODING_RGB888 :
		case DISPLAY_COLOR_CODING_ARGB8888 :
		default :
			dsi_color_coding = DSI_COLOR_CODING_RGB888;
			break;
	}

	// h/vsync active low, data enable active high
	// this setting seems to have no influence!
	signal_polarity = DSI_LPCR_HSP | DSI_LPCR_VSP;

	// timings
	switch (orientation) {
		case DISPLAY_ORIENTATION_LANDSCAPE :
			HACT = OTM8009A_800X480_WIDTH;        /* 800 */
			VACT = OTM8009A_800X480_HEIGHT;       /* 480 */
			break;
		case DISPLAY_ORIENTATION_PORTRAIT :
			HACT = OTM8009A_480X800_WIDTH;        /* 480 */
			VACT = OTM8009A_480X800_HEIGHT;       /* 800 */
			break;
		default :
			assert("invalid tft orientation!" && 0);
			return;
	}
	/*
	 * Setup dsi mode
	 */
	switch (mode) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL : {
			HSA  = OTM8009A_480X800_HSYNC;        /* 2 */
			HBP  = OTM8009A_480X800_HBP;          /* 34 */
			HFP  = OTM8009A_480X800_HFP;          /* 34 */
			VSA  = OTM8009A_480X800_VSYNC;        /* 1 */
			VBP  = OTM8009A_480X800_VBP;          /* 15 */
			VFP  = OTM8009A_480X800_VFP;          /* 16 */
			dsi_vmcr_vmt_t video_mode_type;
			uint32_t video_packet_size, num_chunks, null_packet_size;
			switch (mode) {
				case DSI_MODE_VIDEO_SYNC_PULSES :
					/* values taken from an4860 */
					video_mode_type   = DSI_VMCR_VMT_NON_BURST_PULSE;
					video_packet_size = 200;
					num_chunks        = 4;
					null_packet_size  = 299;
					break;
				case DSI_MODE_VIDEO_SYNC_EVENTS :
					video_mode_type   = DSI_VMCR_VMT_NON_BURST_EVENT;
					video_packet_size = 200;
					num_chunks        = 4;
					null_packet_size  = 299;
					break;
				case DSI_MODE_VIDEO_BURST :
				case DSI_MODE_VIDEO_PATTERN_BER :
				case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
				case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
					/* values taken from an4860 */
					video_mode_type   = DSI_VMCR_VMT_BURST;
					video_packet_size = HACT;
					num_chunks        = 0;
					null_packet_size  = 0xfff; // 0
					break;
				default :
					break;
			}
			dsi_video_mode_config(
					/* Video transmission mode */
					video_mode_type,
					/* Video packet size (TBD calculate those values!) */
					video_packet_size,
					/* Number of chunks (TBD calculate those values!) */
					num_chunks,
					/* NULL packet size (TBD calculate those values!) */
					null_packet_size,
					/* Virtual channel ID (0 apparently) */
					0,
					/* Signal polarity of HSYNC/VSYNC/DATA_ENABLE */
					signal_polarity,
					/* Color coding */
					dsi_color_coding, false,
					/* Timings */
					lane_byte_clk, lcd_clk,
					HSA, HBP, HACT, HFP,
					VSA, VBP, VACT, VFP,
					/* High speed to low power transition config */
					true, // Enable commands in low-power mode
					DSI_VMCR_LPHFPE | DSI_VMCR_LPHBPE |
					DSI_VMCR_LPVAE  | DSI_VMCR_LPVFPE | DSI_VMCR_LPVBPE |
					DSI_VMCR_LPVSAE,
					/* Frame BTA ack disabled */
					false,
					/* largest (VACT) packet size (TBD calculate those values!) */
		//			16,0
					28,8
		//			28,0
		//			20,0
		//			64,64
				);
		}	break;

		case DSI_MODE_ADAPTED_COMMAND_MODE :
			HSA = HBP = HFP = VSA = VBP = VFP = 1;
			/* Configure DSI to adapted command mode */
			dsi_adapted_command_mode_config(
					/* Virtual channel ID (0 apparently) */
					0,
					/* Maximum command size */
					HACT,
					/* Signal polarity of HSYNC/VSYNC/DATA_ENABLE */
					signal_polarity,
					/* Color coding */
					dsi_color_coding,
#if ENABLE_TEARING
					/* Tearing signal config
					 * - te-src: external pin (needs enabling in display!)
					 * - te-pol: rising edge
					 * - vsync-pol: falling edge
					 * - auto-refresh: enable
					 */
					DSI_WCFGR_TESRC | DSI_WCFGR_AR,
					/* Enable tearing effect ack request */
					true
#else
					/* Tearing signal config
					 * - te-src: through DSI link
					 * - te-pol: rising edge
					 * - vsync-pol: falling edge
					 * - auto-refresh: disable
					 */
					0,
					/* Disable tearing effect ack request */
					false
#endif
				);

#if ENABLE_TEARING
			/* Enable the Tearing Effect interrupt */
			DSI_WIER |= DSI_WIER_TEIE;
#endif
			/* Enable the End of Refresh interrupt */
			DSI_WIER |= DSI_WIER_ERIE;
			break;
	}


	/*
	 * LTDC config
	 *
	 */

	/* Basic config */
	display_ltdc_config(
			signal_polarity,
			HSA, HBP, HACT, HFP,
			VSA, VBP, VACT, VFP,
			color_coding, false, 0,
			layer1_frame_buffer,
			layer2_frame_buffer
		);
	/* Enable the transfer Error interrupt */
	LTDC_IER |= LTDC_IER_TERRIE;
	/* Enable the FIFO underrun interrupt */
	LTDC_IER |= LTDC_IER_FUIE;


	/*
	 * Reset the display
	 *
	 */
	tft_reset();


	/*
	 * Enable LTDC, DSI host and wrapper
	 *
	 */
	switch (mode) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			/* Enable LTDC by setting LTDCEN bit */
			LTDC_GCR |= LTDC_GCR_LTDC_ENABLE;
			wait_cycles(10);
			break;
		case DSI_MODE_VIDEO_PATTERN_BER :
			/* BER pattern (comment for color bar pattern) */
			DSI_VMCR |= DSI_VMCR_PGM;
			/* enable pattern */
			DSI_VMCR |= DSI_VMCR_PGE;
			break;
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
			/* horizontal (only color bar) */
			DSI_VMCR |= DSI_VMCR_PGO;
			// fall through
			// no break
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			/* color bar pattern */
			DSI_VMCR &= ~DSI_VMCR_PGM;
			/* enable pattern */
			DSI_VMCR |= DSI_VMCR_PGE;
			break;
	}

	/* Enable DSI host and wrapper */
	dsi_enable();

	/* Enable all dsi interrupts */
	DSI_IER0 |= DSI_IER0_FLAGS_MASK;
	DSI_IER1 |= DSI_IER1_FLAGS_MASK;
//	DSI_WIER |= DSI_WIER_TEIE | DSI_WIER_ERIE | DSI_WIER_PLLLIE | DSI_WIER_PLLUIE | DSI_WIER_RRIE;

//	/* Disable all dsi interrupts */
//	DSI_IER0 &= ~(DSI_IER0_FLAGS_MASK);
//	DSI_IER1 &= ~(DSI_IER1_FLAGS_MASK);
//	DSI_WIER &= ~(DSI_WIER_TEIE | DSI_WIER_ERIE | DSI_WIER_PLLLIE | DSI_WIER_PLLUIE | DSI_WIER_RRIE);


	/*
	 * Enable interrupts
	 *
	 */

//	/* Set interrupt priorities */
//	nvic_set_priority(NVIC_LCD_TFT_IRQ,10);
//	nvic_set_priority(NVIC_LCD_TFT_ERR_IRQ,9);
//	nvic_set_priority(NVIC_DSIHOST_IRQ,10);

	/* Enable interrupts */
	nvic_enable_irq(NVIC_LCD_TFT_IRQ);
	nvic_enable_irq(NVIC_LCD_TFT_ERR_IRQ);
	nvic_enable_irq(NVIC_DSIHOST_IRQ);

	msleep(100);

	/*
	 * Initialize the display
	 *
	 */

	if (mode==DSI_MODE_ADAPTED_COMMAND_MODE) {
		/* Set all transmissions to low power mode during setup */
		dsi_command_config(
				DSI_CMCR_GSW0TX | DSI_CMCR_GSW1TX | DSI_CMCR_GSW2TX |
				DSI_CMCR_GSR0TX | DSI_CMCR_GSR1TX | DSI_CMCR_GSR2TX |
				DSI_CMCR_GLWTX  |
				DSI_CMCR_DSW0TX | DSI_CMCR_DSW1TX | DSI_CMCR_DSR0TX |
				DSI_CMCR_DLWTX
			);
	}

	/* Send initialisation commands to display */
	uint32_t otm8009a_format;
	switch (color_coding) {
		case DISPLAY_COLOR_CODING_RGB565 :
			otm8009a_format = OTM8009A_FORMAT_RBG565;
			break;
		case DISPLAY_COLOR_CODING_RGB888 :
		case DISPLAY_COLOR_CODING_ARGB8888 :
		default :
			otm8009a_format = OTM8009A_FORMAT_RGB888;
			break;
	}
	uint32_t otm8009a_orientation=0;
	switch (orientation) {
		case DISPLAY_ORIENTATION_PORTRAIT :
			otm8009a_orientation = OTM8009A_ORIENTATION_PORTRAIT;
			break;
		case DISPLAY_ORIENTATION_LANDSCAPE :
		default :
			otm8009a_orientation = OTM8009A_ORIENTATION_LANDSCAPE;
			break;
	}
	OTM8009A_Init(otm8009a_format, otm8009a_orientation);

	if (mode==DSI_MODE_ADAPTED_COMMAND_MODE) {
#if ENABLE_TEARING
		/* enable tearing */
		dsi_short_write(0 , DSI_GHCR_DT_DCS_SHORT_WRITE_1_PARAM, DSI_DCS_CMD_SET_TEAR_ON, 0);  // Only V-Blanking info
#endif

		/* Set all command transmissions to high speed mode */
		dsi_command_config(0);
	}


	/* Change flow control */
	dsi_protocol_flow_config(DSI_PCR_BTAE);

	/*
	 * Refresh the display
	 *
	 */
	dsi_refresh();
}


/*
 * LTDC related functions
 *
 */

void display_ltdc_set_background_color(uint8_t r, uint8_t g, uint8_t b) {
	ltdc_set_background_color(r,g,b);
}

static inline uint32_t align_up_to(uint32_t alignment, uint32_t alignee) {
	uint32_t off = alignee%alignment;
	if (!off) return alignee;
	return alignee+alignment-off;
}
void display_ltdc_config_layer(
		display_layer_t layer,
		bool enable
) {
	if (enable) {
		ltdc_layer_ctrl_enable(layer, LTDC_LxCR_LAYER_ENABLE);
	} else {
		ltdc_layer_ctrl_disable(layer, LTDC_LxCR_LAYER_ENABLE);
	}
}
void display_ltdc_config_windowing_xywh(
		display_layer_t layer,
		uint16_t x,     uint16_t y,
		uint16_t width, uint16_t height
) {
	uint16_t xoff = (LTDC_BPCR >> 16) & 0xffff; // hsync+h_back_porch
	uint16_t yoff = (LTDC_BPCR >> 0 ) & 0xffff; // vsync+v_back_porch
	ltdc_setup_windowing(layer, xoff+x, yoff+y, width,height);
}
/**
 * Config DSI ltdc interface
 * @param signal_polarity
 * @param HSA
 * @param HBP
 * @param HACT
 * @param HFP
 * @param VSA
 * @param VBP
 * @param VACT
 * @param VFP
 * @param color_coding
 * @param enable_color_keying_in_rgb565_mode
 * @param color_key
 * @param layer1_frame_buffer at least HACT*VACT*color_coding_size bytes
 * @param layer2_frame_buffer at least HACT*VACT*color_coding_size bytes
 * @return returns the actual width the frame buffer needs to have
 */
uint32_t display_ltdc_config(
	dsi_lpcr_flags_t signal_polarity,
	uint32_t HSA, uint32_t HBP, uint32_t HACT, uint32_t HFP,
	uint32_t VSA, uint32_t VBP, uint32_t VACT, uint32_t VFP,
	display_color_coding_t color_coding,
	bool enable_color_keying_in_rgb565_mode, uint16_t color_key,
	uint8_t layer1_frame_buffer[], uint8_t layer2_frame_buffer[]
) {
	assert(LTDC_LAYER_1 == DISPLAY_LAYER_1);
	assert(LTDC_LAYER_2 == DISPLAY_LAYER_2);

	/* Configure R,G,B component values for LCD background color */
	ltdc_set_background_color(0, 0, 0);

	/* Set PC polarity to active low */
	LTDC_GCR &= ~(LTDC_GCR_PCPOL_ACTIVE_HIGH);

	/* Set signal polarities (inverted from DSI! DEP is inverted twice!!) */
	LTDC_GCR &= ~(LTDC_GCR_DEPOL_ACTIVE_HIGH
				| LTDC_GCR_VSPOL_ACTIVE_HIGH
				| LTDC_GCR_HSPOL_ACTIVE_HIGH);
	if ( (signal_polarity & DSI_LPCR_DEP)) {
		LTDC_GCR |= LTDC_GCR_DEPOL_ACTIVE_HIGH;
	}
	if (!(signal_polarity & DSI_LPCR_VSP)) {
		LTDC_GCR |= LTDC_GCR_VSPOL_ACTIVE_HIGH;
	}
	if (!(signal_polarity & DSI_LPCR_HSP)) {
		LTDC_GCR |= LTDC_GCR_HSPOL_ACTIVE_HIGH;
	}

	/* Timing configuration */
	ltdc_set_tft_sync_timings(
			HSA,  VSA,  /* sync time */
			HBP,  VBP,  /* 'blind' pixels left  and up */
			HACT, VACT, /* display width/height, visible resolution */
			HFP,  VFP   /* 'blind' pixels right and down */
		);

	/* Pixel format */
	uint32_t ltdc_color_coding, ltdc_color_coding_byte_size;
	switch (color_coding) {
		case DISPLAY_COLOR_CODING_RGB565 :
			ltdc_color_coding = LTDC_LxPFCR_RGB565;
			ltdc_color_coding_byte_size = 2;
			break;
		case DISPLAY_COLOR_CODING_RGB888 :
			ltdc_color_coding = LTDC_LxPFCR_RGB888;
			ltdc_color_coding_byte_size = 3;
			break;
		case DISPLAY_COLOR_CODING_ARGB8888 :
		default :
			ltdc_color_coding = LTDC_LxPFCR_ARGB8888;
			ltdc_color_coding_byte_size = 4;
			break;
	}
	ltdc_set_pixel_format(LTDC_LAYER_1, ltdc_color_coding);
	ltdc_set_pixel_format(LTDC_LAYER_2, ltdc_color_coding);

    /* Default Color configuration (configure A,R,G,B component values) */
	ltdc_set_default_colors(LTDC_LAYER_1, 0, 0, 0, 0);
	ltdc_set_default_colors(LTDC_LAYER_2, 0, 0, 0, 0);

	/* Set layer constant alpha multiplier */
	ltdc_set_constant_alpha(LTDC_LAYER_1, 255);
	ltdc_set_constant_alpha(LTDC_LAYER_2, 255);

    /* Configure blending factors */
	ltdc_set_blending_factors(
			LTDC_LAYER_1,
			LTDC_LxBFCR_BF1_PIXEL_ALPHA_x_CONST_ALPHA,
			LTDC_LxBFCR_BF2_PIXEL_ALPHA_x_CONST_ALPHA
		);
	ltdc_set_blending_factors(
			LTDC_LAYER_2,
			LTDC_LxBFCR_BF1_PIXEL_ALPHA_x_CONST_ALPHA,
			LTDC_LxBFCR_BF2_PIXEL_ALPHA_x_CONST_ALPHA
		);

	/* Color keying */
	if ((ltdc_color_coding == LTDC_LxPFCR_RGB565)
	  && enable_color_keying_in_rgb565_mode) {
		uint32_t rgb888_color_key =
				ltdc_get_rgb888_from_rgb565(color_key);
		ltdc_set_color_key(
				LTDC_LAYER_2,
				(rgb888_color_key >> 16)&0xff,
				(rgb888_color_key >>  8)&0xff,
				(rgb888_color_key >>  0)&0xff
			);
		ltdc_layer_ctrl_enable(LTDC_LAYER_2, LTDC_LxCR_COLKEY_ENABLE);
	}

	/* Windowing */
	display_ltdc_config_windowing_xywh(LTDC_LAYER_1, 0,0, HACT,VACT);
	display_ltdc_config_windowing_xywh(LTDC_LAYER_2, 0,0, HACT,VACT);

	/* Line length and pitch */
	uint32_t line_byte_size = HACT*ltdc_color_coding_byte_size;
	uint32_t line_byte_pitch = align_up_to(64,line_byte_size);
//	assert("Invalid display width"&&(line_byte_size==line_byte_pitch));
	ltdc_set_fb_line_length(LTDC_LAYER_1, line_byte_size+3,line_byte_pitch);
	ltdc_set_fb_line_length(LTDC_LAYER_2, line_byte_size+3,line_byte_pitch);

	/* Configure the number of lines */
	ltdc_set_fb_line_count(LTDC_LAYER_1, VACT);
	ltdc_set_fb_line_count(LTDC_LAYER_2, VACT);

	/* Pixel buffers */
	ltdc_set_fbuffer_address(LTDC_LAYER_1, (uint32_t)layer1_frame_buffer);
	ltdc_set_fbuffer_address(LTDC_LAYER_2, (uint32_t)layer2_frame_buffer);

	/* Enable foreground & background Layers */
//	ltdc_layer_ctrl_disable(LTDC_LAYER_1, LTDC_LxCR_LAYER_ENABLE);
//	ltdc_layer_ctrl_disable(LTDC_LAYER_2, LTDC_LxCR_LAYER_ENABLE);
	ltdc_layer_ctrl_enable(LTDC_LAYER_1, LTDC_LxCR_LAYER_ENABLE);
	ltdc_layer_ctrl_enable(LTDC_LAYER_2, LTDC_LxCR_LAYER_ENABLE);

//	/* enable dithering to add artsy graphical artifacts */
	ltdc_ctrl_enable(LTDC_GCR_DITHER_ENABLE);

	/* update settings (this is sometimes needed..) */
	ltdc_reload(LTDC_SRCR_RELOAD_IMR);
	while (!display_ltdc_config_ready());

	return line_byte_pitch/ltdc_color_coding_byte_size;
}

display_dsi_mode_t display_get_dsi_mode(void) {
	if (DSI_WCFGR & DSI_WCFGR_DSIM) {
		if (DSI_MCR & DSI_MCR_CMDM) {
			return DSI_MODE_ADAPTED_COMMAND_MODE;
		}
	} else
	if (DSI_VMCR & DSI_VMCR_PGE) {
		if (DSI_VMCR & DSI_VMCR_PGM) {
			return DSI_MODE_VIDEO_PATTERN_BER;
		} else
		if (DSI_VMCR & DSI_VMCR_PGO) {
			return DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL;
		} else {
			return DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL;
		}
	} else {
		switch ((dsi_vmcr_vmt_t)((DSI_VMCR >> DSI_VMCR_VMT_SHIFT)&DSI_VMCR_VMT_MASK)) {
			case DSI_VMCR_VMT_NON_BURST_PULSE :
				return DSI_MODE_VIDEO_SYNC_PULSES;
			case DSI_VMCR_VMT_NON_BURST_EVENT :
				return DSI_MODE_VIDEO_SYNC_EVENTS;
			case DSI_VMCR_VMT_BURST :
			case DSI_VMCR_VMT_BURST_ :
				return DSI_MODE_VIDEO_BURST;
		}
	}
	return -1;
}

bool display_ready() {
	switch (display_get_dsi_mode()) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			// TBD
			break;
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			if (DSI_WISR & DSI_WISR_BUSY) return false;
			return true;
			break;
	}
	return display_ltdc_config_ready();
}
void display_update() {
	switch (display_get_dsi_mode()) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			break;
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			//if (!(DSI_WCFGR & DSI_WCFGR_AR)) {
				dsi_refresh();
			//}
			break;
	}
}
bool display_ltdc_config_ready() {
	return !LTDC_SRCR_IS_RELOADING();
}
void display_ltdc_config_begin() {
	switch (display_get_dsi_mode()) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			break;
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			/* Disable the DSI wrapper */
			DSI_WCR &= ~DSI_WCR_DSIEN;
			wait_cycles(10);
//			while (!display_ready());
			break;
	}
}
void display_ltdc_config_end() {
	switch (display_get_dsi_mode()) {
		case DSI_MODE_VIDEO_SYNC_PULSES :
		case DSI_MODE_VIDEO_SYNC_EVENTS :
		case DSI_MODE_VIDEO_BURST :
		case DSI_MODE_VIDEO_PATTERN_BER :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_HORIZONTAL :
		case DSI_MODE_VIDEO_PATTERN_COLOR_BARS_VERTICAL :
			ltdc_reload(LTDC_SRCR_RELOAD_VBR);
			break;
		case DSI_MODE_ADAPTED_COMMAND_MODE :
			ltdc_reload(LTDC_SRCR_RELOAD_IMR);
			while (!display_ltdc_config_ready());
			/* Enable the DSI wrapper */
			DSI_WCR |= DSI_WCR_DSIEN;
			wait_cycles(10);
			break;
	}
}

/* Interrupts */
void lcd_tft_isr(void) {
	uint32_t events = LTDC_ISR & (LTDC_ISR_LIF | LTDC_ISR_RRIF);
	if (events & LTDC_ISR_LIF) {
		/* Line interrupt:
		 *  generated when a programmed line is reached.
		 *  The line interrupt position is programmed in the LTDC_LIPCR register
		 */
	} else
	if (events & LTDC_ISR_RRIF) {
		/* Register reload interrupt:
		 *  generated when the shadow registers reload
		 *  was performed during the vertical blanking period
		 */
	}
	LTDC_ICR = events;
	wait_cycles(3); // wait a bit for the isr to be cleared
	// nop?
}
void lcd_tft_err_isr(void) {
	uint32_t errors = LTDC_ISR & (LTDC_ISR_FUIF | LTDC_ISR_TERRIF);
	assert(errors && "There can't be no errors!");
	if (errors & LTDC_ISR_FUIF) {
		/* FIFO underrun interrupt:
		 *  generated when a pixel is requested from an empty layer FIFO
		 */
	}
	if (errors & LTDC_ISR_TERRIF) {
		/* Transfer error interrupt:
		 *  generated when an AHB bus error occurs during data transfer
		 */
	}
	LTDC_ICR = errors;
	wait_cycles(3); // wait a bit for the isr to be cleared
	// nop?
}
void dsihost_isr(void) {
	dsi_isr0_flags_t isr0 = DSI_ISR0;
	dsi_isr1_flags_t isr1 = DSI_ISR1;
	dsi_wisr_flags_t wisr = DSI_WISR & (DSI_WISR_TEIF | DSI_WISR_ERIF | DSI_WISR_PLLLIF | DSI_WISR_PLLUIF | DSI_WISR_RRIF);
	if (isr0) {
		assert("Shirt!" &&0);
	}
	if (isr1) {
		assert("Shirt!" &&0);
	}
	if (wisr) {
		DSI_WIFCR = wisr;
	}
	wait_cycles(3); // wait a bit for the isr to be cleared
}

/* Implemented functions */
void DSI_IO_WriteCmd(uint32_t NbrParams, uint8_t *pParams) {
	if (NbrParams <= 1) {
		dsi_short_write(0, DSI_GHCR_DT_DCS_SHORT_WRITE_1_PARAM, pParams[0], pParams[1]);
	} else {
		dsi_long_write(0, DSI_GHCR_DT_DCS_LONG_WRITE, NbrParams, pParams[NbrParams], pParams);
	}
}

void OTM8009A_IO_Delay(uint32_t Delay) {
	msleep(Delay);
}
