// Config for harfbuzz (and probably others..)

//#ifdef __cplusplus
#include <climits> // fix for arm-none-eabi-g++ (include <limits.h> does not work)
//#endif

//#ifndef NDEBUG
//#define NDEBUG
//#endif
#ifndef __OPTIMIZE_SIZE__
#define __OPTIMIZE_SIZE__
#endif

#define HB_NO_MT
//#define HB_DISABLE_DEPRECATED
#define HB_NO_ATEXIT
//#define HB_NO_BUFFER_MESSAGE
#define HB_NO_BUFFER_SERIALIZE
#define HB_NO_BITMAP
#define HB_NO_CFF
#define HB_NO_COLOR
#define HB_NO_FACE_COLLECT_UNICODES
#define HB_NO_GETENV
#define HB_NO_HINTING
#define HB_NO_LANGUAGE_PRIVATE_SUBTAG
#define HB_NO_LAYOUT_FEATURE_PARAMS
#define HB_NO_LAYOUT_UNUSED
//#define HB_NO_MATH
#define HB_NO_MMAP
#define HB_NO_NAME
#define HB_NO_OPEN
#define HB_NO_SETLOCALE

#define HB_NO_OT_FONT
#define HB_NO_OT_FONT_GLYPH_NAMES
#define HB_NO_OT_SHAPE_FRACTIONS

#define HB_NO_STAT
#define HB_NO_SUBSET_LAYOUT
#define HB_NO_VAR

#define HB_NO_AAT
#define HB_NO_LEGACY
