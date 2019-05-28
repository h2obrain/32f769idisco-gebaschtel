/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Chuck McManis <cmcmanis@mcmanis.com>
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

#include "clock.h"
#include "sdram.h"
#include <assert.h>

#define max(x,y) (x>y?x:y)
/**
 * copied from EmcraftSystems/u-boot
 */
#define NS2CLK(ns) ((uint32_t)((uint64_t)ns*CLOCK_SETUP.ahb_frequency/1000000000))
#define SDRAM_TRRD	NS2CLK(12)
#define SDRAM_TRCD	NS2CLK(18)
#define SDRAM_TRP	NS2CLK(18)
#define SDRAM_TRAS	NS2CLK(42)
#define SDRAM_TRC	NS2CLK(60)
#define SDRAM_TRFC	NS2CLK(60)
#define SDRAM_TCDL	(1 - 1)
#define SDRAM_TRDL	NS2CLK(12)
#define SDRAM_TBDL	(1 - 1)
#define SDRAM_TREF	(NS2CLK(64000000 / 8192) - 20)
#define SDRAM_TCCD	(1 - 1)
#define SDRAM_TXSR	SDRAM_TRFC	/* Row cycle time after precharge */
#define SDRAM_TMRD	1		    /* Page 10, Mode Register Set */
/* Last data in to row precharge, need also comply ineq on page 1648 */
#define SDRAM_TWR	max(\
	(int)max((int)SDRAM_TRDL, (int)(SDRAM_TRAS - SDRAM_TRCD)), \
	(int)(SDRAM_TRC - SDRAM_TRCD - SDRAM_TRP)\
)
/**/

/*
 * Initialize the SD RAM controller. (with default values)
 */
