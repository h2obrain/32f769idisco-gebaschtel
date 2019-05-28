/*
 * dsi_helper_functions.h
 *
 *  Created on: 23 Apr 2019
 *      Author: h2obrain
 */

#ifndef DRIVERS_DSI_HELPER_FUNCTIONS_H_
#define DRIVERS_DSI_HELPER_FUNCTIONS_H_

#include <stdbool.h>
#include <assert.h>
#include <libopencm3/stm32/dsi.h>
#include <libopencm3/stm32/ltdc.h>
#include <libopencm3/stm32/dma2d.h>
#include "../clock.h"


#define WAIT_AFTER() //wait_cycles(3)
#define WAIT_AFTER2() //wait_cycles(10) //216000/100000) //msleep_loop(1) //msleep(1) // wait_cycles(10000)

#ifndef MAX
#define MAX(a,b) a>b?a:b
#endif

/*
 * Common register types
 */

/** @defgroup dsi_color_coding_values dsi_color_coding_t values
 * @ingroup dsihost_registers
 * @brief  Color type used in various registers
@{*/

//0000: 16-bit configuration 1
//0001: 16-bit configuration 2
//0010: 16-bit configuration 3
//0011: 18-bit configuration 1
//0100: 18-bit configuration 2
//0101: 24-bit
//Others: Reserved
typedef enum {
	DSI_COLOR_CODING_RGB565   = 0,
	DSI_COLOR_CODING_RGB565_  = 1,
	DSI_COLOR_CODING_RGB565__ = 2,
	DSI_COLOR_CODING_RGB666   = 3,
	DSI_COLOR_CODING_RGB666_  = 4,
	DSI_COLOR_CODING_RGB888   = 5
	/* Reserved: 6-15 */
} dsi_color_coding_t;

/*@}*/





/*
 * basic control functions
 *
 */

/** @defgroup dsi_functions DSI Functions
 * @ingroup dsi_registers
 * @brief DSI Host Dummi access functions
@{*/

/**
 * Enable DSI-host/wrapper
 */
static inline
void dsi_enable(void) {
	/* Enable the DSI host */
	DSI_CR  |= DSI_CR_EN;
	WAIT_AFTER();
	/* Enable the DSI wrapper */
	DSI_WCR |= DSI_WCR_DSIEN;
	WAIT_AFTER();
	WAIT_AFTER2();
}

/**
 * Disable DSI-host/wrapper
 */
static inline
void dsi_disable(void) {
	/* Disable the DSI wrapper */
	DSI_WCR &= ~DSI_WCR_DSIEN;
	WAIT_AFTER();
	/* Disable the DSI host */
	DSI_CR  &= ~DSI_CR_EN;
	WAIT_AFTER();
	WAIT_AFTER2();
}
/**
 * Disable DSI-host/wrapper/pll/regulator and ltdc
 */
static inline
void dsi_disable_all(void) {
	dsi_disable();
	/* Disable PHY and clock lane */
	DSI_PCTLR &= ~(DSI_PCTLR_DEN | DSI_PCTLR_CKE);
	/* Disable PLL */
	DSI_WRPCR &= ~(DSI_WRPCR_PLLEN);
	/* Disable regulator */
	DSI_WRPCR &= ~(DSI_WRPCR_REGEN);
	/* Disable LTDC */
	LTDC_GCR &= LTDC_GCR_LTDC_ENABLE;
	WAIT_AFTER2();
}
/**
 * Enable DSI 1.2V regulator
 */
static inline
void dsi_regulator_enable(void) {
	DSI_WRPCR |= DSI_WRPCR_REGEN;
	WAIT_AFTER2();
}
/**
 * Get DSI 1.2V regulator status
 * @return true if ready
 */
static inline
bool dsi_regulator_ready(void) {
	return (DSI_WISR & DSI_WISR_RRS) != 0;
}
/*@}*/

/**
 * DO NOT CALL THIS FUNCTION DIRECTLY!
 * set RCC_DCKCFGR2_DSISEL in RCC_DCKCFGR2 to use PLLR
 * when dsi-pll could be disabled, eg. in low power mode
 *
 * byte_lane_clock = HSE_CLK / idf * ndiv / odf /8
 * @param idf  1-7
 * @param ndiv 10-125
 * @param odf  1, 2, 4 or 8
 * @return byte_lane_clock
 */
