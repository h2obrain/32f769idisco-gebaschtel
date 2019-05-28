/*
 * trace_stdio.c
 *
 *  Created on: 29 Nov 2018
 *      Author: omeier
 */

#include "trace_stdio.h"
#include "trace.h"

#ifndef STIMULUS_STDIO
#define STIMULUS_STDIO 0
#endif

#define PREVENT_OVERFLOWS
#ifdef PREVENT_OVERFLOWS
#include "../clock.h"
#define WAIT_INTERVAL 1
#define WAIT_CYCLES   (250000*WAIT_INTERVAL)
static void wait_if_needed(void) {
	static uint32_t wc = 0;
	if (wc==WAIT_INTERVAL-1) {
		wc = 0;
		wait_cycles(WAIT_CYCLES);
	} else {
		wc++;
	}
}
#endif

void trace_puts(const char *s, uint32_t max_len) {
	while ((*s!=0) && (max_len--)) {
		trace_putc(*s++);
	}
}

void trace_putc(const char c) {
//	if (c == '\n') {
//		trace_send_blocking8(STIMULUS_STDIO, '\r');
//#ifdef PREVENT_OVERFLOWS
//		wait_if_needed();
//#endif
//	}
	trace_send_blocking8(STIMULUS_STDIO, c);
#ifdef PREVENT_OVERFLOWS
	wait_if_needed();
#endif
}




