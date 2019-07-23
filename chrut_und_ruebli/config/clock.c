/*
 * clock.c
 *
 *  Created on: 20 Jul 2019
 *      Author: h2obrain
 */

#include "clock_config.h"
const struct rcc_clock_scale rcc_3v3_200MHz =
	{ /* 200MHz */
		.plln = 400,
		.pllp = 2,
		.pllq = 9,
		.pllr = 0,
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_4,
		.ppre2 = RCC_CFGR_PPRE_DIV_2,
		.vos_scale = PWR_SCALE1,
		.overdrive = 1,
		.flash_waitstates = 7,
		.ahb_frequency  = 200000000,
		.apb1_frequency = 50000000,
		.apb2_frequency = 100000000,
	};