static inline
uint32_t dsi_pll_config(
		uint32_t idf, uint32_t ndiv, uint32_t odf
) {
	assert(!(DSI_WRPCR & DSI_WRPCR_PLLEN));

	assert((1<=idf)&&(idf<=7));
	assert((10<=ndiv)&&(ndiv<=125));
	assert((1==odf)||(2==odf)||(4==odf)||(8==odf));

	uint32_t FVCO = HSE_FREQUENCY_MHZ / idf * 1000000 * 2 * ndiv;
	assert((500000000<=FVCO)&&(FVCO<=1000000000));
	uint32_t HS = FVCO / (2*odf);
	assert((80000000<=HS)&&(HS<=500000000));
	uint32_t LBCLK = HS / 8;
	assert(LBCLK<=62500000);

	switch (odf) {
		case 1:
			odf = 0;
			break;
		case 2:
			odf = 1;
			break;
		case 4:
			odf = 2;
			break;
		case 8:
			odf = 3;
			break;
		default :
			assert("Invalid odf value"&&0);
	}

	/* clear DSI sel flag (default) */
	RCC_DCKCFGR2 &= ~RCC_DCKCFGR2_DSISEL;
	/* setup pll */
	DSI_WRPCR &= ~(
		(DSI_WRPCR_ODF_MASK << DSI_WRPCR_ODF_SHIFT)
	  | (DSI_WRPCR_IDF_MASK << DSI_WRPCR_IDF_SHIFT)
	  | (DSI_WRPCR_NDIV_MASK << DSI_WRPCR_NDIV_SHIFT));
	DSI_WRPCR |=
		((odf & DSI_WRPCR_ODF_MASK) << DSI_WRPCR_ODF_SHIFT)
	  | ((idf & DSI_WRPCR_IDF_MASK) << DSI_WRPCR_IDF_SHIFT)
	  | ((ndiv & DSI_WRPCR_NDIV_MASK) << DSI_WRPCR_NDIV_SHIFT);

	WAIT_AFTER2();
	return LBCLK;
}
static inline
void dsi_pll_enable(void) {
    DSI_WRPCR |= DSI_WRPCR_PLLEN;
	WAIT_AFTER2();
}
static inline
bool dsi_pll_ready(void) {
	return (DSI_WISR & DSI_WISR_PLLLS) != 0;
}

static inline
void dsi_protocol_flow_config(dsi_pcr_flags_t flow_control)
{
	/* Set the DSI Host Protocol Configuration Register */
	DSI_PCR &= ~(DSI_PCR_FLAGS_MASK);
	DSI_PCR |= (flow_control & DSI_PCR_FLAGS_MASK);
	WAIT_AFTER2();
}

/**
 *
 * @param idf
 * @param ndiv
 * @param odf
 * @param tx_escape_clk 1-20MHz
 * @param num_lanes
 * @param clock_lane_config
 * @param clocklane_hs2lp_time Only needed for acl-ctrl
 * @param clocklane_lp2hs_time Only needed for acl-ctrl
 * @param datalane_hs2lp_time Only needed for adl-ctrl
 * @param datalane_lp2hs_time Only needed for adl-ctrl
 * @param mrd_time
 * @param sw_time
 * @param flow_control
 * @return
 */
