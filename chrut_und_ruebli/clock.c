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

/*
 * Now this is just the clock setup code from systick-blink as it is the
 * transferrable part.
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

/* Common function descriptions */
#include "clock.h"


/*
 * clock_setup(void)
 *
 * This function sets up both the base board clock rate
 * and a 1khz "system tick" count. The SYSTICK counter is
 * a standard feature of the Cortex-M series.
 */
void clock_setup(void)
{
	/* Setup external clock */
	RCC_CR |= RCC_CR_HSEBYP; // External clock is oscillator, not a quartz
	rcc_clock_setup_hse(&CLOCK_SETUP, HSE_FREQUENCY_MHZ);

	/* clock rate / CLOCK_SETUP.ahb_frequency * 1000 to get 1mS interrupt rate */
	systick_set_reload(CLOCK_SETUP.ahb_frequency/1000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();

	/* this done last */
	systick_interrupt_enable();
}

/* milliseconds since boot */
static volatile uint64_t system_millis;

/* Called when systick fires */
void sys_tick_handler(void)
{
	system_millis++;
}

/* simple sleep for delay milliseconds */
void msleep(uint32_t delay)
{
	uint64_t wake = system_millis + delay;
	while (wake > system_millis) {
		continue;
	}
}

/* Getter function for the current time */
uint64_t mtime(void)
{
	return system_millis;
}


/*
 * Delay functions which are not using clock :)
 */
#define CYCLES_PER_LOOP 1
void wait_cycles(uint32_t n)
{
	uint32_t l = n/CYCLES_PER_LOOP;
	if (!l) return;
	__asm__ __volatile__("0:" "SUBS %[count], 1;" "BNE 0b;":[count]"+r"(l));
}

void msleep_loop(uint32_t ms)
{
	wait_cycles(CLOCK_SETUP.ahb_frequency / 1000 * ms);
}
