/*
 * newlib_integration.c
 *
 *  Created on: 18 Jan 2015
 *      Author: Oliver Meier
 */

#include <support/newlib_integration.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

/**
 *  functions for newlib implementations
 *  	everyone has a different view on implementation..
 *  		http://www.billgatliff.com/newlib.html
 *  		http://www.embecosm.com/appnotes/ean9/ean9-howto-newlib-1.0.html
 */

putsf_table_t putsf_table[3] = {NULL,NULL,NULL};
int _write(int fd, const void *buf, size_t cnt);
__attribute__((externally_visible))
int _write(int fd, const void *buf, size_t cnt) {
	assert((fd>0) && (fd<3) && putsf_table[fd] && "\nprintf: setup putsf_table!"); // STDOUT_FILENO || STDERR_FILENO
	putsf_table[fd](buf,cnt);
	return cnt;
//	errno = EIO;
//	return -1;
}
//long _write_r (struct _reent *ptr, int fd, const void *buf, size_t cnt);
//long _write_r (struct _reent *ptr, int fd, const void *buf, size_t cnt) {
//	ptr < reentrant stuff..
//	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
////		assert(((char *)buf)[cnt]==0);
//		((char *)buf)[cnt] = 0;
//		codebase_puts(buf);
//		return cnt;
//	}
//	errno = EIO;
//	return -1;
//}

//void _exit (int status);
__attribute__((externally_visible))
void _exit (int status) {
	panic_mode_data_t panic_data = {
		.mode=SYSTEM_EXIT,
		.system_exit = {
			.status = status
		}
	};
	panic_mode_handler(&panic_data); // blocking forever here..
	while(1);
}



/* variables defined by the linker */
#if defined(MALLOC_AREA_START) && defined(MALLOC_AREA_SIZE)
#elif defined(BETTER_LD_VERSION)
extern unsigned _internal_heap_start;
extern unsigned _internal_heap_end;
#elif defined(EXTERNAL_RAM)
#define RAM_ADDR 0x20010000
#include <libopencm3/cm3/vector.h>
//extern unsigned _stack; // already defined in vector.h
#endif

#include <support/system.h>


/*
 * malloc implementation
 */
#include <malloc.h>
//
//see #include <unistd.h> with #define _COMPILING_NEWLIB defined
__attribute__((externally_visible))
caddr_t  _sbrk ( ptrdiff_t incr );
//__attribute__((externally_visible))
//void __malloc_lock(struct _reent *_r);
//__attribute__((externally_visible))
//void __malloc_unlock(struct _reent *_r);

/*
 * This function is called if malloc wants to extend its memory pool
 */
__attribute__((externally_visible))
caddr_t _sbrk ( ptrdiff_t incr ) {
#if defined(MALLOC_AREA_START) && defined(MALLOC_AREA_SIZE)
	static caddr_t heap_end_current = (caddr_t )     (((MALLOC_AREA_START)                 +7) & ~7);
	static caddr_t const heap_end   = (caddr_t const)(((MALLOC_AREA_START+MALLOC_AREA_SIZE)+7) & ~7);
#elif defined(BETTER_LD_VERSION)
	static caddr_t heap_end_current = (caddr_t )     &_internal_heap_start;
	static caddr_t const heap_end   = (caddr_t const)&_internal_heap_end;
#else
	static caddr_t heap_end_current = (caddr_t )     &_stack;
	static caddr_t const heap_end   = (caddr_t const) RAM_ADDR;
#endif
	caddr_t prev_heap_end, next_heap_end;
	prev_heap_end = heap_end_current;
	/* align data on a 8 byte boundary */
	next_heap_end = (caddr_t)(((size_t)heap_end_current + incr + 7) & ~7UL);
	if (next_heap_end > heap_end ) {
		/* heap overflow */
        errno = ENOMEM;
        return (caddr_t)NULL;
	}
	heap_end_current = next_heap_end;
	assert(((uint32_t)heap_end_current&7)==0);
	return prev_heap_end;
}

/*
 * Those functions are called before/after malloc is doing its thing
 * This should make malloc interrupt safe (maybe add a mutex!)
 */
static uint32_t _isr_disabled_by_malloc_count = 0;
static bool _isr_enabled_before_malloc = false;
__attribute__((externally_visible))
void __malloc_lock(struct _reent *_r) {
	(void)_r;
	if (cb_is_interrupts_enabled()) {
		cb_disable_interrupts();
		_isr_enabled_before_malloc = true;
	}
	if (_isr_enabled_before_malloc) {
		_isr_disabled_by_malloc_count++;
	}
}
__attribute__((externally_visible))
void __malloc_unlock(struct _reent *_r) {
	(void)_r;
	assert(!cb_is_interrupts_enabled());
	if (_isr_enabled_before_malloc) {
		assert(_isr_disabled_by_malloc_count);
		_isr_disabled_by_malloc_count--;
		if (!_isr_disabled_by_malloc_count) {
			_isr_enabled_before_malloc = false;
			cb_enable_interrupts();
		}
	}
}

void malloc_test() {
#define KILLME_SIZE (100*(1<<10))
	uint32_t i, *kill_me, **kill_me_ptr;
	kill_me = malloc(KILLME_SIZE);
	kill_me_ptr = (uint32_t **)kill_me;
	for (i=0; i<KILLME_SIZE/sizeof(uint32_t); i++) {
		*kill_me_ptr = malloc(sizeof(uint32_t));
		if (*kill_me_ptr)
			**kill_me_ptr = 0xAAAAAAAA;
		else
			break;
		kill_me_ptr++;
	}
	kill_me_ptr = (uint32_t **)kill_me;
	for (i=0; i<KILLME_SIZE/sizeof(uint32_t); i++) {
		if (*kill_me_ptr)
			free(*kill_me_ptr);
		else
			break;
		kill_me_ptr++;
	}
	free(kill_me);
	kill_me = malloc(KILLME_SIZE);
	kill_me_ptr = malloc(0x153E8);
	if (kill_me)     free(kill_me);
	if (kill_me_ptr) free(kill_me_ptr);
}