static inline
uint32_t dsi_phy_config(
	uint32_t idf, uint32_t ndiv, uint32_t odf,
	uint32_t tx_escape_clk,
	uint32_t num_lanes,
//	uint32_t unit_interval_x4,
	dsi_clcr_flags_t clock_lane_config,
	uint32_t clocklane_hs2lp_time, uint32_t clocklane_lp2hs_time,
	uint32_t datalane_hs2lp_time, uint32_t datalane_lp2hs_time,
	uint32_t mrd_time,  uint32_t sw_time,
	dsi_pcr_flags_t flow_control
) {
	assert((1<=num_lanes)&&(num_lanes<=2));
	assert(tx_escape_clk<=20000000);

	/* Enable DSI host 1.2V voltage regulator */
	dsi_regulator_enable();
	while (!dsi_regulator_ready());

	/*
	 * Config DSI pll
	 *  lane_byte_clk = 62.5Mhz = HSE(25Mhz) / idf * ndiv / odf / 8
	 *
	 */
	uint32_t lane_byte_clk = dsi_pll_config(idf,ndiv,odf);
	dsi_pll_enable();
	while (!dsi_pll_ready());

	/* Enable PHY and clock lane */
	DSI_PCTLR |= DSI_PCTLR_DEN | DSI_PCTLR_CKE;

	/* Config clock lane control (auto by phy) */
	DSI_CLCR &= ~(DSI_CLCR_ACR | DSI_CLCR_DPCC);
	DSI_CLCR |= clock_lane_config & DSI_CLCR_FLAGS_MASK;

	/* Set lane count */
	DSI_PCONFR &= ~DSI_PCONFR_NL_MASK;
	DSI_PCONFR |= (((num_lanes-1) & DSI_PCONFR_NL_MASK) << DSI_PCONFR_NL_SHIFT);

	/* Set TX escape clock div */
	uint32_t txeckdiv = lane_byte_clk / tx_escape_clk;
	assert("Values 0 and 1 stop txe-clk!"&&(txeckdiv>1));
	assert((txeckdiv==4)&&"Remove this debug test!");
	DSI_CCR &= ~(DSI_CCR_TXECKDIV_MASK << DSI_CCR_TXECKDIV_SHIFT);
	DSI_CCR |= (txeckdiv & DSI_CCR_TXECKDIV_MASK) << DSI_CCR_TXECKDIV_SHIFT;

	/* Clock lane timer configuration */
	/* In Automatic Clock Lane control mode, the DSI Host can turn off the clock lane between two
	  High-Speed transmission.
	  To do so, the DSI Host calculates the time required for the clock lane to change from HighSpeed
	  to Low-Power and from Low-Power to High-Speed.
	  This timings are configured by the HS2LP_TIME and LP2HS_TIME in the DSI Host Clock Lane Timer Configuration Register (DSI_CLTCR).
	  But the DSI Host is not calculating LP2HS_TIME + HS2LP_TIME but 2 x HS2LP_TIME.

	  Workaround : Configure HS2LP_TIME and LP2HS_TIME with the same value being the max of HS2LP_TIME or LP2HS_TIME.
	*/
	uint32_t max_transition_time = MAX(clocklane_hs2lp_time,clocklane_lp2hs_time);
	DSI_CLTCR |=
		((max_transition_time & DSI_CLTCR_HS2LP_TIME_MASK) << DSI_CLTCR_HS2LP_TIME_SHIFT)
	  | ((max_transition_time & DSI_CLTCR_LP2HS_TIME_MASK) << DSI_CLTCR_LP2HS_TIME_SHIFT);
//	DSI_CLTCR |=
//		((clocklane_hs2lp_time & DSI_CLTCR_HS2LP_TIME_MASK) << DSI_CLTCR_HS2LP_TIME_SHIFT)
//	  | ((clocklane_lp2hs_time & DSI_CLTCR_LP2HS_TIME_MASK) << DSI_CLTCR_LP2HS_TIME_SHIFT);

	/* Data lane timer configuration */
	DSI_DLTCR =
		((datalane_hs2lp_time & DSI_DLTCR_HS2LP_TIME_MASK) << DSI_DLTCR_HS2LP_TIME_SHIFT)
	  | ((datalane_lp2hs_time & DSI_DLTCR_LP2HS_TIME_MASK) << DSI_DLTCR_LP2HS_TIME_SHIFT)
	  | ((mrd_time & DSI_DLTCR_MRD_TIME_MASK) << DSI_DLTCR_MRD_TIME_SHIFT);

	/* Configure the wait period to request HS transmission after a stop state */
	DSI_PCONFR &= ~(DSI_PCONFR_SW_TIME_MASK << DSI_PCONFR_SW_TIME_SHIFT);
	DSI_PCONFR |= ((sw_time & DSI_PCONFR_SW_TIME_MASK) << DSI_PCONFR_SW_TIME_SHIFT);

	/* DSI wrapper config */
	// unit_interval_x4 = (1/0.25ns * idf * odf) / (HSE * ndiv)
	uint32_t unit_interval_x4 = (uint32_t)(((uint64_t)4000000000 * idf * odf) / ((HSE_FREQUENCY_MHZ*1000000) * ndiv));
	DSI_WPCR0 &= ~(DSI_WPCR0_UIX4_MASK << DSI_WPCR0_UIX4_SHIFT);
	DSI_WPCR0 |= (unit_interval_x4 & DSI_WPCR0_UIX4_MASK) << DSI_WPCR0_UIX4_SHIFT;

	/* Config flow control */
	dsi_protocol_flow_config(flow_control);

	return lane_byte_clk;
}


/**
 * Config DSI video mode
 * @param video_mode_type
 * @param video_packet_size
 * @param num_chunks
 * @param null_packet_size
 * @param virtual_channel_id
 * @param control_signals_polarity
 * @param color_coding
 * @param loosely_packet_rgb666
 * @param lane_byte_clk
 * @param lcd_clk
 * @param HSA   sync_width
 * @param HBP   h_back_porch
 * @param HACT  active_width
 * @param HFP   h_front_porch
 * @param VSA   sync_height
 * @param VBP   v_back_porch
 * @param VACT  active_height
 * @param VFP   v_front_porch
 * @param low_power_cmd_enable
 * @param low_power_transition_config
 *           Allowed flags:
 *				DSI_VMCR_LPCE,DSI_VMCR_LPHFPE,DSI_VMCR_LPHBPE,
 *				DSI_VMCR_LPVAE, DSI_VMCR_LPVFPE, DSI_VMCR_LPVBPE,
 *				DSI_VMCR_LPVSAE //, DSI_VMCR_FBTAAE
 * @param frame_bta_ack_enable
 * @param largest_packet_size
 * @param vact_largest_packet_size
 */
