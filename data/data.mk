include $(TOP_DIR)/tools/functions.mk

BTH ?=python3 $(TOP_DIR)/tools/scripts/bin_to_source.py

DATA_DIR :=$(MKF_DIR)
DATA_FOLDER := data
DATA_HEADER_DIR :=$(DATA_DIR)/include
DATA_SOURCE_DIR :=$(DATA_DIR)/src

DATA_FILE_TYPES = jpg ttf
DATA_HEADERS := 
DATA_SOURCES := 

define DataToHeader
#$$(info <$(1)> <$(2)> <$(3)> <$(4)> <$(5)>)
DATA_HEADERS += $(3)/$(4)
DATA_SOURCES += $(5)/$(6)
$(3)/$(4) $(5)/$(6): $(2)
	@printf "  %-8s $$(subst $$(DATA_DIR)/,,$(2)) > $(4)/$(6)\n" "$$(call up,$(1))"
	@mkdir $$(if $$(Q),,-v) -p $$(dir $(3)/$(4));
	@mkdir $$(if $$(Q),,-v) -p $$(dir $(5)/$(6));
	$$(Q)$$(BTH) -O -H $(3) -h $(4) -c$(5)/$(6) $(2)
endef
define DataToSources
DATA_FILES := $$(filter-out $$(DATA_HEADER_DIR)/% $(DATA_SOURCE_DIR)/%,$$(call rwildcard,$$(DATA_DIR)/,*.$(1)))
$$(foreach DF,$$(DATA_FILES),$$(eval $$(call DataToHeader,$(1),$$(DF),$$(DATA_HEADER_DIR),$$(patsubst $$(DATA_DIR)/%,$(DATA_FOLDER)/%,$$(DF:.$1=_$1.h)),$$(DATA_SOURCE_DIR),$$(patsubst $$(DATA_DIR)/%,$$(DATA_FOLDER)/%,$$(DF:.$1=_$1.c)))))
endef
$(foreach TYPE,$(DATA_FILE_TYPES),$(eval $(call DataToSources,$(TYPE))))

#DATA_HEADERS := $(addprefix $(DATA_DIR), $(DATA_HEADERS))
#DATA_SOURCES := $(addprefix $(DATA_DIR), $(DATA_SOURCES))

# data files
INCLUDE_DIRS = include
SOURCES_DIRS = #src
DEPS += $(DATA_HEADERS) $(DATA_SOURCES)
#INCS += $(DATA_HEADERS)
SRCS += $(DATA_SOURCES)

