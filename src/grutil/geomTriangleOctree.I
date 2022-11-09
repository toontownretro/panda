/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTriangleOctree.I
 * @author brian
 * @date 2022-11-09
 */

/**
 *
 */
INLINE GeomTriangleOctree::
GeomTriangleOctree() :
  _geom(nullptr),
  _vdata(nullptr),
  _min_size(1.0f),
  _min_tris(3)
{
}

/**
 *
 */
INLINE bool GeomTriangleOctree::OctreeNode::
is_leaf() const {
  return _children[0] == nullptr;
}

/**
 *
 */
INLINE GeomTriangleOctree::OctreeNode *GeomTriangleOctree::
get_root() const {
  return _root;
}

/**
 *
 */
INLINE const int *GeomTriangleOctree::
get_triangle(int i) const {
  nassertr(i >= 0 && i < (int)_all_tris.size(), nullptr);
  return _all_tris[i].data();
}
