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

#ifndef BOARD_H_
#define BOARD_H_
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


/**
 * Simplifying functions
 */

typedef struct {
	uint32_t GPIO;      // base address
	uint32_t GPIO_PINS; // flags
	// config
	uint8_t mode;
	uint8_t pullup_type;
	/*  */
	struct {       // if mode = af or output (i don't think af is needed here, but it's used in many samples, so i leave it..
		uint8_t otype;
		uint8_t ospeed;
	} out_mode;
	struct {        // if mode = af
		uint8_t gpioaf;
	} af_mode;
} pin_setup_t;

/* zero terminated list of pin-configs */
void board_setup_clocks(uint32_t* clocks);

/* zero terminated list of pin-configs */
void board_setup_pins(pin_setup_t *pin_configs);

#endif /* BOARD_H_ */
