/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_material.cxx
 * @author lachbr
 * @date 2021-03-21
 */

// Test program for new material system.

#include "standardMaterial.h"
#include "materialRegistry.h"

int
main(int argc, char *argv[]) {

  MaterialRegistry *reg = MaterialRegistry::get_global_ptr();
  PT(StandardMaterial) mat = DCAST(StandardMaterial,
    reg->create_material(StandardMaterial::get_class_type()));

  nassertr(mat != nullptr, 1);
  nassertr(mat->is_exact_type(StandardMaterial::get_class_type()), 1);

  mat->set_base_color(LColor(0.75, 0.2, 0.8, 1.0));
  nassertr(mat->get_base_color() == LColor(0.75, 0.2, 0.8, 1.0), 1);
  nassertr(mat->get_base_texture() == nullptr, 1);

  mat->set_rim_light(true);
  nassertr(mat->get_rim_light() == true, 1);

  nassertr(mat->get_param("$basecolor") != nullptr, 1);

  mat->write_pmat("test_material.pmat");

  bool ret = mat->write_mto("test_material.mto");
  nassertr(ret, 1);

  return 0;
}