static inline
void dsi_video_mode_config(
		dsi_vmcr_vmt_t video_mode_type,
		uint32_t video_packet_size,
		uint32_t num_chunks,
		uint32_t null_packet_size,
		uint32_t virtual_channel_id,
		dsi_lpcr_flags_t control_signals_polarity,
		dsi_color_coding_t color_coding,
		bool loosely_packet_rgb666,
		uint32_t lane_byte_clk, uint32_t lcd_clk,
		uint32_t HSA, uint32_t HBP, uint32_t HACT, uint32_t HFP,
		uint32_t VSA, uint32_t VBP, uint32_t VACT, uint32_t VFP,
		bool low_power_cmd_enable,
		dsi_vmcr_flags_t low_power_transition_config,
		bool frame_bta_ack_enable,
		uint32_t largest_packet_size,
		uint32_t vact_largest_packet_size
) {
	/* Convert timing values to DSI time */
	uint32_t horizontal_line;
	horizontal_line   = (uint32_t)(((uint64_t)(HSA + HBP + HACT + HFP) * lane_byte_clk +lcd_clk/2)/lcd_clk);
	HSA = (uint32_t)(((uint64_t)HSA * lane_byte_clk +lcd_clk/2)/lcd_clk);
	HBP = (uint32_t)(((uint64_t)HBP * lane_byte_clk +lcd_clk/2)/lcd_clk);
//	VSA = VSA;
//	VBP = VBP;
//	VFP = VFP;
//	VACT = VACT; /* Value depending on display orientation choice portrait/landscape */

	/* Select video mode by resetting CMDM and DSIM bits */
	DSI_MCR   &= ~DSI_MCR_CMDM;
	DSI_WCFGR &= ~DSI_WCFGR_DSIM;

	/* Configure the video mode transmission type */
	DSI_VMCR &= ~(DSI_VMCR_VMT_MASK << DSI_VMCR_VMT_SHIFT);
	DSI_VMCR |= (video_mode_type & DSI_VMCR_VMT_MASK) << DSI_VMCR_VMT_SHIFT;

	/* Configure the video packet size */
	DSI_VPCR &= ~((DSI_VPCR_VPSIZE_MASK << DSI_VPCR_VPSIZE_SHIFT));
	DSI_VPCR |= ((video_packet_size & DSI_VPCR_VPSIZE_MASK) << DSI_VPCR_VPSIZE_SHIFT);

	/* Set the chunks number to be transmitted through the DSI link */
	DSI_VCCR &= ~((DSI_VCCR_NUMC_MASK << DSI_VCCR_NUMC_SHIFT));
	DSI_VCCR |= ((num_chunks & DSI_VCCR_NUMC_MASK) << DSI_VCCR_NUMC_SHIFT);

	/* Set the size of the null packet */
	DSI_VNPCR &= ~((DSI_VNPCR_NPSIZE_MASK << DSI_VNPCR_NPSIZE_SHIFT));
	DSI_VNPCR |= ((null_packet_size & DSI_VNPCR_NPSIZE_MASK) << DSI_VNPCR_NPSIZE_SHIFT);

	/* Select the virtual channel for the LTDC interface traffic */
	DSI_LVCIDR &= ~((DSI_LVCIDR_VCID_MASK << DSI_LVCIDR_VCID_SHIFT));
	DSI_LVCIDR |= ((virtual_channel_id & DSI_LVCIDR_VCID_MASK) << DSI_LVCIDR_VCID_SHIFT);

	/* Configure the polarity of control signals */
	DSI_LPCR &= ~(DSI_LPCR_FLAGS_MASK);
	DSI_LPCR |= (control_signals_polarity & DSI_LPCR_FLAGS_MASK);

	/* Select the color coding for the host */
	DSI_LCOLCR &= ~(DSI_LCOLCR_COLC_MASK << DSI_LCOLCR_COLC_SHIFT);
	DSI_LCOLCR |= (color_coding & DSI_LCOLCR_COLC_MASK) << DSI_LCOLCR_COLC_SHIFT;

	/* Select the color coding for the wrapper */
	DSI_WCFGR &= ~(DSI_WCFGR_COLMUX_MASK << DSI_WCFGR_COLMUX_SHIFT);
	DSI_WCFGR |= (color_coding & DSI_WCFGR_COLMUX_MASK) << DSI_WCFGR_COLMUX_SHIFT;

	/* Enable/disable the loosely packed variant to 18-bit configuration */
	DSI_LCOLCR &= ~DSI_LCOLCR_LPE;
	switch (color_coding) {
		case DSI_COLOR_CODING_RGB666 :
		case DSI_COLOR_CODING_RGB666_ :
			if (loosely_packet_rgb666) {
				DSI_LCOLCR |= DSI_LCOLCR_LPE;
			}
			break;
		default :
			break;
	}

	/* Set the Horizontal Synchronization Active (HSA) in lane byte clock cycles */
	DSI_VHSACR &= ~((DSI_VHSACR_HSA_MASK << DSI_VHSACR_HSA_SHIFT));
	DSI_VHSACR |= ((HSA & DSI_VHSACR_HSA_MASK) << DSI_VHSACR_HSA_SHIFT);

	/* Set the Horizontal Back Porch (HBP) in lane byte clock cycles */
	DSI_VHBPCR &= ~((DSI_VHBPCR_HBP_MASK << DSI_VHBPCR_HBP_SHIFT));
	DSI_VHBPCR |= ((HBP & DSI_VHBPCR_HBP_MASK) << DSI_VHBPCR_HBP_SHIFT);

	/* Set the total line time (HLINE=HSA+HBP+HACT+HFP) in lane byte clock cycles */
	DSI_VLCR &= ~((DSI_VLCR_HLINE_MASK << DSI_VLCR_HLINE_SHIFT));
	DSI_VLCR |= ((horizontal_line & DSI_VLCR_HLINE_MASK) << DSI_VLCR_HLINE_SHIFT);

	/* Set the Vertical Synchronization Active (VSA) */
	DSI_VVSACR &= ~((DSI_VVSACR_VSA_MASK << DSI_VVSACR_VSA_SHIFT));
	DSI_VVSACR |= ((VSA & DSI_VVSACR_VSA_MASK) << DSI_VVSACR_VSA_SHIFT);

	/* Set the Vertical Back Porch (VBP) */
	DSI_VVBPCR &= ~((DSI_VVBPCR_VBP_MASK << DSI_VVBPCR_VBP_SHIFT));
	DSI_VVBPCR |= ((VBP & DSI_VVBPCR_VBP_MASK) << DSI_VVBPCR_VBP_SHIFT);

	/* Set the Vertical Front Porch (VFP) */
	DSI_VVFPCR &= ~((DSI_VVFPCR_VFP_MASK << DSI_VVFPCR_VFP_SHIFT));
	DSI_VVFPCR |= ((VFP & DSI_VVFPCR_VFP_MASK) << DSI_VVFPCR_VFP_SHIFT);

	/* Set the Vertical Active period*/
	DSI_VVACR &= ~((DSI_VVACR_VA_MASK << DSI_VVACR_VA_SHIFT));
	DSI_VVACR |= ((VACT & DSI_VVACR_VA_MASK) << DSI_VVACR_VA_SHIFT);

	/* Configure the command transmission low power transition */
	uint32_t vmcr_lp_transition_config_mask =
				DSI_VMCR_LPHFPE | DSI_VMCR_LPHBPE |
				DSI_VMCR_LPVAE  | DSI_VMCR_LPVFPE | DSI_VMCR_LPVBPE |
				DSI_VMCR_LPVSAE;
	DSI_VMCR &= ~(DSI_VMCR_LPCE | vmcr_lp_transition_config_mask | DSI_VMCR_FBTAAE);
	low_power_transition_config &= vmcr_lp_transition_config_mask;
	if (low_power_cmd_enable) low_power_transition_config |= DSI_VMCR_LPCE;
	if (frame_bta_ack_enable) low_power_transition_config |= DSI_VMCR_FBTAAE;
	DSI_VMCR |= low_power_transition_config;

	/* Low power (VACT) largest packet size and */
	DSI_LPMCR &= ~(
		(DSI_LPMCR_LPSIZE_MASK << DSI_LPMCR_LPSIZE_SHIFT)
	  | (DSI_LPMCR_VLPSIZE_MASK << DSI_LPMCR_VLPSIZE_SHIFT));
	DSI_LPMCR |=
		((largest_packet_size & DSI_LPMCR_LPSIZE_MASK) << DSI_LPMCR_LPSIZE_SHIFT)
	  | ((vact_largest_packet_size & DSI_LPMCR_VLPSIZE_MASK) << DSI_LPMCR_VLPSIZE_SHIFT);

	WAIT_AFTER2();
}

