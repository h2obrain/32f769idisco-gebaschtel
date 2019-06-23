TARGET  = chruut_und_ruebli
TOP_DIR =..

# Find source files for different file endings
# in a path relative to this Makefile
SOURCES_DIRS += .
INCLUDE_DIRS += config

# paths to search for lib-related .mk-files (relative to the top-dir)
LIB_SRC_DIRS += lib/support lib/drivers lib/gfx
#LIB_SRC_DIRS += lib/sled_hijack
 
DEVICE   = stm32f769nih6

USE_LTO  = false
OPT      = -O0
#CFLAGS += -D_GNU_SOURCE
DEFS    += -DMALLOC_AREA_START=0xc0000000U -DMALLOC_AREA_SIZE=0x400000U

LDLIBS   = -lm

## harfbuzz and freetype2  #
LIB_SRC_DIRS += lib/freetype2
LIB_SRC_DIRS += lib/harfbuzz
LDLIBS   += -lstdc++
###

OOCD           = openocd
OOCD_INTERFACE = stlink-v2-1
OOCD_TARGET    = stm32f7x


# setup malloc for external ram
# see support/newlib_integration
# the first ram-bank is reserved as display memory 
#CFLAGS += -DMALLOC_AREA_START=0xC2000000U -DMALLOC_AREA_SIZE=0x8000000U

include $(TOP_DIR)/tools/rules.mk
