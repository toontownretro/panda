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
#define HAVE_SLEEF 1
#include "mathutil_simd.h"
#include "clockObject.h"

/**
 *
 */
int
main(int argc, char *argv[]) {
  LQuaternionf rot;
  rot.set_hpr(LVecBase3(90, 0, 0));
  LQuaternionf rot2;
  rot2.set_hpr(LVecBase3(20, 0, 0));

  SIMDQuaternionf q = rot;
  q.output(std::cerr);
  SIMDQuaternionf q2 = rot2;
  q2.output(std::cerr);

  SIMDQuaternionf ql = (-q).align(q2).lerp(q2, 0.1f);
  ql.output(std::cerr);
  std::cerr << ql.get_lquat(0).get_hpr() << "\n";
  ql = q.slerp(q2, 0.1f);
  ql.output(std::cerr);
  std::cerr << ql.get_lquat(0).get_hpr() << "\n";

  //SIMDVector3f v = LVector3::up();
  //v.output(std::cerr);

  return 0;
}
