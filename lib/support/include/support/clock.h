/*
 * delay.h
 *
 *  Created on: 29 May 2019
 *      Author: Oliver Meier
 */

#ifndef SUPPORT_CLOCK_H_
#define SUPPORT_CLOCK_H_

#include <stdint.h>
#include <clock_config.h>

/*
 * Definitions for functions being abstracted out
 */
void msleep(uint32_t);
uint64_t mtime(void);
void clock_setup(void);
/*
 * Delay functions which are not depending on interrupts
 */
void wait_cycles(uint32_t);
void msleep_loop(uint32_t);

#endif /* SUPPORT_CLOCK_H_ */


