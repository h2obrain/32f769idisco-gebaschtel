ifndef TOP_DIR
$(error TOP_DIR needs to be set before including opencm3.mk)
endif

FTGL_DIR ?=$(MKF_DIR)/freetype-gl/harfbuzz
FTGL_DIR :=$(FTGL_DIR)

FTGL_INCLUDES += -I$(FTGL_DIR) 

FTGL_SRCS += platform.c
FTGL_SRCS += freetype-gl-err.c
FTGL_SRCS += texture-atlas.c
FTGL_SRCS += texture-font.c

FTGL_INCS += platform.h
FTGL_INCS += freetype-gl-err.h freetype-gl-errdef.h
FTGL_INCS += texture-atlas.h
FTGL_INCS += texture-font.h

FTGL_INCLUDES := $(FTGL_INCLUDES)
FTGL_DEFS     := $(FTGL_DEFS)
FTGL_SRCS     := $(addprefix $(FTGL_DIR)/,$(FTGL_SRCS))
FTGL_INCS     := $(addprefix $(FTGL_DIR)/,$(FTGL_INCS))

#### make system
INCLUDES += $(FTGL_INCLUDES)
DEFS     += $(FTGL_DEFS)
SRCS     += $(FTGL_SRCS)
INCS     += $(FTGL_INCS)
#DEPS     += $(FTGL_INCS)

