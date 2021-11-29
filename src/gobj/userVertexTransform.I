/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userVertexTransform.I
 * @author drose
 * @date 2005-03-24
 */

/**
 * Returns the name passed to the constructor.  Completely arbitrary.
 */
INLINE const std::string &UserVertexTransform::
get_name() const {
  return _name;
}
