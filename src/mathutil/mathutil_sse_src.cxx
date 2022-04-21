/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_sse_src.cxx
 * @author brian
 * @date 2022-04-13
 */

/**
 *
 */
std::ostream &
operator << (std::ostream &out, const PN_vec4f &ff) {
  const float *data = simd_data(ff);
  out << "__m128 [ " << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " ]";
  return out;
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, const PN_vec4i &ff) {
  const int *data = simd_data(ff);
  out << "__m128i [ " << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " ]";
  return out;
}
