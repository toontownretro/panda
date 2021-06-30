/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEvent.cxx
 * @author brian
 * @date 2021-06-14
 */

#include "animEvent.h"
#include "config_anim.h"

AnimEvent *AnimEvent::_ptr = nullptr;

/**
 *
 */
AnimEvent *AnimEvent::
ptr() {
  if (_ptr == nullptr) {
    _ptr = new AnimEvent;
    _ptr->load_values();
  }

  return _ptr;
}

/**
 *
 */
const ConfigVariableList &AnimEvent::
get_config_var() const {
  return anim_events;
}
