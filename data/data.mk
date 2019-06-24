include $(TOP_DIR)/tools/functions.mk

BTH ?=python3 $(TOP_DIR)/tools/scripts/bin_to_array.py

DATA_DIR :=$(MKF_DIR)
DATA_HEADER_DIR :=$(DATA_DIR)/include/data

DATA_FILE_TYPES = jpg ttf
DATA_HEADERS := 
define DataToHeader
DATA_HEADERS += $(3)
#$$(info <$(1)> <$(2)> <$(3)>)
$(3): $(2)
	@printf "  %-8s $$(subst $$(DATA_DIR)/,,$(2)) > $$(subst $$(DATA_HEADER_DIR)/,,$(3))\n" "$$(call up,$(1))"
	@mkdir $$(if $$(Q),,-v) -p $$(dir $(3));
	$$(Q)$$(BTH) -o $(3) $(2)
endef
define DataToHeaders
DATA_FILES := $$(call rwildcard,$$(DATA_DIR)/,*.$(1))
$$(foreach DF,$$(DATA_FILES),$$(eval $$(call DataToHeader,$(1),$$(DF),$$(patsubst $$(DATA_DIR)/%,$$(DATA_HEADER_DIR)/%,$$(DF:.$1=_$1.h)))))
endef
$(foreach TYPE,$(DATA_FILE_TYPES),$(eval $(call DataToHeaders,$(TYPE))))

# data files
INCLUDE_DIRS = include
SOURCES_DIRS =
DEPS += $(DATA_HEADERS)
#INCS += $(DATA_HEADERS)

