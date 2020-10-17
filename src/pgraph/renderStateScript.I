/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderStateScript.I
 * @author lachbr
 * @date 2020-10-13
 */

/**
 *
 */
INLINE RenderStateScript::
RenderStateScript() {
  _state = nullptr;
}

/**
 * Returns the RenderState corresponding to this script.
 */
INLINE const RenderState *RenderStateScript::
get_state() const {
  return _state;
}

/**
 * Returns the filename the script was loaded from.
 */
INLINE const Filename &RenderStateScript::
get_filename() const {
  return _filename;
}
