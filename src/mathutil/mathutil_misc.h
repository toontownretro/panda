/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_misc.h
 * @author lachbr
 * @date 2020-10-19
 *
 * Miscellaneous math utility functions.
 */

#ifndef MATHUTIL_MISC_H
#define MATHUTIL_MISC_H

#include "config_mathutil.h"
#include "luse.h"

#define NORMAL_EPSILON 0.00001
#define ON_EPSILON 0.1 // we should ensure that (float)BOGUS_RANGE < (float)(BOGUA_RANGE + 0.2 * ON_EPSILON)
#define EQUAL_EPSILON 0.001
#define ON_LIGHTMAP_EPSILON ON_EPSILON / 16.0

INLINE void swap_floats(float &a, float &b);

template <class T>
INLINE T clamp(T const &val, T const &minVal, T const &maxVal);

INLINE float remap_val_clamped(float val, float A, float B, float C, float D);

// convert texture to linear 0..1 value
INLINE float tex_light_to_linear(int c, int exponent);

// maps a float to a byte fraction between min & max
INLINE unsigned char fixed_8_fraction(float t, float tMin, float tMax);

INLINE PN_stdfloat inv_r_squared(const LVector3 &v);

extern EXPCL_PANDA_MATHUTIL bool
solve_inverse_quadratic(float x1, float y1, float x2, float y2,
                        float x3, float y3, float &a, float &b, float &c);

extern EXPCL_PANDA_MATHUTIL bool
solve_inverse_quadratic_monotonic(float x1, float y1, float x2, float y2,
                                  float x3, float y3, float &a, float &b, float &c);

// Returns A + (B-A)*flPercent.
// float Lerp( float flPercent, float A, float B );
template <class T>
INLINE T tlerp(float percent, T const &A, T const &B);

INLINE float flerp(float f1, float f2, float t);
INLINE float flerp(float f1, float f2, float i1, float i2, float x);

INLINE int ceil_pow_2(int in);
INLINE int floor_pow_2(int in);

#include "mathutil_misc.I"
#include "mathutil_misc.T"

#endif // MATHUTIL_MISC_H
