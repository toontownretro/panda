/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_material.cxx
 * @author lachbr
 * @date 2021-03-16
 */

#include "config_material.h"
#include "materialBase.h"
#include "materialParamBool.h"
#include "materialParamColor.h"
#include "materialParamFloat.h"
#include "materialParamTexture.h"
#include "materialParamVector.h"

ConfigureDef(config_material);
ConfigureFn(config_material) {
  init_libmaterial();
}

NotifyCategoryDef(material, "");

/**
 *
 */
void
init_libmaterial() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  MaterialBase::init_type();

  MaterialParamBool::init_type();
  MaterialParamColor::init_type();
  MaterialParamFloat::init_type();
  MaterialParamTexture::init_type();
  MaterialParamVector::init_type();

  MaterialParamBool::register_with_read_factory();
  MaterialParamColor::register_with_read_factory();
  MaterialParamFloat::register_with_read_factory();
  MaterialParamTexture::register_with_read_factory();
  MaterialParamVector::register_with_read_factory();
}