/**
 *
 * @param virtual_channel_id
 * @param max_cmd_size
 * @param control_signals_polarity
 * @param color_coding
 * @param tearing_config
 * 			Allowed flags:
 * 			  DSI_WCFGR_TESRC, DSI_WCFGR_TEPOL, DSI_WCFGR_AR,
 * 			  DSI_WCFGR_VSPOL
 * @param tearing_ack_request
 */
static inline
void dsi_adapted_command_mode_config(
	uint32_t virtual_channel_id,
	uint32_t max_cmd_size,
	dsi_lpcr_flags_t control_signals_polarity,
	dsi_color_coding_t color_coding,
	dsi_wcfgr_flags_t  tearing_config,
	bool tearing_ack_request
) {
	/* Select command mode by setting CMDM and DSIM bits */
	DSI_MCR   |= DSI_MCR_CMDM;
	DSI_WCFGR &= ~DSI_WCFGR_DSIM;
	DSI_WCFGR |=  DSI_WCFGR_DSIM;

	/* Select the virtual channel for the LTDC interface traffic */
	DSI_LVCIDR &= ~((DSI_LVCIDR_VCID_MASK << DSI_LVCIDR_VCID_SHIFT));
	DSI_LVCIDR |= ((virtual_channel_id & DSI_LVCIDR_VCID_MASK) << DSI_LVCIDR_VCID_SHIFT);

	/* Configure the polarity of control signals */
	DSI_LPCR &= ~(DSI_LPCR_FLAGS_MASK);
	DSI_LPCR |= (control_signals_polarity & DSI_LPCR_FLAGS_MASK);

	/* Select the color coding for the host */
	DSI_LCOLCR &= ~(DSI_LCOLCR_COLC_MASK << DSI_LCOLCR_COLC_SHIFT);
	DSI_LCOLCR |= (color_coding & DSI_LCOLCR_COLC_MASK) << DSI_LCOLCR_COLC_SHIFT;

	/* Select the color coding for the wrapper */
	DSI_WCFGR &= ~(DSI_WCFGR_COLMUX_MASK << DSI_WCFGR_COLMUX_SHIFT);
	DSI_WCFGR |= (color_coding & DSI_WCFGR_COLMUX_MASK) << DSI_WCFGR_COLMUX_SHIFT;

	/* Configure the maximum allowed size for write memory command */
	DSI_LCCR &= ~((DSI_LCCR_CMDSIZE_MASK << DSI_LCCR_CMDSIZE_SHIFT));
	DSI_LCCR |= ((max_cmd_size & DSI_LCCR_CMDSIZE_MASK) << DSI_LCCR_CMDSIZE_SHIFT);

	/* Configure the tearing effect source and polarity and select the refresh mode */
	dsi_wcfgr_flags_t tearing_config_mask = DSI_WCFGR_TESRC | DSI_WCFGR_TEPOL | DSI_WCFGR_AR | DSI_WCFGR_VSPOL;
	DSI_WCFGR &= ~tearing_config_mask;
	DSI_WCFGR |= tearing_config & tearing_config_mask;

	/* Configure the tearing effect acknowledge request */
	DSI_CMCR &= ~DSI_CMCR_TEARE;
	if (tearing_ack_request) DSI_CMCR |= DSI_CMCR_TEARE;
	WAIT_AFTER2();
}

