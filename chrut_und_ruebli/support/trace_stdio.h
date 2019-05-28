/*
 * trace_stdio.h
 *
 *  Created on: 29 Nov 2018
 *      Author: omeier
 */

#ifndef _TRACE_STDIO_H_
#define _TRACE_STDIO_H_

#include <stdint.h>

void trace_puts(const char *s, uint32_t max_len);
void trace_putc(const char c);

#endif /* _TRACE_STDIO_H_ */
