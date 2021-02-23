/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBase.I
 * @author drose
 * @date 1999-02-19
 */

/**
 * Don't use this constructor.  It exists only so that AnimChannelFixed may
 * define itself outside of the hierarchy.  Normally, an AnimChannel must be
 * created as part of a hierarchy.
 */
INLINE AnimChannelBase::
AnimChannelBase(const std::string &name)
  : Namable(name)
{
  _root = nullptr;
  _last_frame = -1;
}

/**
 * Creates a new AnimChannelBase, just like this one, without copying any
 * children.  The new copy is added to the indicated parent.  Intended to be
 * called by make_copy() only.
 */
INLINE AnimChannelBase::
AnimChannelBase(const AnimChannelBase &copy) :
  TypedWritableReferenceCount(copy),
  Namable(copy),
  _last_frame(copy._last_frame),
  _root(copy._root)
{
}
