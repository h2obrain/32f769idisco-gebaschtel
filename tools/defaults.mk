####################################################################
#
# Defaults
#
####################################################################
# Output
#  BIN_DIR
#  TOP_DIR_REAL
#  CURDIR_FROM_TOP
#  OELF,OMAP,OBIN,OHEX,OSREC,OLIST
#  ...

ifndef DEFAULTS_MK

ifndef TOP_DIR
$(error TOP_DIR needs to be set before including defaults.mk)
endif

# Be silent per default, but 'make V=1' will show all compiler calls.
ifeq ($(filter-out 0,$(V)),)
Q    := @
NULL := 2>/dev/null
endif

####################################################################
# default paths
#

TARGET      ?=my_target

OUTPUT_DIR  ?=.
OBJ_DIR     ?=$(OUTPUT_DIR)/obj
BIN_DIR     ?=$(OUTPUT_DIR)/bin

OELF        ?=$(BIN_DIR)/$(TARGET).elf
OMAP        ?=$(BIN_DIR)/$(TARGET).map
OBIN        ?=$(BIN_DIR)/$(TARGET).bin
OHEX        ?=$(BIN_DIR)/$(TARGET).hex
OSREC       ?=$(BIN_DIR)/$(TARGET).srec
OLIST       ?=$(BIN_DIR)/$(TARGET).list

OPENCM3_DIR ?=$(TOP_DIR)/lib/opencm3

####################################################################
# Executables
#
PREFIX  ?= arm-none-eabi-

CC      :=$(PREFIX)gcc
CXX     :=$(PREFIX)g++
LD      :=$(PREFIX)gcc
NM      :=$(PREFIX)gcc-nm
AR      :=$(PREFIX)gcc-ar
RANLIB  :=$(PREFIX)gcc-ranlib
AS      :=$(PREFIX)as
OBJCOPY :=$(PREFIX)objcopy
OBJDUMP :=$(PREFIX)objdump
GDB     :=$(PREFIX)gdb

# Default options
OPT     ?=-O3
DEBUG   ?=-ggdb3
CSTD    ?=-std=gnu18
#CSTD    ?=-std=c99
CXXSTD  ?=-std=gnu++17
USE_LTO ?=1

STFLASH ?= st-flash
#OOCD    ?= openocd

####################################################################
# extra..
#

#LIB_EXT    ?=.a


####################################################################
# Genlink config
#

##LDLIBS_:=$(LDLIBS)
##LIBDEPS_:=$(LIBDEPS)
#include $(OPENCM3_DIR)/mk/genlink-config.mk
##LDLIBS:=$(LDLIBS_)
##LIBDEPS:=$(LIBDEPS_)

LDSCRIPT       ?= $(OBJ_DIR)/generated.$(DEVICE).ld
DEVICES_DATA   ?= $(OPENCM3_DIR)/ld/devices.data

ifeq ($(DEVICE),)
$(error no DEVICE specified for linker script generator)
endif

GENLINK_VARS  = ARCH_FLAGS CPPFLAGS_MK
ifneq ($(filter undefined,$(foreach VAR,$(GENLINK_VARS),$(origin $(VAR)))),)

genlink_family    :=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) FAMILY)
genlink_subfamily :=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) SUBFAMILY)
genlink_cpu       :=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) CPU)
genlink_fpu       :=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) FPU)
genlink_cppflags  :=$(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) CPPFLAGS)

ifndef ARCH_FLAGS
ARCH_FLAGS     = -mcpu=$(genlink_cpu)
ifeq ($(genlink_cpu),$(filter $(genlink_cpu),cortex-m0 cortex-m0plus cortex-m3 cortex-m4 cortex-m7))
ARCH_FLAGS    += -mthumb
endif
ifeq ($(genlink_fpu),soft)
ARCH_FLAGS    += -msoft-float
else ifeq ($(genlink_fpu),hard-fpv4-sp-d16)
ARCH_FLAGS    += -mfloat-abi=hard -mfpu=fpv4-sp-d16
else ifeq ($(genlink_fpu),hard-fpv5-sp-d16)
ARCH_FLAGS    += -mfloat-abi=hard -mfpu=fpv5-sp-d16
else ifeq ($(genlink_fpu),hard-fpv5-dp-d16)
ARCH_FLAGS    += -mfloat-abi=hard -mfpu=fpv5-d16
else
$(error No match for the FPU flags)
endif
endif

endif


####################################################################
# Build variables

# C flags
ifndef CFLAGS_MK
CFLAGS_MK     = $(OPT) $(CSTD) $(DEBUG)
CFLAGS_MK    += $(ARCH_FLAGS)
CFLAGS_MK    += -Wextra -Wshadow -Wimplicit-function-declaration
CFLAGS_MK    += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
CFLAGS_MK    += -Wno-packed-bitfield-compat
CFLAGS_MK    += -fno-common -ffunction-sections -fdata-sections
endif

# C++ flags
ifndef CXXFLAGS_MK
CXXFLAGS_MK   = $(OPT) $(CXXSTD) $(DEBUG)
CXXFLAGS_MK  += $(ARCH_FLAGS)
CXXFLAGS_MK  += -Wextra -Wshadow -Wredundant-decls # -Weffc++
CXXFLAGS_MK  += -fno-common -ffunction-sections -fdata-sections
endif

# C & C++ preprocessor common flags
ifndef CPPFLAGS_MK
CPPFLAGS_MK   = -MD
CPPFLAGS_MK  += -Wall -Wundef
CPPFLAGS_MK  += $(DEFS)
ifneq ($(filter-out 0 false,$(USE_LTO)),)
CPPFLAGS_MK  += -flto
CPPFLAGS_MK  += -ffat-lto-objects
endif
CPPFLAGS_MK  += $(genlink_cppflags)
endif

# Assembler flags
ifndef ASMFLAGS_MK
ASMFLAGS_MK   = $(STD) $(OPT)
ifeq ($(genlink_cpu),$(filter $(genlink_cpu),cortex-m0 cortex-m0plus cortex-m3 cortex-m4 cortex-m7))
ASMFLAGS_MK  += -mthumb-interwork -mthumb
endif
ASMFLAGS_MK  += -fno-builtin
ASMFLAGS_MK  += -Wall
endif

# Linker flags
ifndef LDFLAGS_MK
LDFLAGS_MK    = --static -nostartfiles
LDFLAGS_MK   += -T$(LDSCRIPT)
LDFLAGS_MK   += $(ARCH_FLAGS) $(DEBUG)
ifneq ($(filter-out 0 false,$(USE_LTO)),)
LDFLAGS_MK   += -flto -fuse-linker-plugin
#LDFLAGS_MK   += -ffat-lto-objects
#LDFLAGS_MK   += -Wl,--allow-multiple-definition
#LDFLAGS_MK   += -nodefaultlibs -nostdlib
endif
LDFLAGS_MK   += -Wl,-Map=$(OMAP) -Wl,--cref
LDFLAGS_MK   += -Wl,--gc-sections
ifeq ($(V),99)
LDFLAGS_MK   += --print-gc-sections
endif
LDFLAGS_MK   += -Wl,--fatal-warnings
endif


############################
# always needed libraries

LDLIBS       += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

# paths to search for lib-related *.mk-files (relative to the top-dir)
LIB_SRC_DIRS += lib

endif
