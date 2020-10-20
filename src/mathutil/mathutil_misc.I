/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_misc.I
 * @author lachbr
 * @date 2020-10-19
 *
 * Miscellaneous math utility functions.
 */

INLINE void
swap_floats(float &a, float &b) {
  float temp = b;
  b = a;
  a = temp;
}

INLINE float
remap_val_clamped(float val, float A, float B, float C, float D) {
  if (A == B) {
    return val >= B ? D : C;
  }

  float cVal = (val - A) / (B - A);
  cVal = clamp(cVal, 0.0f, 1.0f);

  return C + (D - C) * cVal;
}

// convert texture to linear 0..1 value
INLINE float
tex_light_to_linear(int c, int exponent) {
  nassertr(exponent >= -128 && exponent <= 127, 0.0);

  return (float)c * std::pow(2.0, exponent) / 255.0;
}

// maps a float to a byte fraction between min & max
INLINE unsigned char
fixed_8_fraction(float t, float tMin, float tMax) {
  if (tMax <= tMin) {
    return 0;
  }

  float frac = remap_val_clamped(t, tMin, tMax, 0.0f, 255.0f);
  return (unsigned char)(frac + 0.5f);
}

INLINE PN_stdfloat
inv_r_squared(const LVector3 &v) {
  return 1.f / std::max(1.f, v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

INLINE float
flerp(float f1, float f2, float t) {
  return f1 + (f2 - f1) * t;
}

INLINE float
flerp(float f1, float f2, float i1, float i2, float x) {
  return f1 + (f2 - f1) * (x - i1) / (i2 - i1);
}

INLINE int
ceil_pow_2(int in) {
	int retval;

	retval = 1;
	while( retval < in ) {
		retval <<= 1;
  }

	return retval;
}

INLINE int
floor_pow_2(int in) {
	int retval;

	retval = 1;
	while( retval < in ) {
    retval <<= 1;
  }

	return retval >> 1;
}
