/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspRoot.cxx
 * @author lachbr
 * @date 2021-01-02
 */

#include "bspRoot.h"

TypeHandle BSPRoot::_type_handle;

/**
 *
 */
BSPRoot::
BSPRoot(const BSPRoot &copy) :
  PandaNode(copy),
  _data(copy._data)
{
}

/**
 *
 */
PandaNode *BSPRoot::
make_copy() const {
  return new BSPRoot(*this);
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * PandaNode by duplicating instances (by calling dupe_for_flatten()), false
 * otherwise (for instance, a Camera cannot be safely flattened, because the
 * Camera pointer itself is meaningful).
 */
bool BSPRoot::
safe_to_flatten() const {
  return false;
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool BSPRoot::
safe_to_combine() const {
  return false;
}