/**
 *
 * @param command_config
 * 			Allowed flags:
 * 				DSI_CMCR_GSW0TX, DSI_CMCR_GSW1TX, DSI_CMCR_GSW2TX,
 *				DSI_CMCR_GSR0TX, DSI_CMCR_GSR1TX, DSI_CMCR_GSR2TX,
 *				DSI_CMCR_GLWTX,
 *				DSI_CMCR_DSW0TX, DSI_CMCR_DSW1TX, DSI_CMCR_DSR0TX,
 *				DSI_CMCR_DLWTX, DSI_CMCR_MRDPS, DSI_CMCR_ARE
 */
static inline
void dsi_command_config(
		dsi_cmcr_flags_t command_config
) {
	dsi_cmcr_flags_t command_config_mask =
				DSI_CMCR_GSW0TX | DSI_CMCR_GSW1TX | DSI_CMCR_GSW2TX |
				DSI_CMCR_GSR0TX | DSI_CMCR_GSR1TX | DSI_CMCR_GSR2TX |
				DSI_CMCR_GLWTX  |
				DSI_CMCR_DSW0TX | DSI_CMCR_DSW1TX | DSI_CMCR_DSR0TX |
				DSI_CMCR_DLWTX  | DSI_CMCR_MRDPS | DSI_CMCR_ARE;
	/* Select High-speed or Low-power for command transmission */
	DSI_CMCR &= ~command_config_mask;
	DSI_CMCR |= command_config & command_config_mask;
	/* Configure the acknowledge request after each packet transmission */
	DSI_CMCR &= ~DSI_CMCR_ARE;
	DSI_CMCR |= DSI_CMCR_ARE;
	WAIT_AFTER2();
}

static inline
void dsi_refresh(void) {
	/* Update the display */
	DSI_WCR |= DSI_WCR_LTDCEN;
	WAIT_AFTER2();
}


