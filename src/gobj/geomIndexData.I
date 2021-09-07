/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexData.I
 * @author brian
 * @date 2021-09-06
 */

/**
 * Returns the numeric type of the index buffer.
 */
INLINE GeomEnums::NumericType GeomIndexData::
get_index_type() const {
  return _index_type;
}

/**
 * Returns the byte length of a single entry within the index buffer.
 * This is determined by the index type.
 */
INLINE int GeomIndexData::
get_index_stride() const {
  switch(_index_type) {
  case GeomEnums::NT_uint8:
    return 1;
  case GeomEnums::NT_uint16:
    return 2;
  case GeomEnums::NT_uint32:
    return 4;
  default:
    // None of the index types other than the ones handled above are supported.
    assert(false);
    return 0;
  }
}
