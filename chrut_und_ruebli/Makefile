TARGET  = chruut_und_ruebli
TOP_DIR =..

# Find source files for different file endings
# in a path relative to this Makefile
SOURCES_DIRS += .
INCLUDE_DIRS += config

# paths to search for lib-related .mk-files (relative to the top-dir)
LIB_SRC_DIRS += data
LIB_SRC_DIRS += lib/support lib/drivers lib/gfx
#LIB_SRC_DIRS += lib/sled_hijack
 
DEVICE   = stm32f769nih6

USE_LTO  = false
OPT      = -Og

#CFLAGS  += --specs=nosys.specs
#CFLAGS  += -D_GNU_SOURCE

#DEFS    += -DVIDEO_MEMORY_ADDRESS=0xc0000000U -DVIDEO_MEMORY_SIZE="(2*800*480*sizeof(uint32_t))"
#DEFS    += -DMALLOC_AREA_START="(VIDEO_MEMORY_ADDRESS+VIDEO_MEMORY_SIZE)" -DMALLOC_AREA_SIZE="(0x1000000U-VIDEO_MEMORY_SIZE)" #0x400000U
DEFS    += -DMALLOC_AREA_START=0xc0000000U -DMALLOC_AREA_SIZE=0x1000000U #0x400000U

LDLIBS  += -lm

## harfbuzz and freetype2  #
LIB_SRC_DIRS += lib/freetype2
LIB_SRC_DIRS += lib/harfbuzz
LDLIBS       += -lstdc++ #_nano
## freetype-gl  #
LIB_SRC_DIRS += lib/freetype-gl
###



OOCD           = openocd
OOCD_INTERFACE = stlink-v2-1
OOCD_TARGET    = stm32f7x


# setup malloc for external ram
# see support/newlib_integration
# the first ram-bank is reserved as display memory 
#CFLAGS += -DMALLOC_AREA_START=0xC2000000U -DMALLOC_AREA_SIZE=0x8000000U

include $(TOP_DIR)/tools/rules.mk
