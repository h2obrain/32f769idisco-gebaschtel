# Cheap way to build libopencm3
ifndef TOP_DIR
$(error TOP_DIR needs to be set before including opencm3.mk)
endif

include $(TOP_DIR)/tools/defaults.mk
include $(TOP_DIR)/tools/functions.mk

ifndef OPENCM3_DIR
$(error OPENCM3_DIR needs to be set before including opencm3.mk (in defaults.mk))
endif

# libopencm3 build targets and sources
INCLUDES +=-I$(OPENCM3_DIR)/include

# TODO find absolute way!
OPENCM3_LIBNAME ?=opencm3_$(genlink_family)

# Find libopencm3 target folder
#  this tries to match LIBNAME to manufacturer or manufacturer/series folder in lib/
#  eg. LIBNAME=lm3s LIBFOLDER=lib/lm3s or LIBNAME=stm32f7 LIBFOLDER=lib/stm32/f7
OPENCM3_TARGET=$(strip \
    $(foreach M,$(filter-out usb ethernet dispatch,$(call DIR,$(OPENCM3_DIR)/lib)), \
        $(if $(subst opencm3_$M,,$(OPENCM3_LIBNAME)),\
            $(foreach S,$(call DIR,$(OPENCM3_DIR)/lib/$(M)), \
                $(if $(subst opencm3_$M$S,,$(OPENCM3_LIBNAME)),,$M/$S)), \
            $M)))
ifeq ($(OPENCM3_TARGET),)
$(error OPENCM3_TARGET for $(OPENCM3_LIBNAME) was not found!)
# do it with OPENCM3_LIBNAME=opencm3_$(genlink_subfamily) or so..
endif
# exceptions
ifeq ($(OPENCM3_LIBDIR),lib/lpc43xx)
OPENCM3_LIBDIR=lib/lpc43xx/m4
endif

# generate nvic header and add dependency
OPENCM3_NVIC_HEADER = $(OPENCM3_DIR)/include/libopencm3/$(OPENCM3_TARGET)/nvic.h
OPENCM3_IRQ_DEFN_FILE = $(wildcard $(OPENCM3_DIR)/include/libopencm3/$(OPENCM3_TARGET)/irq.json)
$(OPENCM3_NVIC_HEADER): $(OPENCM3_IRQ_DEFN_FILE)
	@echo "  GENHDR   $<"
	@cd $(OPENCM3_DIR)/ && ./scripts/irq2nvic_h ./$(subst $(OPENCM3_DIR)/,,$<)
DEPS += $(OPENCM3_NVIC_HEADER)

# parse target makefile for needed sources
SRCS += $(shell $(TOP_DIR)/lib/opencm3_extract_sources.py $(OPENCM3_DIR)/lib/$(OPENCM3_TARGET))
ifneq ($(.SHELLSTATUS),0)
  $(error $(TOP_DIR)/lib/opencm3_extract_sources.py command failed!)
endif
