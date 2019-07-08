####################################
# exported from INSTALL.ANY
# 
# the files in OPT will not be compiled (except if they are needed)
# compiled modules have to be reflected in config/freetype/*.h

# base components (required)

FT2_BASE += src/base/ftsystem.c
FT2_BASE += src/base/ftinit.c
FT2_BASE += src/base/ftdebug.c

FT2_BASE += src/base/ftbase.c

FT2_BASE += src/base/ftbbox.c           # recommended, see <ftbbox.h>
FT2_BASE += src/base/ftglyph.c          # recommended, see <ftglyph.h>

FT2_BASE_OPT += src/base/ftbdf.c        # optional, see <ftbdf.h>
FT2_BASE_OPT += src/base/ftbitmap.c     # optional, see <ftbitmap.h>
FT2_BASE_OPT += src/base/ftcid.c        # optional, see <ftcid.h>
FT2_BASE_OPT += src/base/ftfstype.c     # optional
FT2_BASE_OPT += src/base/ftgasp.c       # optional, see <ftgasp.h>
FT2_BASE_OPT += src/base/ftgxval.c      # optional, see <ftgxval.h>
FT2_BASE_OPT += src/base/ftmm.c         # optional, see <ftmm.h>
FT2_BASE_OPT += src/base/ftotval.c      # optional, see <ftotval.h>
FT2_BASE_OPT += src/base/ftpatent.c     # optional
FT2_BASE_OPT += src/base/ftpfr.c        # optional, see <ftpfr.h>
FT2_BASE_OPT += src/base/ftstroke.c     # optional, see <ftstroke.h>
FT2_BASE_OPT += src/base/ftsynth.c      # optional, see <ftsynth.h>
FT2_BASE_OPT += src/base/fttype1.c      # optional, see <t1tables.h>
FT2_BASE_OPT += src/base/ftwinfnt.c     # optional, see <ftwinfnt.h>

#FT2_BASE_OPT += src/base/ftmac.c        # only on the Macintosh


# font drivers (optional; at least one is needed)

FT2_DRIVERS_OPT += src/bdf/bdf.c           # BDF font driver
FT2_DRIVERS     += src/cff/cff.c           # CFF/OpenType font driver
FT2_DRIVERS_OPT += src/cid/type1cid.c      # Type 1 CID-keyed font driver
FT2_DRIVERS     += src/pcf/pcf.c           # PCF font driver
FT2_DRIVERS_OPT += src/pfr/pfr.c           # PFR/TrueDoc font driver
FT2_DRIVERS     += src/sfnt/sfnt.c         # SFNT files support (TrueType & OpenType)
FT2_DRIVERS     += src/truetype/truetype.c # TrueType font driver
FT2_DRIVERS_OPT += src/type1/type1.c       # Type 1 font driver
FT2_DRIVERS_OPT += src/type42/type42.c     # Type 42 font driver
FT2_DRIVERS_OPT += src/winfonts/winfnt.c   # Windows FONT / FNT font driver


# rasterizers (optional; at least one is needed for vector formats)

FT2_RASTERIZERS += src/raster/raster.c     # monochrome rasterizer
FT2_RASTERIZERS += src/smooth/smooth.c     # anti-aliasing rasterizer


# auxiliary modules (optional)

FT2_AUX     += src/autofit/autofit.c   # auto hinting module
FT2_AUX_OPT += src/cache/ftcache.c     # cache sub-system (in beta)
FT2_AUX_OPT += src/gzip/ftgzip.c       # support for compressed fonts (.gz)
FT2_AUX_OPT += src/lzw/ftlzw.c         # support for compressed fonts (.Z)
FT2_AUX_OPT += src/bzip2/ftbzip2.c     # support for compressed fonts (.bz2)
FT2_AUX_OPT += src/gxvalid/gxvalid.c   # TrueTypeGX/AAT table validation
FT2_AUX_OPT += src/otvalid/otvalid.c   # OpenType table validation
FT2_AUX     += src/psaux/psaux.c       # PostScript Type 1 parsing
FT2_AUX_OPT += src/pshinter/pshinter.c # PS hinting module
FT2_AUX     += src/psnames/psnames.c   # PostScript glyph names support


# Notes:
#
#	`ftcache.c'  needs `ftglyph.c'
#	`ftfstype.c' needs `fttype1.c'
#	`ftglyph.c'  needs `ftbitmap.c'
#	`ftstroke.c' needs `ftglyph.c'
#	`ftsynth.c'  needs `ftbitmap.c'
#
#	`cff.c'      needs `sfnt.c', `pshinter.c', and `psnames.c'
#	`truetype.c' needs `sfnt.c' and `psnames.c'
#	`type1.c'    needs `psaux.c' `pshinter.c', and `psnames.c'
#	`type1cid.c' needs `psaux.c', `pshinter.c', and `psnames.c'
#	`type42.c'   needs `truetype.c'

FT2_INTERNAL_DEPS = $(FT2_BASE_OPT) $(FT2_DRIVERS_OPT) $(FT2_AUX_OPT)

# use only the variables below!

TOP_DIR      ?=.
FT2_FOLDER   ?=$(MKF_DIR)/freetype2
FT2_INCLUDES += -I$(MKF_DIR)/config -I$(FT2_FOLDER)/include
FT2_DEFS     += -DFT2_BUILD_LIBRARY

FT2_SRCS   += $(FT2_BASE) $(FT2_DRIVERS) $(FT2_RASTERIZERS) $(FT2_AUX)

ifneq ($(filter %/ftcache.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/ftglyph.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/ftfstype.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/fttype1.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/ftglyph.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/ftbitmap.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/ftstroke.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/ftglyph.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/ftsynth.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/ftbitmap.c, $(FT2_INTERNAL_DEPS))
endif

ifneq ($(filter %/cff.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/sfnt.c %/pshinter.c %/psnames.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/truetype.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/sfnt.c %/psnames.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/type1.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/psaux.c %/pshinter.c %/psnames.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/type1cid.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/psaux.c %/pshinter.c %/psnames.c, $(FT2_INTERNAL_DEPS))
endif
ifneq ($(filter %/type42.c, $(FT2_SRCS)),)
FT2_SRCS += $(filter %/truetype.c, $(FT2_INTERNAL_DEPS))
endif

FT2_SRCS := $(sort $(addprefix $(FT2_FOLDER)/,$(FT2_SRCS)))

FT2_FOLDER   :=$(FT2_FOLDER)
FT2_INCLUDES := $(FT2_INCLUDES)
FT2_DEFS     := $(FT2_DEFS)

#### make system
INCLUDES += $(FT2_INCLUDES)
DEFS     += $(FT2_DEFS)
SRCS     += $(FT2_SRCS)
#DEPS   += $(FT2_INCS)
