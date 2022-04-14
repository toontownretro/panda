/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_simd.cxx
 * @author brian
 * @date 2022-04-14
 */

#include "pandabase.h"
#include "mathutil_simd.h"

#include <mmintrin.h>

#pragma clang optimize off

/**
 *
 */
int
main(int argc, char *argv[]) {
  FourVector3s v1(LVector3::up());
  FourVector3s v2(LVector3::right());
  FourVector3s c = v1.cross(v2);

  std::cout << c << "\n";

  return 0;
}
