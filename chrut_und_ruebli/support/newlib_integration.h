/*
 * newlib_integration.h
 *
 *  Created on: 18 Jan 2015
 *      Author: Oliver Meier
 */

#ifndef _NEWLIB_INTEGRATION_H_
#define _NEWLIB_INTEGRATION_H_


#include "debug.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#ifndef NULL
#define NULL 0
#endif

typedef void (*putsf_table_t)(const char *buf, uint32_t max_len);
extern putsf_table_t putsf_table[];
void malloc_test(void);
void malloc_test2(void);

#endif /* _NEWLIB_INTEGRATION_H_ */
