#############################################################
#
# Collect files and create lists
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
# List of source files (only .c and .s supported!)
#  SRCS
# List of files to be $(filter-out IGNORED_SRCS, SRCS) after all source-files were collected
#  IGNORED_SRCS
# List of include files (only .h supported!)
#  INCS
# Custom build recipe dependencies
#  DEPS
# CC flags
#  CFLAGS
#  INCLUDES
#############################################################
# Output
#  INCS SRCS OBJS ODEPS
#  OBJ_DIR_LOCAL
#  ...

ifndef TOP_DIR
$(error TOP_DIR needs to be set before including collect.mk)
endif

include $(TOP_DIR)/tools/defaults.mk
include $(TOP_DIR)/tools/functions.mk

TOP_DIR_REAL:=$(realpath $(TOP_DIR))
CURDIR_FROM_TOP=$(patsubst $(TOP_DIR_REAL)/%,%,$(CURDIR))

# OBJ_DIR ???LOCAL???
OBJ_DIR_LOCAL=$(OBJ_DIR)/$(CURDIR_FROM_TOP)

# Add possibility to include everything relative to TOP_DIR
#INCLUDES += -I$(TOP_DIR)

# libopencm3 build targets and sources
INCLUDES +=-I$(OPENCM3_DIR)/include
include $(TOP_DIR)/tools/opencm3.mk

# Initialize ignored sources
ifndef IGNORED_SRCS
IGNORED_SRCS:=
endif