typedef enum {
	/*
	 *  READ requests indicating a PRESP_TO (replicated for HS and LP)
	 */

	/* Generic read, no parameters short */
	DSI_GHCR_DT_READ                     = 0x04,
	/* Generic read, 1 parameter short */
	DSI_GHCR_DT_READ_1_PARAM             = 0x14,
	/* Generic read, 2 parameters short */
	DSI_GHCR_DT_READ_2_PARAMS            = 0x24,
	/* DCS read, no parameters short */
	DSI_GHCR_DT_DCS_READ                 = 0x06,

	/*
	 * WRITE requests indicating a PRESP_TO (replicated for HS and LP)
	 */

	/* Generic short write, no parameters short */
	DSI_GHCR_DT_SHORT_WRITE              = 0x03,
	/* Generic short write, 1 parameter short */
	DSI_GHCR_DT_SHORT_WRITE_1_PARAM      = 0x13,
	/* Generic short write, 2 parameters short */
	DSI_GHCR_DT_SHORT_WRITE_2_PARAMS     = 0x23,
	/* Generic long write long */
	DSI_GHCR_DT_LONG_WRITE               = 0x29,
	/* DCS short write, no parameters short */
	DSI_GHCR_DT_DCS_SHORT_WRITE          = 0x05,
	/* DCS short write, 1 parameter short */
	DSI_GHCR_DT_DCS_SHORT_WRITE_1_PARAM  = 0x15,
	/* DCS long write/write_LUT, command packet long */
	DSI_GHCR_DT_DCS_LONG_WRITE           = 0x39,
	/* Set maximum return packet size */
	DSI_GHCR_DT_SET_MAX_RETURN_PKT_SIZE  = 0x37,
} dsi_ghcr_dt_t;
typedef enum {
	DSI_DCS_CMD_ENTER_IDLE_MODE       = 0x39,
	DSI_DCS_CMD_ENTER_INVERT_MODE     = 0x21,
	DSI_DCS_CMD_ENTER_NORMAL_MODE     = 0x13,
	DSI_DCS_CMD_ENTER_PARTIAL_MODE    = 0x12,
	DSI_DCS_CMD_ENTER_SLEEP_MODE      = 0x10,
	DSI_DCS_CMD_EXIT_IDLE_MODE        = 0x38,
	DSI_DCS_CMD_EXIT_INVERT_MODE      = 0x20,
	DSI_DCS_CMD_EXIT_SLEEP_MODE       = 0x11,
	DSI_DCS_CMD_GET_3D_CONTROL        = 0x3F,
	DSI_DCS_CMD_GET_ADDRESS_MODE      = 0x0B,
	DSI_DCS_CMD_GET_BLUE_CHANNEL      = 0x08,
	DSI_DCS_CMD_GET_DIAGNOSTIC_RESULT = 0x0F,
	DSI_DCS_CMD_GET_DISPLAY_MODE      = 0x0D,
	DSI_DCS_CMD_GET_GREEN_CHANNEL     = 0x07,
	DSI_DCS_CMD_GET_PIXEL_FORMAT      = 0x0C,
	DSI_DCS_CMD_GET_POWER_MODE        = 0x0A,
	DSI_DCS_CMD_GET_RED_CHANNEL       = 0x06,
	DSI_DCS_CMD_GET_SCANLINE          = 0x45,
	DSI_DCS_CMD_GET_SIGNAL_MODE       = 0x0E,
	DSI_DCS_CMD_NOP                   = 0x00,
	DSI_DCS_CMD_READ_DDB_CONTINUE     = 0xA8,
	DSI_DCS_CMD_READ_DDB_START        = 0xA1,
	DSI_DCS_CMD_READ_MEMORY_CONTINUE  = 0x3E,
	DSI_DCS_CMD_READ_MEMORY_START     = 0x2E,
	DSI_DCS_CMD_SET_3D_CONTROL        = 0x3D,
	DSI_DCS_CMD_SET_ADDRESS_MODE      = 0x36,
	DSI_DCS_CMD_SET_COLUMN_ADDRESS    = 0x2A,
	DSI_DCS_CMD_SET_DISPLAY_OFF       = 0x28,
	DSI_DCS_CMD_SET_DISPLAY_ON        = 0x29,
	DSI_DCS_CMD_SET_GAMMA_CURVE       = 0x26,
	DSI_DCS_CMD_SET_PAGE_ADDRESS      = 0x2B,
	DSI_DCS_CMD_SET_PARTIAL_COLUMNS   = 0x31,
	DSI_DCS_CMD_SET_PARTIAL_ROWS      = 0x30,
	DSI_DCS_CMD_SET_PIXEL_FORMAT      = 0x3A,
	DSI_DCS_CMD_SET_SCROLL_AREA       = 0x33,
	DSI_DCS_CMD_SET_SCROLL_START      = 0x37,
	DSI_DCS_CMD_SET_TEAR_OFF          = 0x34,
	DSI_DCS_CMD_SET_TEAR_ON           = 0x35,
	DSI_DCS_CMD_SET_TEAR_SCANLINE     = 0x44,
	DSI_DCS_CMD_SET_VSYNC_TIMING      = 0x40,
	DSI_DCS_CMD_SOFT_RESET            = 0x01,
	DSI_DCS_CMD_WRITE_LUT             = 0x2D,
	DSI_DCS_CMD_WRITE_MEMORY_CONTINUE = 0x3C,
	DSI_DCS_CMD_WRITE_MEMORY_START    = 0x2C,
} dsi_dcs_cmd_t;


static inline
bool dsi_cmd_fifo_empty(void) {
	return (bool)(DSI_GPSR & DSI_GPSR_CMDFE);
}
static inline
bool dsi_read_fifo_empty(void) {
	return (bool)(DSI_GPSR & DSI_GPSR_PRDFE);
}
static inline
bool dsi_write_fifo_empty(void) {
	return (bool)(DSI_GPSR & DSI_GPSR_PWRFE);
}

/* DSI_GHCR:  DSI Host generic header configuration */
static inline void dsi_ghcr_config(
		uint32_t channel_id,
		dsi_ghcr_dt_t data_type,
		uint8_t  wc_lsb,
		uint8_t  wc_msb
) {
	/* Update the DSI packet header with new information */
	DSI_GHCR =
		((wc_msb & DSI_GHCR_WCMSB_MASK) << DSI_GHCR_WCMSB_SHIFT)
	  | ((wc_lsb & DSI_GHCR_WCLSB_MASK) << DSI_GHCR_WCLSB_SHIFT)
	  | ((channel_id & DSI_GHCR_VCID_MASK) << DSI_GHCR_VCID_SHIFT)
	  | ((data_type & DSI_GHCR_DT_MASK) << DSI_GHCR_DT_SHIFT);
}


