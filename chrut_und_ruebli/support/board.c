/*
 * This file is part of the libopencm3 project.
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
 
#include "board.h"

/**
 * Simplifying functions
 */

/* zero terminated list of pin-configs */
void board_setup_clocks(uint32_t* clocks)
{
	while (*clocks) {
		rcc_periph_clock_enable(*clocks);
		clocks++;
	}
	/* some clocks need some time before we can write to the registers
	 * (see errata sheet (it's only 1-2 cpu cycles..)
	 */
	__asm__ volatile ("dsb");
}

/* zero terminated list of pin-configs */
void board_setup_pins(pin_setup_t *pin_configs)
{
	while (pin_configs->GPIO) {
		/* mask out pins which are configured later in the array */
		pin_setup_t *pin_configs_following = pin_configs+1;
		while (pin_configs_following->GPIO) {
			if (pin_configs->GPIO == pin_configs_following->GPIO) {
				pin_configs->GPIO_PINS &= ~(pin_configs->GPIO_PINS & pin_configs_following->GPIO_PINS);
			}
			pin_configs_following++;
		}

		/* setup the pin(s) */
		if (pin_configs->GPIO_PINS) {
			gpio_mode_setup(pin_configs->GPIO, pin_configs->mode, pin_configs->pullup_type, pin_configs->GPIO_PINS);
			if ((pin_configs->mode == GPIO_MODE_OUTPUT) || (pin_configs->mode == GPIO_MODE_AF))
				gpio_set_output_options(pin_configs->GPIO, pin_configs->out_mode.otype, pin_configs->out_mode.ospeed, pin_configs->GPIO_PINS);
			if  (pin_configs->mode == GPIO_MODE_AF)
				gpio_set_af(pin_configs->GPIO, pin_configs->af_mode.gpioaf, pin_configs->GPIO_PINS);
		}

		pin_configs++;
	}
}
