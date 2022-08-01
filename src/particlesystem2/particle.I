/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particle.I
 * @author brian
 * @date 2022-04-02
 */

/**
 *
 */
INLINE unsigned char ParticleFieldDef::
get_component_width() const {
  switch (_numeric_type) {
  case PNT_int8:
  case PNT_uint8:
    return 1;
  case PNT_int16:
  case PNT_uint16:
    return 2;
  case PNT_int32:
  case PNT_uint32:
  case PNT_float32:
    return 4;
  case PNT_int64:
  case PNT_uint64:
  case PNT_float64:
    return 8;
  }
}