static inline
void dsi_short_write(
		uint32_t channel_id,
		dsi_ghcr_dt_t data_type,
		uint8_t param_1,
		uint8_t param_2
) {
	/* Check the parameters */
	assert((data_type==DSI_GHCR_DT_SHORT_WRITE)
		|| (data_type==DSI_GHCR_DT_SHORT_WRITE_1_PARAM)
		|| (data_type==DSI_GHCR_DT_SHORT_WRITE_2_PARAMS)
		|| (data_type==DSI_GHCR_DT_DCS_SHORT_WRITE)
		|| (data_type==DSI_GHCR_DT_DCS_SHORT_WRITE_1_PARAM));

	/* Wait for Command FIFO Empty */
	while (!dsi_cmd_fifo_empty());

	/* Configure the packet to send a short DCS command with 0 or 1 parameter */
	dsi_ghcr_config(channel_id, data_type, param_1,param_2);
}

static inline
void dsi_long_write(
		uint32_t channel_id,
		dsi_ghcr_dt_t data_type,
		uint32_t param_count,
		uint32_t param_1,
		uint8_t* params
) {
//	uint32_t uicounter = 0;
	uint8_t* params_start = params;
	uint8_t* params_end   = params_start + param_count;

	/* Check the parameters */
	assert((data_type==DSI_GHCR_DT_SHORT_WRITE)
		|| (data_type==DSI_GHCR_DT_SHORT_WRITE_1_PARAM)
		|| (data_type==DSI_GHCR_DT_SHORT_WRITE_2_PARAMS)
		|| (data_type==DSI_GHCR_DT_LONG_WRITE)
		|| (data_type==DSI_GHCR_DT_DCS_SHORT_WRITE)
		|| (data_type==DSI_GHCR_DT_DCS_SHORT_WRITE_1_PARAM)
		|| (data_type==DSI_GHCR_DT_DCS_LONG_WRITE)
		|| (data_type==DSI_GHCR_DT_SET_MAX_RETURN_PKT_SIZE));
	//assert(param_count+1<=max_param_count);

	/* Wait for Command FIFO Empty */
	while (!dsi_cmd_fifo_empty());

	/* Set the DCS code hexadecimal on payload byte 1, and the other parameters on the write FIFO command*/
	while (params < params_end) {
		if(params == params_start) {
			DSI_GPDR=
					(             param_1 |
					((uint32_t)(*(params))  << 8) |
					((uint32_t)(*(params+1))<<16) |
					((uint32_t)(*(params+2))<<24));
			params += 3;
		} else {
			DSI_GPDR =
					((uint32_t)(*(params)) |
					((uint32_t)(*(params+1)) << 8) |
					((uint32_t)(*(params+2)) << 16) |
					((uint32_t)(*(params+3)) << 24)
				);
			params += 4;
		}
	}

	/* Configure the packet to send a long DCS command */
	dsi_ghcr_config(
			channel_id,
			data_type,
			 ((param_count+1)&0x00FF),
			(((param_count+1)&0xFF00)>>8)
		);
}

static inline
void dsi_read(
		 uint32_t channel_id,
		 uint8_t* array,
		 uint32_t size,
		 dsi_ghcr_dt_t data_type,
		 uint32_t dcs_cmd,
		 uint8_t* params
) {
	/* Check the parameters */
	assert((data_type==DSI_GHCR_DT_READ)
		|| (data_type==DSI_GHCR_DT_READ_1_PARAM)
		|| (data_type==DSI_GHCR_DT_READ_2_PARAMS)
		|| (data_type==DSI_GHCR_DT_DCS_READ));
	//assert(param_count+1<=max_param_count);

	if (size > 2) {
		/* set max return packet size */
		dsi_short_write(channel_id, DSI_GHCR_DT_SET_MAX_RETURN_PKT_SIZE, ((size)&0xFF), (((size)>>8)&0xFF));
	}

	/* Configure the packet to read command */
	switch (data_type) {
		case DSI_GHCR_DT_DCS_READ :
			dsi_ghcr_config(channel_id, data_type, dcs_cmd, 0);
			break;
		case DSI_GHCR_DT_READ :
			dsi_ghcr_config(channel_id, data_type, 0, 0);
			break;
		case DSI_GHCR_DT_READ_1_PARAM :
			dsi_ghcr_config(channel_id, data_type, params[0], 0);
			break;
		case DSI_GHCR_DT_READ_2_PARAMS :
			dsi_ghcr_config(channel_id, data_type, params[0], params[1]);
			break;
		default :
			assert("Invalid data type" && 0);
			return;
	}

	/* Check that the payload read FIFO is not empty */
	while (dsi_read_fifo_empty());

	/* Read the first 4 bytes */
	*((uint32_t *)array) = DSI_GPDR;
	if (size <= 4) return;

	/* Get the remaining bytes if any */
	do {
		/* Decrement remaining size and increment array pointer */
		size  -= 4;
		array += 4;
		/* Check that the payload read FIFO is not empty */
		while (dsi_read_fifo_empty());
		/* Read 4 bytes */
		*((uint32_t *)array) = (DSI_GPDR);
	} while (size>4);
}





#endif /* DRIVERS_DSI_HELPER_FUNCTIONS_H_ */
