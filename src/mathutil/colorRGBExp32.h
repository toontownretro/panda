/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorRGBExp32.h
 * @author brian
 * @date 2020-10-19
 */

#ifndef COLORRGBEXP32_H
#define COLORRGBEXP32_H

#include "config_mathutil.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "luse.h"

/**
 * Floating point RGB color compressed into 4 bytes.  Implementation taken from
 * Valve's Source Engine.
 */
class EXPCL_PANDA_MATHUTIL ColorRGBExp32 {
PUBLISHED:
  INLINE ColorRGBExp32();
  INLINE ColorRGBExp32(unsigned char r, unsigned char g, unsigned char b,
                       signed char exponent);
  ColorRGBExp32(const LVecBase3 &rgb);

  INLINE unsigned char get_r() const;
  MAKE_PROPERTY(r, get_r);

  INLINE unsigned char get_g() const;
  MAKE_PROPERTY(g, get_g);

  INLINE unsigned char get_b() const;
  MAKE_PROPERTY(b, get_b);

  INLINE signed char get_exponent() const;
  MAKE_PROPERTY(exponent, get_exponent);

  LVecBase3 as_linear_color() const;

  void read_datagram(DatagramIterator &dgi);
  void write_datagram(Datagram &dg) const;

private:
  unsigned char _r;
  unsigned char _g;
  unsigned char _b;
  signed char _exponent;
};

struct CompressedLightCube {
  ColorRGBExp32 color[6];

  INLINE void read_datagram(DatagramIterator &dgi);
  INLINE void write_datagram(Datagram &dg) const;
};

#include "colorRGBExp32.I"

#endif // COLORRGBEXP32_H
