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
#include "clockObject.h"

#include <mmintrin.h>

//

/**
 *
 */
int
main(int argc, char *argv[]) {
  ClockObject *clock = ClockObject::get_global_clock();

  constexpr int num_vecs = 1000000;
  constexpr int num_groups = num_vecs / SIMD_NATIVE_WIDTH;
  SIMDNativeVector3 *vecs = new SIMDNativeVector3[num_groups];
  for (int i = 0; i < num_groups; ++i) {
    vecs[i].load(LVector3::up());
  }
  SIMDNativeVector3 *vec2 = new SIMDNativeVector3[num_groups];
  for (int i = 0; i < num_groups; ++i) {
    vec2[i].load(LVector3::down());
  }
  SIMD_NATIVE_ALIGN float dots[num_vecs];

  double vstart = clock->get_real_time();

  SIMDNativeFloat *vdots = reinterpret_cast<SIMDNativeFloat *>(dots);
  for (int i = 0; i < num_groups; ++i) {
    (*vdots) = vecs[i].dot(vec2[i]);
    vdots++;
  }
  double vend = clock->get_real_time();

  std::cerr << vend - vstart << "\n";

  std::cerr << dots[5] << "\n";

  //vstart = clock->get_real_time();
  //for (int i = 0; i < num_vecs; ++i) {
  //  dots[i] = vecs[i].dot(vec2[i]);
 // }
  //vend = clock->get_real_time();

  //std::cerr << vend - vstart << "\n";

  return 0;
}
