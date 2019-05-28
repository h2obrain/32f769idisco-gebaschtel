/*
 * system.h
 *
 *  Created on: 30 Jan 2016
 *      Author: Oliver Meier
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <inttypes.h>
#include <stdio.h>
#include <libopencmsis/core_cm3.h>

#include "newlib_integration.h"
#include "debug.h"
#include "trace_stdio.h"
#include "../clock.h"

static inline
void system_setup(void) {
	debug_setup();
	const char msg[] = "\nEnabling printf on swo trace\n";
	trace_puts(msg,sizeof(msg));
	putsf_table[STDOUT_FILENO] = trace_puts;
	// do not use printf until ram was initialized!
//	printf("Printf on swo trace\n");
#if 0
	printf("Testing overflows..\n");
	fflush(stdout);
	for (uint32_t j=0; j<80; j++) {
		trace_putc((j%10)+48);
		trace_putc(' ');
		for (uint32_t i=0; i<80; i++) {
			trace_putc((i%10)+48);
		}
		trace_putc('\n');
	}
#endif
}

// simple static extension functions
__attribute__((always_inline))
static inline void cb_enable_interrupts(void) {
	if (!panic_mode_active)  __enable_irq();
}
__attribute__((always_inline))
static inline void cb_disable_interrupts(void) {
	__disable_irq();
}
__attribute__((always_inline))
static inline bool cb_is_interrupts_enabled(void) {
	return !cm_is_masked_interrupts();
}

__attribute__((always_inline))
static inline bool cb_is_interrupt(void) {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE) != 0 ;
}

static inline void cb_system_reset(void) {
	__asm__ volatile ("dsb");                                    /* Ensure all outstanding memory accesses included
															        buffered write are completed before reset */
	SCB->AIRCR  = (SCB_AIRCR_VECTKEY      |
				 (SCB->AIRCR & SCB_AIRCR_PRIGROUP_MASK) |
				 SCB_AIRCR_SYSRESETREQ);                         /* Keep priority group unchanged */
	__asm__ volatile ("dsb");                                    /* Ensure completion of memory access */
	while(1);   /* wait until reset */
}

// board
#include "board.h"

#endif /* _SYSTEM_H_ */