# Collect .mk files
MK_FILES_COLLECTED:=$(foreach LSD,$(LIB_SRC_DIRS),$(wildcard $(TOP_DIR)/$(LSD)/*.mk))


############################
# Add paths for main Makefile
CleanPath_=$(if $2,$(subst $1,,$2)/$3,)
CleanPath=$(call CleanPath_,$1,$(realpath $(dir $2)),$(notdir $2))
INCLUDE_DIRS_COLLECTED:=$(foreach DIR,$(INCLUDE_DIRS),$(call CleanPath,$(TOP_DIR_REAL),$(DIR)))
SOURCES_DIRS_COLLECTED:=$(foreach DIR,$(SOURCES_DIRS),$(call CleanPath,$(TOP_DIR_REAL),$(DIR)))
IGNORED_INCLUDE_DIRS_COLLECTED:=$(foreach DIR,$(IGNORED_INCLUDE_DIRS),$(call CleanPath,$(TOP_DIR_REAL),$(DIR)))
IGNORED_SOURCES_DIRS_COLLECTED:=$(foreach DIR,$(IGNORED_SOURCES_DIRS),$(call CleanPath,$(TOP_DIR_REAL),$(DIR)))
# Include mk files from LIB_SRC_DIRS
define BuildDirs
INCLUDE_DIRS:=
SOURCES_DIRS:=
IGNORED_INCLUDE_DIRS:=
IGNORED_SOURCES_DIRS:=
MKF_DIR:=$$(patsubst %/,%,$$(dir $(1)))
include $(1)
INCLUDE_DIRS_COLLECTED+=$$(foreach DIR,$$(addprefix $$(MKF_DIR)/,$$(INCLUDE_DIRS)),$$(call CleanPath,$$(TOP_DIR_REAL),$$(DIR)))
SOURCES_DIRS_COLLECTED+=$$(foreach DIR,$$(addprefix $$(MKF_DIR)/,$$(SOURCES_DIRS)),$$(call CleanPath,$$(TOP_DIR_REAL),$$(DIR)))
IGNORED_INCLUDE_DIRS_COLLECTED+=$$(foreach DIR,$$(addprefix $$(MKF_DIR)/,$$(IGNORED_INCLUDE_DIRS)),$$(call CleanPath,$$(TOP_DIR_REAL),$$(DIR)))
IGNORED_SOURCES_DIRS_COLLECTED+=$$(foreach DIR,$$(addprefix $$(MKF_DIR)/,$$(IGNORED_SOURCES_DIRS)),$$(call CleanPath,$$(TOP_DIR_REAL),$$(DIR)))
endef
$(foreach MKF,$(MK_FILES_COLLECTED),$(eval $(call BuildDirs,$(MKF))))
# Cleanup dirs
IGNORED_INCLUDE_DIRS_COLLECTED:=$(sort $(IGNORED_INCLUDE_DIRS_COLLECTED))
IGNORED_SOURCES_DIRS_COLLECTED:=$(sort $(IGNORED_SOURCES_DIRS_COLLECTED))
#INCLUDE_DIRS_COLLECTED:=$(filter-out $(addsuffix %,$(IGNORED_INCLUDE_DIRS_COLLECTED)),$(sort $(INCLUDE_DIRS_COLLECTED)))
#SOURCES_DIRS_COLLECTED:=$(filter-out $(addsuffix %,$(IGNORED_SOURCES_DIRS_COLLECTED)),$(sort $(SOURCES_DIRS_COLLECTED)))
INCLUDE_DIRS_COLLECTED:=$(filter-out $(IGNORED_INCLUDE_DIRS_COLLECTED),$(sort $(INCLUDE_DIRS_COLLECTED)))
SOURCES_DIRS_COLLECTED:=$(filter-out $(IGNORED_SOURCES_DIRS_COLLECTED),$(sort $(SOURCES_DIRS_COLLECTED)))
#$(info --IGNORED INC DIRS--)
#$(foreach INC,$(IGNORED_INCLUDE_DIRS_COLLECTED),$(info $(INC)))
#$(info --IGNORED SRC DIRS--)
#$(foreach SRC,$(IGNORED_SOURCES_DIRS_COLLECTED),$(info $(SRC)))
#$(info --INC DIRS--)
#$(foreach INC,$(INCLUDE_DIRS_COLLECTED),$(info $(INC)))
#$(info --SRC DIRS--)
#$(foreach SRC,$(SOURCES_DIRS_COLLECTED),$(info $(SRC)))
#$(error okay)

############################
# Find header files (INCS) recursively
INCS+=$(foreach DIR,$(INCLUDE_DIRS_COLLECTED),$(call rwildcard,$(TOP_DIR)$(DIR)/,*.h))
# Find source files (SRCS) for different file endings
SOURCE_FILE_ENDINGS := c cxx cpp s
SRCS+=$(foreach EXT,$(SOURCE_FILE_ENDINGS),$(foreach DIR,$(SOURCES_DIRS_COLLECTED),$(wildcard $(TOP_DIR)$(DIR)/*.$(EXT))))
# Cleanup headers/sources
#INCS:=$(sort $(foreach INC,$(filter %.h, $(INCS)),$(patsubst $(TOP_DIR_REAL)%,$(TOP_DIR)%,$(realpath $(INC)))))
#SRCS:=$(sort $(foreach SRC,$(filter %.c %.s, $(SRCS)),$(patsubst $(TOP_DIR_REAL)%,$(TOP_DIR)%,$(realpath $(SRC)))))
define CLEANUP_LIST
$1:=$$(realpath $$(filter $$(addprefix %,$$(SOURCE_FILE_ENDINGS)), $$($1)))
$1:=$$(patsubst $$(CURDIR)/%,%,$$($1))
$1:=$$(patsubst $$(TOP_DIR_REAL)%,$$(TOP_DIR)%,$$($1))
$1:=$$(sort $$($1))
#SRCS:=$(realpath $(filter $(addprefix %,$(SOURCE_FILE_ENDINGS)), $(SRCS)))
#SRCS:=$(patsubst $(CURDIR)/%,%,$(SRCS))
#SRCS:=$(patsubst $(TOP_DIR_REAL)%,$(TOP_DIR)%,$(SRCS))
#SRCS:=$(sort $(SRCS))
endef
$(eval $(call CLEANUP_LIST,INCS))
$(eval $(call CLEANUP_LIST,SRCS))

# filter
SRCS:=$(filter-out $(IGNORED_SRCS),$(SRCS))
#$(info filtered out '$(IGNORED_SRCS)')
#$(foreach INC,$(INCS),$(info $(INC)))
#$(foreach SRC,$(SRCS),$(info $(SRC)))
#$(error lol)
############################


############################
# includes
INCLUDES += $(addprefix -I$(TOP_DIR),$(INCLUDE_DIRS_COLLECTED))
# export includes to eclipse
#$(foreach INCLUDE,$(sort $(INCLUDES:-I$(TOP_DIR)%=%)),$(info <includepath workspace_path="true">/W600-sdk$(INCLUDE)</includepath>))
#$(error okay)

############################
# objects (maybe use SOURCE_FILE_ENDINGS?)
OBJS := $(SRCS)
OBJS := $(OBJS:%.c=%.o)
OBJS := $(OBJS:%.cxx=%.o)
OBJS := $(OBJS:%.cpp=%.o)
OBJS := $(OBJS:%.s=%.o)
OBJS := $(OBJS:%.o=$(OBJ_DIR_LOCAL)/%.o)
#OBJS := $(patsubst ./%,%,$(subst $(CURDIR_FROM_TOP)/../,,$(OBJS))) << this break obj-to-src reference
#OBJS := $(filter %.o, $(OBJS)) # TBD add check here
ODEPS := $(OBJS:%.o=%.d)
#$(error $(OBJS))

###########################
# clean
#FILES_TO_CLEAN   := $(OELF) $(OMAP) $(OBIN) $(OHEX) $(OSREC) $(OLIST) $(OBJS) $(ODEPS) $(LDSCRIPT)
FILES_TO_CLEAN   := $(patsubst $(CURDIR)/%,%,$(realpath $(OELF) $(OMAP) $(OBIN) $(OHEX) $(OSREC) $(OLIST) $(OBJS) $(ODEPS) $(LDSCRIPT)))
$(foreach V,$(sort $(dir $(FILES_TO_CLEAN))),$(eval FOLDERS_TO_CLEAN:=$(V) $(FOLDERS_TO_CLEAN)))
#FILES_TO_CLEAN   := $(wildcard $(FILES_TO_CLEAN)) << this is done by realpath
#$(info before: $(FOLDERS_TO_CLEAN))
FOLDERS_TO_CLEAN := $(wildcard $(FOLDERS_TO_CLEAN))
#$(info after: $(FOLDERS_TO_CLEAN))

############################
# Combine C_FLAGS
CPPFLAGS_MK += $(INCLUDES)
CPPFLAGS_MK += $(CMACRO)

# Add Makefiles as build dependencies
MK_DEPS += Makefile $(TOP_DIR)/tools/rules.mk $(COLLECTED_MK_FILES)

