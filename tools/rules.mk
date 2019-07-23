#############################################################
#
# Makefile logic and glue and whatnot..
#
# Note: MKF_DIR is defined just before .mk files are included
# Note: TOP_DIR has to be defined in the main Makefile
############################
# File lists (to be documented)
# Subfolders containing .mk files just like this one
#  LIB_SRC_DIRS
# Include directories to be added to INCLUDES
#  INCLUDE_DIRS
# Directories containing source files (see SOURCE_FILE_ENDINGS)
#  SOURCES_DIRS
# List of source files (only .c, .cxx, .cpp and .s supported!)
#  SRCS
# List of files to be $(filter-out IGNORED_SRCS, SRCS) after all source-files were collected
#  IGNORED_SRCS
# List of include files (only .h) - disabled
#  INCS
# Custom build recipe dependencies
#  DEPS
# CC flags
#  CFLAGS
#  INCLUDES
#############################################################

ifndef TOP_DIR
$(error TOP_DIR needs to be set before including rules.mk)
endif

# default target
all:  elf

# find all needed file/etc
include $(TOP_DIR)/tools/collect.mk

############################
# targets

.SUFFIXES: # Delete the default suffixes
.SUFFIXES: .h .c .hpp .hh .cc .cpp .cxx .s .S .o 
.SECONDEXPANSION:
.SECONDARY:
#.ONESHELL:
.PHONY: all elf bin hex srec list clean flash stlink-flash

elf:  $(OELF)
#map:  $(OMAP)
bin:  $(OBIN)
hex:  $(OHEX)
srec: $(OSREC)
list: $(OLIST)

# clean
clean:
	@echo "  CLEAN"
ifneq ($(FILES_TO_CLEAN),)
	@echo "        removing files";
	@rm $(if $(Q),,-v) $(FILES_TO_CLEAN);
endif
ifneq ($(FOLDERS_TO_CLEAN),)
	@echo "        removing folders";
	@rmdir $(if $(Q),,-v) -p $(FOLDERS_TO_CLEAN) 2>/dev/null || :
	@# Note: rmdir errors are ignored as it can not handle overlapping paths (:==true)
endif
	

# extract various binary formats
$(OBIN): $(OELF)
	@echo "  OBJCOPY  $(@)"
	$(Q)$(OBJCOPY) -Obinary $(<) $(@)
$(OHEX): $(OELF)
	@echo "  OBJCOPY  $(@)"
	$(Q)$(OBJCOPY) -Oihex $(<) $(@)
$(SREC): $(OELF)
	@echo "  OBJCOPY  $(@)"
	$(Q)$(OBJCOPY) -Osrec $(<) $(@)
$(LIST): $(OELF)
	@echo "  OBJCOPY  $(@)"
	$(Q)$(OBJCOPY) -S $(<) > $(@)

# build elf binary
$(OELF) $(OMAP): $(OBJS) $(LDSCRIPT) $(LIBDEPS) #$(INCS)
	@echo "  LD       $(@)"
	@mkdir $(if $(Q),,-v) -p $(dir $(OELF));
	@mkdir $(if $(Q),,-v) -p $(dir $(OMAP));
	$(Q)$(LD) $(LDFLAGS_MK) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(OELF)

# build ld script
include $(OPENCM3_DIR)/mk/genlink-rules.mk
#$(LDSCRIPT): $(OPENCM3_DIR)/ld/linker.ld.S $(OPENCM3_DIR)/ld/devices.data
#	@printf "  GENLNK  $(DEVICE)\n"
#	$(Q)$(CPP) $(ARCH_FLAGS) $(shell $(OPENCM3_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) DEFS) -P -E $< > $@

#$(error $(DEPS))

