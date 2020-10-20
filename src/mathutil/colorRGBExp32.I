/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorRGBExp32.I
 * @author lachbr
 * @date 2020-10-19
 */

/**
 *
 */
INLINE ColorRGBExp32::
ColorRGBExp32() {
  _r = 0;
  _g = 0;
  _b = 0;
  _exponent = 0;
}

/**
 *
 */
INLINE ColorRGBExp32::
ColorRGBExp32(unsigned char r, unsigned char g, unsigned char b,
              signed char exponent) {
  _r = r;
  _g = g;
  _b = b;
  _exponent = exponent;
}

/**
 * Returns the red component.
 */
INLINE unsigned char ColorRGBExp32::
get_r() const {
  return _r;
}

/**
 * Returns the green component.
 */
INLINE unsigned char ColorRGBExp32::
get_g() const {
  return _g;
}

/**
 * Returns the blue component.
 */
INLINE unsigned char ColorRGBExp32::
get_b() const {
  return _b;
}

/**
 * Returns the exponent.
 */
INLINE signed char ColorRGBExp32::
get_exponent() const {
  return _exponent;
}