void
sdram_init(void) {
	/* Note the 32F769DISCOVERY board has the ram attached to bank 1 */
	/* Timing parameters computed for a 216Mhz clock */
	/* These parameters are specific to the SDRAM chip on the board */
	sdram_init_custom(
			SDRAM_BANK1,
			32, 12,8,
			(struct sdram_timing) {
				.trcd = SDRAM_TRCD,  /* RCD Delay */
				.trp  = SDRAM_TRP,   /* RP Delay */
				.twr  = SDRAM_TWR,   /* Write Recovery Time */
				.trc  = SDRAM_TRC,   /* Row Cycle Delay */
				.tras = SDRAM_TRAS,  /* Self Refresh Time */
				.txsr = SDRAM_TWR,   /* Exit Self Refresh Time */
				.tmrd = SDRAM_TMRD,  /* Load to Active Delay */
			},
			3,
			false, false, 2
		);
}
void
sdram_init_custom(
		enum fmc_sdram_bank sdram_bank,
		uint32_t memory_width,
		uint32_t row_count, uint32_t column_count,
		struct sdram_timing timing,
		uint32_t cas_latency,
		bool read_burst, bool write_burst,
		uint32_t burst_length
) {
	uint32_t cr_tmp, tr_tmp; /* control, timing registers */
	uint32_t cmd_tmp;        /* sdram command */

	cr_tmp = cmd_tmp = 0;

	cr_tmp |= FMC_SDCR_RPIPE_1CLK;
	cr_tmp |= FMC_SDCR_SDCLK_2HCLK;
	cr_tmp |= FMC_SDCR_NB4; // sdram has 4 internal banks
	switch (cas_latency) {
		case 2:
			cr_tmp  |= FMC_SDCR_CAS_2CYC;
			cmd_tmp |= SDRAM_MODE_CAS_LATENCY_2;
			break;
		case 3:
			cr_tmp  |= FMC_SDCR_CAS_3CYC;
			cmd_tmp |= SDRAM_MODE_CAS_LATENCY_3;
			break;
		default:
			assert("Invalid CAS latency");
			break;
	}
	switch (memory_width) {
		case 8:
			cr_tmp |= FMC_SDCR_MWID_8b;
			break;
		case 16:
			cr_tmp |= FMC_SDCR_MWID_16b;
			break;
		case 32:
			cr_tmp |= FMC_SDCR_MWID_32b;
			break;
		default:
			assert("Invalid memory width" && 0);
			break;
	}
	switch (row_count) {
		case 11:
			cr_tmp |= FMC_SDCR_NR_11;
			break;
		case 12:
			cr_tmp |= FMC_SDCR_NR_12;
			break;
		case 13:
			cr_tmp |= FMC_SDCR_NR_13;
			break;
		default:
			assert("Invalid row count" && 0);
			break;
	}
	switch (column_count) {
		case 8:
			cr_tmp |= FMC_SDCR_NC_8;
			break;
		case 9:
			cr_tmp |= FMC_SDCR_NC_9;
			break;
		case 10:
			cr_tmp |= FMC_SDCR_NC_10;
			break;
		case 11:
			cr_tmp |= FMC_SDCR_NC_11;
			break;
		default:
			assert("Invalid column count" && 0);
			break;
	}
	if (read_burst) {
		cr_tmp |= FMC_SDCR_RBURST;
	}

	/* If we're programming BANK 2, but per the manual some of the parameters
	 * only work in CR1 and TR1 so we pull those off and put them in the
	 * right place.
	 */
	tr_tmp     = sdram_timing(&timing);
	if ((sdram_bank==SDRAM_BANK1)||(sdram_bank==SDRAM_BOTH_BANKS)) {
		FMC_SDCR1 = cr_tmp;
		FMC_SDTR1 = tr_tmp;
	} else
	if (sdram_bank==SDRAM_BANK2) {
		FMC_SDCR1 |= (cr_tmp & FMC_SDCR_DNC_MASK);
		FMC_SDTR1 |= (tr_tmp & FMC_SDTR_DNC_MASK);
	} else {
		assert(0);
	}
	if ((sdram_bank==SDRAM_BANK2)||(sdram_bank==SDRAM_BOTH_BANKS)) {
		FMC_SDCR2 = cr_tmp;
		FMC_SDTR2 = tr_tmp;
	}

	/* Now start up the Controller per the manual
	 *	- Clock config enable
	 *	- PALL state
	 *	- set auto refresh
	 *	- Load the Mode Register
	 */
	sdram_command(sdram_bank, SDRAM_CLK_CONF, 1, 0);
	msleep_loop(1); /* sleep at least 200uS */
	sdram_command(sdram_bank, SDRAM_PALL, 1, 0);
	msleep_loop(1); /* ? sleep >100uS */
	sdram_command(sdram_bank, SDRAM_AUTO_REFRESH, 7, 0);
	msleep_loop(1); /* ?? sleep >100uS */

	cmd_tmp |= SDRAM_MODE_BURST_TYPE_SEQUENTIAL |
	           SDRAM_MODE_OPERATING_MODE_STANDARD;
	if (write_burst) {
		cmd_tmp |= SDRAM_MODE_WRITEBURST_MODE_PROGRAMMED;
	} else {
		cmd_tmp |= SDRAM_MODE_WRITEBURST_MODE_SINGLE;
	}
	switch (burst_length) {
		case 1:
			cmd_tmp |= SDRAM_MODE_BURST_LENGTH_1;
			break;
		case 2:
			cmd_tmp |= SDRAM_MODE_BURST_LENGTH_2;
			break;
		case 4:
			cmd_tmp |= SDRAM_MODE_BURST_LENGTH_4;
			break;
		case 8:
			cmd_tmp |= SDRAM_MODE_BURST_LENGTH_8;
			break;
		default:
			assert("Invalid burst length");
			break;
	}
	sdram_command(sdram_bank, SDRAM_LOAD_MODE, 1, cmd_tmp);


	msleep_loop(1); /* ?? sleep >100uS */
	sdram_command(sdram_bank, SDRAM_NORMAL, 0, 0);

	/*
	 * set the refresh counter to insure we kick off an
	 * auto refresh often enough to prevent data loss.
	 */
//	FMC_SDRTR = 683; // 64mbit
//	FMC_SDRTR = 683; // 512mbit // 64ms/(1<<13)*180MHz/2-20
	FMC_SDRTR = 683; // 128mbit // 64ms/(1<<13)*216MHz/2-20
	/* and Poof! a 16 megabytes of ram shows up in the address space */
}
