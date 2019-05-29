/*
 * debug.c
 *
 *  Created on: 29 Oct 2015
 *      Author: Oliver Meier
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libopencmsis/core_cm3.h>
#include <libopencm3/cm3/nvic.h>
#include <support/debug.h>
#include <support/system.h>

void debug_setup() {
	/**
	 * WTF? this fixes hardfault error?
	 */
	nvic_set_priority(NVIC_HARD_FAULT_IRQ,  0);
	nvic_set_priority(NVIC_MEM_MANAGE_IRQ,  0);
	nvic_set_priority(NVIC_BUS_FAULT_IRQ,   0);
	nvic_set_priority(NVIC_USAGE_FAULT_IRQ, 0);
	/** end!? */

	//SCB_CCR |= SCB_CCR_DIV_0_TRP | SCB_CCR_UNALIGN_TRP;
	//SCB_CCR &= ~(SCB_CCR_DIV_0_TRP | SCB_CCR_UNALIGN_TRP);
}

volatile bool panic_mode_active = false;
static panic_mode_data_t panic_data = {0};
static assert_user_data_t assert_user_data = {0};

__attribute__ ((weak)) //,__noreturn__))
void panic_mode(panic_mode_data_t *data) {
	(void)data;
	abort();
}

void panic_mode_handler(panic_mode_data_t *data) {
	cb_disable_interrupts();
	if (panic_mode_active) {
		// TODO something meaningful
		return;
	}
	panic_mode_active = true;
	panic_mode(data);
	abort();
}

/* Note: hard_fault_handler_c must be global to be accessible from assembly code */
void hard_fault_handler_c(unsigned long *hardfault_args) __attribute__ ((__noreturn__));
// Use the 'naked' attribute so that C stacking is not used.
__attribute__((naked))
void hard_fault_handler(void){
	__asm( ".syntax unified\n"
	"MOVS R0, #4 \n"
	"MOV R1, LR \n"
	"TST R0, R1 \n"
	"BEQ _MSP \n"
	"MRS R0, PSP \n"
	"B hard_fault_handler_c \n"
	"_MSP: \n"
	"MRS R0, MSP \n"
	"B hard_fault_handler_c \n"
	".syntax divided\n") ;
}
__attribute__((externally_visible))
void hard_fault_handler_c(uint32_t *hardfault_args)  {
	panic_data.mode = HARD_FAULT;

	panic_data.hard_fault.cfsr  = SCB->CFSR;
	panic_data.hard_fault.hfsr  = SCB->HFSR;
	panic_data.hard_fault.mmfar = SCB->MMFAR;
	panic_data.hard_fault.bfar  = SCB->BFAR;

	if (hardfault_args) {
		panic_data.hard_fault.r0  = hardfault_args[0];
		panic_data.hard_fault.r1  = hardfault_args[1];
		panic_data.hard_fault.r2  = hardfault_args[2];
		panic_data.hard_fault.r3  = hardfault_args[3];
		panic_data.hard_fault.r12 = hardfault_args[4];
		panic_data.hard_fault.lr  = hardfault_args[5];
		panic_data.hard_fault.pc  = hardfault_args[6];
		panic_data.hard_fault.psr = hardfault_args[7];
	} else {
		memset(&panic_data.hard_fault.r0, 0, 8*sizeof(uint32_t));
	}

//	MEMCPY((void *)&panic_data.hard_fault, SCB, sizeof(SCB_Type));
	panic_mode_handler((panic_mode_data_t *)&panic_data);
	abort();
}
/* currently not set up.. */
__attribute__ ((__noreturn__,__naked__))
void mem_manage_handler() {
	hard_fault_handler();
//	hard_fault_handler_c(NULL);
}
__attribute__ ((__noreturn__,__naked__))
void bus_fault_handler() {
	hard_fault_handler();
//	hard_fault_handler_c(NULL);
}
__attribute__ ((__noreturn__,__naked__))
void usage_fault_handler() {
	hard_fault_handler();
//	hard_fault_handler_c(NULL);
}

//__attribute__ ((__noreturn__,__naked__))
__attribute__ ((__noreturn__))
void __assert_func(const char *file, int line, const char *func, const char *assert_line) {
	memcpy((assert_user_data_t *)&assert_user_data,
		   &(assert_user_data_t){
				.file=file, .line=line, .func=func, .assert_line=assert_line
			}, sizeof(assert_user_data_t));
	memcpy((panic_mode_data_t *)&panic_data, &(panic_mode_data_t) {
			.mode = ASSERTION_WRONG,
			.assertion_wrong = {
				.line=line,
			},
			.user_data = (uint32_t)&assert_user_data
		}, sizeof(panic_mode_data_t));
	size_t l = strnlen(file, (sizeof(panic_data.assertion_wrong.message)-1));
	if (l > (sizeof(panic_data.assertion_wrong.message)-1)) {
		file += l - (sizeof(panic_data.assertion_wrong.message)-1);
		l = (sizeof(panic_data.assertion_wrong.message)-1);
	}
	memcpy((char *)panic_data.assertion_wrong.message,file,l);
	((char *)panic_data.assertion_wrong.message)[(sizeof(panic_data.assertion_wrong.message)-1)] = 0;

	panic_mode_handler((panic_mode_data_t *)&panic_data);
	abort();
}

#ifdef ENABLE_DEBUG_PRINTF
#include <inttypes.h>
#include <stdio.h>
uint32_t fault_descs_count = sizeof((scb_cfsr_fault_description_t[])SCB_CFSR_FAULT_DESCRIPTIONS)/sizeof(scb_cfsr_fault_description_t);
scb_cfsr_fault_description_t fault_descs[] = SCB_CFSR_FAULT_DESCRIPTIONS;
void print_panic_mode_data(panic_mode_data_t *data) {
	printf("Panic!\n");
	switch (data->mode) {
		case HARD_FAULT :
			printf("HARD_FAULT\n");
			printf("Name     Type        Value               Description\n");
			printf("~~~~     ~~~~        ~~~~~               ~~~~~~~~~~~\n");
			printf("cfsr     uint32_t    0x%08"PRIX32" (Hex)    ",   data->hard_fault.__cfsr);
			for (uint32_t i=0; i<fault_descs_count; i++) {
				if (data->hard_fault.cfsr & fault_descs[i].fault) {
					printf(fault_descs[i].description);
					printf("\n");
				}
			}
			printf("hfsr     uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.hfsr);
			printf("mmfar    uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.mmfar);
			printf("bfar     uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.bfar);
			printf("r0       uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.r0);
			printf("r1       uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.r1);
			printf("r2       uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.r2);
			printf("r3       uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.r3);
			printf("r12      uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.r12);
			printf("lr       uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.lr);
			printf("pc       uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.pc);
			printf("psr      uint32_t    0x%08"PRIX32" (Hex)    \n", data->hard_fault.psr);
			break;
		case SYSTEM_EXIT :
			printf("SYSTEM_EXIT ");
			printf("%d\n",data->system_exit.status);
			break;
		case ASSERTION_WRONG :
			{
				assert_user_data_t *d = (assert_user_data_t *)data->user_data;
				printf( "ASSERTION_WRONG\n"
						"Filen    : %s\n"
						"Line     : %d\n"
						"Function : %s\n"
						"Assert   : %s\n",
						d->file,
						d->line,
						d->func,
						d->assert_line
					);
			}
			break;
	}
}
#endif
