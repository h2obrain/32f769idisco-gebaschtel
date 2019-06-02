TARGET  = chruut_und_ruebli

# Find source files for different file endings
# in a path relative to this Makefile
SOURCES_DIRS += .
INCLUDE_DIRS += config

# paths to search for lib-related .mk-files (relative to the top-dir)
LIB_SRC_DIRS += lib/support lib/drivers lib/gfx
LIB_SRC_DIRS += lib/sled_hijack

 
DEVICE   = stm32f769nih6

USE_LTO  = 0
OPT      = -O0
#OPT     = -O3
#CFLAGS += -D_GNU_SOURCE
CFLAGS  += -DMALLOC_AREA_START=0xc0000000U -DMALLOC_AREA_SIZE=0x400000U

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
