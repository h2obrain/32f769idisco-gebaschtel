/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2016 Oliver Meier <h2obrain@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VECTOR_GFX_H_
#define VECTOR_GFX_H_

#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>

#if 0
#define vector_flt_t         float
#define vector_flt_MIN       FLT_MIN
#define vector_flt_MAX       FLT_MAX
#define vector_flt_EPSILON   FLT_EPSILON
#define vector_flt_MIN_VALUE 0.00000001
#define vector_flt_abs       fabsf
#define vector_flt_sqrt      sqrtf
#define vector_flt_pow       powf
#define vector_flt_sin       sinf
#define vector_flt_cos       cosf
#define vector_flt_tan       tanf
#define vector_flt_atan2     atan2f
#define vector_flt_min       fminf
#define vector_flt_max       fmaxf
#define vector_flt_round     roundf
#define vector_flt_round_int nearbyintf
#define vector_flt_floor     floorf
#define vector_flt_ceil      ceilf
#define vector_flt_mod       modf
#else
#define vector_flt_t         double
#define vector_flt_MIN       DBL_MIN
#define vector_flt_MAX       DBL_MAX
#define vector_flt_EPSILON   DBL_EPSILON
#define vector_flt_MIN_VALUE 0.0000000000001 /* TBD */
#define vector_flt_abs       fabs
#define vector_flt_sqrt      sqrt
#define vector_flt_pow       pow
#define vector_flt_sin       sin
#define vector_flt_cos       cos
#define vector_flt_tan       tan
#define vector_flt_atan2     atan2
#define vector_flt_min       fmin
#define vector_flt_max       fmax
#define vector_flt_round     round
#define vector_flt_round_int nearbyint
#define vector_flt_floor     floor
#define vector_flt_ceil      ceil
#define vector_flt_mod       modf
#endif

#define vector_flt_swap(a,b) { vector_flt_t t = a; a = b; b = t; }

#include "vector.h"
#include "matrix.h"
#include "bezier.h"
#include "drawing.h"

#endif /* VECTOR_GFX_H_ */
