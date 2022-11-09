/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTriangleOctree.h
 * @author brian
 * @date 2022-11-09
 */

#ifndef GEOMTRIANGLEOCTREE_H
#define GEOMTRIANGLEOCTREE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "boundingBox.h"
#include "luse.h"
#include "vector_int.h"

#ifndef CPPPARSER
#include <array>
#endif

class Geom;
class GeomVertexData;

/**
 * An octree containing the triangles of a Geom.
 * Allows for quick spatial searches for triangles in a Geom.
 */
class EXPCL_PANDA_GRUTIL GeomTriangleOctree : public ReferenceCount {
PUBLISHED:
#ifndef CPPPARSER
  typedef pvector<std::array<int, 3>> TriangleList;
#endif

  class OctreeNode : public ReferenceCount {
  public:
    INLINE OctreeNode() = default;

    // List of triangles at a leaf.  These are indices
    // into the Geom's vertex data.
    vector_int _triangles;

    PT(OctreeNode) _children[8];
    PT(BoundingBox) _bounds;

    INLINE bool is_leaf() const;
  };

  INLINE GeomTriangleOctree();

  void build(const Geom *geom, const LVecBase3 &min_size, int min_tris);
  INLINE OctreeNode *get_root() const;
  INLINE const int *get_triangle(int i) const;

private:
  void r_subdivide(OctreeNode *node, const vector_int &triangles);

private:
  const Geom *_geom;
  const GeomVertexData *_vdata;
  LVecBase3 _min_size;
  int _min_tris;

#ifndef CPPPARSER
  TriangleList _all_tris;
#endif

  PT(OctreeNode) _root;
};

#include "geomTriangleOctree.I"

#endif // GEOMTRIANGLEOCTREE_H
