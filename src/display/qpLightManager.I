/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLightManager.I
 * @author brian
 * @date 2022-12-21
 */

/**
 *
 */
INLINE Texture *qpLightManager::
get_dynamic_light_buffer() const {
  CDReader cdata(_cycler);
  return cdata->_dynamic_light_buffer;
}
