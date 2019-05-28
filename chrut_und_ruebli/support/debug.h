/*
 * debug.h
 *
 *  Created on: 29 Oct 2015
 *      Author: Oliver Meier
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)

#define DEBUG_CLOCKS RCC_GPIOB

#define DEBUG_PINS \
	{ GPIOB,                                              \
		GPIO3,                                            \
		GPIO_MODE_AF     , GPIO_PUPD_NONE,                \
		.out_mode={ GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ  },  \
		.af_mode= { GPIO_AF0 }}                           \

typedef enum {
	/* --- SCB_CFSR values ----------------------------------------------------- */

	/* Bits [31:26]: reserved - must be kept cleared */
	/* DIVBYZERO: Divide by zero usage fault */
	SCF_DIVBYZERO			= (1 << 25),
	/* UNALIGNED: Unaligned access usage fault */
	SCF_UNALIGNED			= (1 << 24),
	/* Bits [23:20]: reserved - must be kept cleared */
	/* NOCP: No coprocessor usage fault */
	SCF_NOCP				= (1 << 19),
	/* INVPC: Invalid PC load usage fault */
	SCF_INVPC				= (1 << 18),
	/* INVSTATE: Invalid state usage fault */
	SCF_INVSTATE			= (1 << 17),
	/* UNDEFINSTR: Undefined instruction usage fault */
	SCF_UNDEFINSTR			= (1 << 16),
	/* BFARVALID: Bus Fault Address Register (BFAR), valid flag */
	SCF_BFARVALID			= (1 << 15),
	/* Bits [14:13]: reserved - must be kept cleared */
	/* STKERR: Bus fault on stacking for exception entry */
	SCF_STKERR				= (1 << 12),
	/* UNSTKERR: Bus fault on unstacking for a return from exception */
	SCF_UNSTKERR			= (1 << 11),
	/* IMPRECISERR: Imprecise data bus error */
	SCF_IMPRECISERR			= (1 << 10),
	/* PRECISERR: Precise data bus error */
	SCF_PRECISERR			= (1 << 9),
	/* IBUSERR: Instruction bus error */
	SCF_IBUSERR				= (1 << 8),
	/* MMARVALID: Memory Management Fault Address Register (MMAR) valid flag */
	SCF_MMARVALID			= (1 << 7),
	/* Bits [6:5]: reserved - must be kept cleared */
	/* MSTKERR: Memory manager fault on stacking for exception entry */
	SCF_MSTKERR				= (1 << 4),
	/* MUNSTKERR: Memory manager fault on unstacking for a return from exception */
	SCF_MUNSTKERR			= (1 << 3),
	/* Bit 2: reserved - must be kept cleared */
	/* DACCVIOL: Data access violation flag */
	SCF_DACCVIOL			= (1 << 1),
	/* IACCVIOL: Instruction access violation flag */
	SCF_IACCVIOL			= (1 << 0)
} scb_cfsr_fault_t;

typedef struct {
	scb_cfsr_fault_t fault;
	const char* title;
	const char* description;
} scb_cfsr_fault_description_t;

#define SCB_CFSR_FAULT_DESCRIPTIONS { \
		{ SCF_DIVBYZERO  , "DIVBYZERO", "Divide by zero usage fault"}, \
		{ SCF_UNALIGNED  , "UNALIGNED", "Unaligned access usage fault"}, \
		{ SCF_NOCP       , "NOCP", "No coprocessor usage fault"}, \
		{ SCF_INVPC      , "INVPC", "Invalid PC load usage fault"}, \
		{ SCF_INVSTATE   , "INVSTATE", "Invalid state usage fault"}, \
		{ SCF_UNDEFINSTR , "UNDEFINSTR", "Undefined instruction usage fault"}, \
		{ SCF_BFARVALID  , "BFARVALID", "Bus Fault Address Register (BFAR), valid flag"}, \
		{ SCF_STKERR     , "STKERR", "Bus fault on stacking for exception entry"}, \
		{ SCF_UNSTKERR   , "UNSTKERR", "Bus fault on unstacking for a return from exception"}, \
		{ SCF_IMPRECISERR, "IMPRECISERR", "Imprecise data bus error"}, \
		{ SCF_PRECISERR  , "PRECISERR", "Precise data bus error"}, \
		{ SCF_IBUSERR    , "IBUSERR", "Instruction bus error"}, \
		{ SCF_MMARVALID  , "MMARVALID", "Memory Management Fault Address Register (MMAR) valid flag"}, \
		{ SCF_MSTKERR    , "MSTKERR", "Memory manager fault on stacking for exception entry"}, \
		{ SCF_MUNSTKERR  , "MUNSTKERR", "Memory manager fault on unstacking for a return from exception"}, \
		{ SCF_DACCVIOL   , "DACCVIOL", "Data access violation flag"}, \
		{ SCF_IACCVIOL   , "IACCVIOL", "Instruction access violation flag"}, \
}

typedef enum {
	HARD_FAULT,
	SYSTEM_EXIT,
	ASSERTION_WRONG
} panic_mode_t;


typedef struct {
	union {
		scb_cfsr_fault_t cfsr;
		uint32_t       __cfsr;
	};
	uint32_t hfsr;
	uint32_t mmfar;
	uint32_t bfar;
	uint32_t r0,r1,r2,r3,r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
} pm_hard_fault_t;
typedef struct {
	int status;
	// used for filename in assertion_wrong
	const char message[40];
} pm_system_exit_t;
typedef struct {
	int line;
	// used for filename in assertion_wrong
	const char message[40];
} pm_assertion_wrong_t;
typedef struct {
	union {
		panic_mode_t mode;
		uint32_t   __mode;
	};
	union {
		pm_hard_fault_t hard_fault;
//		SCB_Type hard_fault;
		pm_system_exit_t system_exit;
		pm_assertion_wrong_t assertion_wrong;
	};
	uint32_t user_data;
} panic_mode_data_t;
typedef struct {
	const char *file;
	int line;
	const char *func;
	const char *assert_line;
} assert_user_data_t;

#pragma pack(pop)

void debug_setup(void);

extern volatile bool panic_mode_active;
void panic_mode_handler(panic_mode_data_t *data);
void panic_mode(panic_mode_data_t *data);

#define ENABLE_DEBUG_PRINTF
#ifdef ENABLE_DEBUG_PRINTF
void print_panic_mode_data(panic_mode_data_t *data);
#endif

#endif /* _DEBUG_H_ */