#ifdef MALLOC_TESTS
/**
 * Do not use the system after this test as it completely segments the whole memory
 */
#include <malloc.h>
#include "random.h"
void malloc_test2() {
	struct mallinfo info = mallinfo(); (void)info.arena;
	int32_t momsizei = 0x200000; //(0x800000 - (int32_t)info.uordblks);
	assert(momsizei>0);
	size_t momsize = (size_t)momsizei * 3/4;
	size_t totsize = 0;

//	void *mem_for_printf;
	uint64_t t0,to;
    t0 = to = mtime();
	printf("\n ");
	fflush(stdout);
	int16_t cx = gfx_get_cursor_x();
	int16_t cy = gfx_get_cursor_y();
	uint64_t bc = 0, mm = 0, mf = 0, mfr = 0;
#define NUM_BUFFERS 100
	uint8_t *buffers[NUM_BUFFERS] = {NULL}; size_t buffer_sizes[NUM_BUFFERS] = {0};
	volatile int *e = __errno(); (void)e;
	size_t bi = 0;
	bool running = true;
	while (running) {
		bi = (size_t)nearbyint(random_float(0,NUM_BUFFERS-1));
		switch ((int)nearbyint(random_float(0,10))) {
			// allocate
			case 0:
//				assert((mem_for_printf = malloc(100*1024))) ;
				if (totsize >= momsize) continue;
				buffer_sizes[bi] = (uint32_t)nearbyintf(random_float(0,0x10000)); // free memory is around 0x400000
				buffers[bi] = malloc(buffer_sizes[bi]); mm++;
//				free(mem_for_printf);
				if (buffers[bi]==NULL) {
					buffer_sizes[bi] = 0;
					mf++;
				} else {
					assert(((size_t)buffers[bi]&7) == 0);
					totsize += buffer_sizes[bi];
				}
				break;
			// free
			case 1:
				if (buffers[bi] != NULL) {
					assert(((size_t)buffers[bi]>SDRAM_BASE_ADDRESS)&&((size_t)buffers[bi]<SDRAM_BASE_ADDRESS+0x800000));
					free(buffers[bi]);
					totsize -= buffer_sizes[bi];
					buffers[bi] = 0;
					buffer_sizes[bi] = 0;
				}
				break;
			// write/read
			default :
				if (buffers[bi] != NULL) {
					bc += buffer_sizes[bi];
					memset(buffers[bi],'x',buffer_sizes[bi]);
					uint32_t *bb = (uint32_t *)buffers[bi];
					union {
						char a[4];
						uint32_t aa;
					} a = { .a="xxxx" };
					uint32_t r_u32 = buffer_sizes[bi] / 4;
					for (size_t ccc=0; ccc<r_u32; ccc++) {
						if (a.aa != *bb++) mfr++;
					}
					memset(buffers[bi],'x',buffer_sizes[bi]);
				}
				break;
		}
		if ((to < mtime()) && !ili9341_is_reloading()) {
			to = mtime() + 100;
			double time  = (double)(mtime()-t0) / 1000;
			if (time>60) {
				running = false;
				for (bi = 0; bi < NUM_BUFFERS; bi++) {
					free(buffers[bi]);
					buffers[bi] = NULL;
					buffer_sizes[bi] = 0;
				}
			}
			double total = (double)bc /1024/1024;
			gfx_fill_rect(cx,cy,320-cx,240-cy,GFX_COLOR_BLACK);
			gfx_set_cursor(cx,cy);
			malloc_stats();
			fflush(stdout);
			printf(
					"t: % 3.1f  rw: %.3f Mb/s | %.1f Mb\n"
					"rwf:%llu ac:%lluk/%lluk a:%lu/%lukB\n"
					,
					time, total / time, total,
					mfr, mf/1000, mm/1000, (uint32_t)totsize/1024, (uint32_t)momsize/1024
				);

			fflush(stdout);
			ili9341_reload(0);
		}
	}
	printf(">> done");
	fflush(stdout);
	ili9341_reload(0);
	while (1);

	//	gfx_set_cursor(5,gfx_get_cursor_y());
}
#endif


/* Unimplemented calls - aka Schnauze!*/
/*
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int _kill (pid_t pid, int sig);
pid_t _getpid (void);
int _close(int file);
int _fstat(int file, void *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _read(int file, char *ptr, int len);

#undef errno
int errno;

int _kill (pid_t pid, int sig) {
	(void)pid;(void)sig;
	return -1;
}
pid_t _getpid (void) {
	errno = ENOSYS;
	return -1;
}
int _close(int file) {
	(void)file;
	errno = ENOSYS;
	return -1;
}
int _fstat(int file, void *st) {
	(void)file;(void)st;
	errno = ENOSYS;
	return -1;
}
int _isatty(int file) {
	(void)file;
	errno = ENOSYS;
	return 0;
}
int _lseek(int file, int ptr, int dir) {
	(void)file;(void)ptr;(void)dir;
	errno = ENOSYS;
	return -1;
}
int _read(int file, char *ptr, int len) {
	(void)file;(void)ptr;(void)len;
	errno = ENOSYS;
	return -1;
}
*/