# build c
$(OBJ_DIR_LOCAL)/%.o: %.c $(DEPS) #$(INCS)
	@echo "  CC       $(subst $(TOP_DIR_REAL)/,,$(realpath $(<)))"
	@#echo "         > $(subst $(OBJ_DIR_LOCAL)/,,$(@))"
	@mkdir $(if $(Q),,-v) -p $(dir $(@));
	$(Q)$(CC) $(CFLAGS_MK) $(CFLAGS) $(CPPFLAGS_MK) $(CPPFLAGS) -o $(@) -c $(<)
# build c++
$(OBJ_DIR_LOCAL)/%.o: %.cc $(DEPS) #$(INCS)
	@echo "  CXX      $(subst $(TOP_DIR_REAL)/,,$(realpath $(<)))"
	@#echo "         > $(subst $(OBJ_DIR_LOCAL)/,,$(@))"
	@mkdir $(if $(Q),,-v) -p $(dir $(@));
	$(Q)$(CXX) $(CXXFLAGS_MK) $(CXXFLAGS) $(CPPFLAGS_MK) $(CPPFLAGS) -o $(@) -c $(<)
$(OBJ_DIR_LOCAL)/%.o: %.cxx $(DEPS) #$(INCS)
	@echo "  CXX      $(subst $(TOP_DIR_REAL)/,,$(realpath $(<)))"
	@#echo "         > $(subst $(OBJ_DIR_LOCAL)/,,$(@))"
	@mkdir $(if $(Q),,-v) -p $(dir $(@));
	$(Q)$(CXX) $(CXXFLAGS_MK) $(CXXFLAGS) $(CPPFLAGS_MK) $(CPPFLAGS) -o $(@) -c $(<)
$(OBJ_DIR_LOCAL)/%.o: %.cpp $(DEPS) $(INCS)
	@echo "  CXX      $(subst $(TOP_DIR_REAL)/,,$(realpath $(<)))"
	@#echo "         > $(subst $(OBJ_DIR_LOCAL)/,,$(@))"
	@mkdir $(if $(Q),,-v) -p $(dir $(@));
	$(Q)$(CXX) $(CXXFLAGS_MK) $(CXXFLAGS) $(CPPFLAGS_MK) $(CPPFLAGS) -o $(@) -c $(<)
# build asm
$(OBJ_DIR_LOCAL)/%.o: %.s $(DEPS) #$(INCS)
	@echo "  ASM      $(subst $(TOP_DIR_REAL)/,,$(realpath $(<)))"
	@#echo "         > $(subst $(OBJ_DIR_LOCAL)/,,$(@))"
	@mkdir $(if $(Q),,-v) -p $(dir $(@));
	$(Q)$(ASM) $(ASMFLAGS_MK) $(ASMFLAGS) $(INCLUDES) $(DEFS) -o $(@) -c $(<)

# additional dependencies generated by gcc in the last run
-include $(ODEPS)
# or
#depend: .depend
#.depend: $(INCS) $(DEPS) | $(SRCS)
#	rm -f ./.depend
#	$(CC) $(CFLAGS) -MM $| -MF ./.depend
#include .depend

# flash
ifeq ($(BMP_PORT),)
ifeq ($(OOCD_FILE),)
flash: $(OELF)
	@echo "  FLASH    $<"
	$(Q)(echo "halt; program $(realpath $(OELF)) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f target/$(OOCD_TARGET).cfg \
		-c "program $(OELF) verify reset exit" \
		$(NULL)
else
flash: $(OELF)
	@echo "  FLASH    $<"
	$(Q)(echo "halt; program $(realpath $(OELF)) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(OELF) verify reset exit" \
		$(NULL)
endif
else
flash: $(OELF)
	@echo "  GDB      $(OELF) (flash)"
	$(Q)$(GDB) --batch \
		   -ex 'target extended-remote $(BMP_PORT)' \
		   -x $(EXAMPLES_SCRIPT_DIR)/black_magic_probe_flash.scr \
		   $(OELF)
endif

stlink-flash: $(OBIN)
	@echo "  FLASH    $<"
	$(Q)$(STFLASH) write $(OBIN) 0x8000000
