/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorRGBExp32.cxx
 * @author brian
 * @date 2020-10-19
 */

#include "colorRGBExp32.h"
#include "mathutil_misc.h"
#include "cmath.h"

INLINE static int
calc_exponent(float in) {
  int power = 0;
  if (in != 0.0f) {
    while (in > 255.0f) {
      ++power;
      in *= 0.5f;
    }
    while (in < 128.0f) {
      --power;
      in *= 2.0f;
    }
  }
  return power;
}

ColorRGBExp32::
ColorRGBExp32(const LVecBase3 &vin) {
  nassertv(vin[0] >= 0.0f && vin[1] >= 0.0f && vin[1] >= 0.0f);

  LVecBase3 v = vin;

  float max = v[0];
  for (int i = 1; i < 3; ++i) {
    max = std::max(max, v[i]);
  }

  int exponent = calc_exponent(max);
  exponent = std::clamp(exponent, -128, 127);

  float scalar = cpow(2.0f, (float)-exponent);
  for (int i = 0; i < 3; ++i) {
    v[i] *= scalar;
    v[i] = std::min(v[i], 255.0f);
  }

  _r = (unsigned char)v[0];
  _g = (unsigned char)v[1];
  _b = (unsigned char)v[2];
  _exponent = (signed char)exponent;
}

#if 0
// given a floating point number  f, return an exponent e such that
// for f' = f * 2^e,  f is on [128..255].
// Uses IEEE 754 representation to directly extract this information
// from the float.
INLINE static int
calc_exponent(const float *pin) {
  // The thing we will take advantage of here is that the exponent component
  // is stored in the float itself, and because we want to map to 128..255, we
  // want an "ideal" exponent of 2^7. So, we compute the difference between the
  // input exponent and 7 to work out the normalizing exponent. Thus if you
  // pass in 32 (represented in IEEE 754 as 2^5), this function will return 2
  // (because 32 * 2^2 = 128)
  if (*pin == 0.0f) {
    return 0;
  }

  unsigned int fbits = *reinterpret_cast<const unsigned int *>(pin);

  // the exponent component is bits 23..30, and biased by +127
  const unsigned int biasedSeven = 7 + 127;

  signed int expComponent = (fbits & 0x7F800000) >> 23;
  expComponent -= biasedSeven; // now the difference from seven (positive if was less than, etc)
  return expComponent;
}

ColorRGBExp32::
ColorRGBExp32(const LVecBase3 &vin) {
  nassertv(vin[0] >= 0.0f && vin[1] >= 0.0f && vin[1] >= 0.0f);

  // work out which of the channels is the largest ( we will use that to map the exponent )
  // this is a sluggish branch-based decision tree -- most architectures will offer a [max]
  // assembly opcode to do this faster.
  float pMax;
  if (vin[0] > vin[1]) {
    if (vin[0] > vin[2]) {
      pMax = vin[0];
    } else {
      pMax = vin[2];
    }
  } else {
    if (vin[1] > vin[2]) {
      pMax = vin[1];
    } else {
      pMax = vin[2];
    }
  }

  // now work out the exponent for this luxel.
  signed int exponent = calc_exponent(&pMax);

  // make sure the exponent fits into a signed byte.
  // (in single precision format this is assured because it was a signed byte to begin with)
  nassertv(exponent > -128 && exponent <= 127);

  // promote the exponent back onto a scalar that we'll use to normalize all the numbers
  float scalar;
  {
    unsigned int fbits = (127 - exponent) << 23;
    scalar = *reinterpret_cast<float *>(&fbits);
  }

  std::cout << "scalar: " << scalar << "\n";
  std::cout << "vin[0] * scalar: " << vin[0] * scalar << "\n";
  std::cout << "vin[1] * scalar: " << vin[1] * scalar << "\n";
  std::cout << "vin[2] * scalar: " << vin[2] * scalar << "\n";

  // we should never need to clamp:
  nassertv(vin[0] * scalar <= 255.0f &&
           vin[1] * scalar <= 255.0f &&
           vin[2] * scalar <= 255.0f);

  // This awful construction is necessary to prevent VC2005 from using the
  // fldcw/fnstcw control words around every float-to-unsigned-char operation.
  {
    int red = (vin[0] * scalar);
    int green = (vin[1] * scalar);
    int blue = (vin[2] * scalar);

    _r = red;
    _g = green;
    _b = blue;
  }

  _exponent = (signed char)exponent;
}
#endif

LVecBase3 ColorRGBExp32::
as_linear_color() const {
  // FIXME: Why is there a factor of 255 built into this?
  return LVecBase3(
    255.0f * tex_light_to_linear(_r, _exponent),
    255.0f * tex_light_to_linear(_g, _exponent),
    255.0f * tex_light_to_linear(_b, _exponent)
  );
}

/**
 * Fills in the structure from the given datagram.
 */
void ColorRGBExp32::
read_datagram(DatagramIterator &dgi) {
  _r = dgi.get_uint8();
  _g = dgi.get_uint8();
  _b = dgi.get_uint8();
  _exponent = dgi.get_int8();
}

/**
 * Writes the structure to the indicated datagram.
 */
void ColorRGBExp32::
write_datagram(Datagram &dg) const {
  dg.add_uint8(_r);
  dg.add_uint8(_g);
  dg.add_uint8(_b);
  dg.add_int8(_exponent);
}
