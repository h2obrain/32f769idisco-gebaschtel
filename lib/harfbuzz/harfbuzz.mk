
TOP_DIR ?=.

HB_DIR  ?=$(MKF_DIR)/harfbuzz
HB_DIR  :=$(HB_DIR)

include $(HB_DIR)/src/Makefile.sources

HB_INCLUDES += -I$(TOP_DIR)/lib/config
HB_DEFS     += -DHAVE_CONFIG_H
HB_INCLUDES += -I$(HB_DIR)/src
#HB_DEFS     += -DHB_TINY -DHB_NO_OT_FONT

HB_INCS += $(filter %.hh, $(HB_BASE_sources)) $(filter %.h, $(HB_BASE_sources))
HB_INCS += $(filter %.hh, $(HB_BASE_RAGEL_GENERATED_sources))
HB_INCS += $(HB_FT_headers) $(filter %.hh, $(HB_FT_sources))
#HB_INCS += $(HB_GOBJECT_headers) $(filter %.hh, $(HB_GOBJECT_sources))

HB_SRCS += $(filter %.cc, $(HB_BASE_sources))
HB_SRCS += $(filter %.cc, $(HB_BASE_RAGEL_GENERATED_sources))
HB_SRCS += $(filter %.cc, $(HB_FT_sources))
#HB_SRCS += $(filter %.cc, $(HB_GOBJECT_sources))

HB_INCS := $(addprefix $(HB_DIR)/src/,$(sort $(HB_INCS)))
HB_SRCS := $(addprefix $(HB_DIR)/src/,$(sort $(HB_SRCS)))

HB_INCLUDES := $(HB_INCLUDES)
HB_DEFS     := $(HB_DEFS)

## untested
#RAGEL?=ragel
#$(HB_DIR)/%.hh: %.rl
#	$(Q)($(RAGEL) -e -F1 -o "$(@)" "$(<)") || ($(RM) "$(@)"; false)

#### make system
INCLUDES += $(HB_INCLUDES)
DEFS     += $(HB_DEFS) -DHAVE_FREETYPE
SRCS     += $(HB_SRCS)
#DEPS     += $(HB_INCS)

