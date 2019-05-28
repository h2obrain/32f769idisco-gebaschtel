TARGET  = chruut_und_ruebli

# Find source files (SRCS) for different file endings
SRCS   += $(wildcard *.c)
SOURCES_DIRS += support
SOURCES_DIRS += drivers
SOURCES_DIRS += gfx/fonts gfx/gfx gfx/vector_gfx
 
DEVICE   = stm32f769nih6

USE_LTO  = false
OPT      = -O0
#OPT     = -O3
#CFLAGS += -D_GNU_SOURCE

LDLIBS   = -lm


OOCD           = openocd
OOCD_INTERFACE = stlink-v2-1
OOCD_TARGET    = stm32f7x


# setup malloc for external ram
# see support/newlib_integration
# the first ram-bank is reserved as display memory 
#CFLAGS += -DMALLOC_AREA_START=0xC2000000U -DMALLOC_AREA_SIZE=0x8000000U

TOP_DIR=..
include $(TOP_DIR)/tools/rules.mk
