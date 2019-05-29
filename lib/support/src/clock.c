#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

/* Common function descriptions */
#include <support/clock.h>

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
