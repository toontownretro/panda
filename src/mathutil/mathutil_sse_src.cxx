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

FourFloats FourFloats::_zero(0.0f);
FourFloats FourFloats::_one(1.0f);
FourFloats FourFloats::_negative_one(-1.0f);
FourFloats FourFloats::_two(2.0f);
FourFloats FourFloats::_three(3.0f);
FourFloats FourFloats::_four(4.0f);
FourFloats FourFloats::_point_five(0.5f);
FourFloats FourFloats::_flt_epsilon(FLT_EPSILON);

/**
 *
 */
std::ostream &
operator << (std::ostream &out, const FourFloats &ff) {
  const float *data = ff.get_data();
  out << "[ " << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " ]";
  return out;
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, const FourVector3s &fv) {
  out << "x: " << fv.get_x() << "\n";
  out << "y: " << fv.get_y() << "\n";
  out << "z: " << fv.get_z() << "\n";
  return out;
}
