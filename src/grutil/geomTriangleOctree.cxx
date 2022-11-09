/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTriangleOctree.cxx
 * @author brian
 * @date 2022-11-09
 */

#include "geomTriangleOctree.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "mathutil_misc.h"

/**
 *
 */
void GeomTriangleOctree::
build(const Geom *geom, const LVecBase3 &min_size, int min_tris) {
  _geom = geom;
  _vdata = geom->get_vertex_data();
  _min_size = min_size;
  _min_tris = min_tris;

  for (int i = 0; i < geom->get_num_primitives(); ++i) {
    const GeomPrimitive *prim = geom->get_primitive(i);

    TypeHandle prim_type = prim->get_type();

    if (prim_type == GeomTriangles::get_class_type()) {
      for (int j = 0; j < prim->get_num_primitives(); ++j) {
        int start = prim->get_primitive_start(j);
        _all_tris.push_back({ prim->get_vertex(start), prim->get_vertex(start + 1), prim->get_vertex(start + 2) });
      }

    } else if (prim_type == GeomTristrips::get_class_type()) {
      // Extract triangles from tristrip indices.

      CPTA_int ends = prim->get_ends();
      int num_vertices = prim->get_num_vertices();
      int num_unused = prim->get_num_unused_vertices_per_primitive();

      int vi = -num_unused;
      int li = 0;
      while (li < (int)ends.size()) {
        vi += num_unused;
        int end = ends[li];
        nassertv(vi + 2 <= end);
        int v0 = prim->get_vertex(vi);
        ++vi;
        int v1 = prim->get_vertex(vi);
        ++vi;
        bool reversed = false;
        while (vi < end) {
          int v2 = prim->get_vertex(vi);
          ++vi;
          if (reversed) {
            if (v0 != v1 && v0 != v2 && v1 != v2) {
              _all_tris.push_back({ v0, v2, v1 });
            }

            reversed = false;
          } else {
            if (v0 != v1 && v0 != v2 && v1 != v2) {
              _all_tris.push_back({ v0, v1, v2 });
            }
            reversed = true;
          }
          v0 = v1;
          v1 = v2;
        }
        ++li;
      }

      nassertv(vi == num_vertices);
    }

  }

  // Start with the root node, enclosing the entire Geom.
  vector_int triangles;
  triangles.resize(_all_tris.size());
  for (int i = 0; i < (int)_all_tris.size(); ++i) {
    triangles.push_back(i);
  }
  _root = new OctreeNode;
  _root->_bounds = new BoundingBox;
  _root->_bounds->extend_by(_geom->get_bounds()->as_geometric_bounding_volume());
  std::cout << "root bounds " << _root->_bounds->get_minq() << " " << _root->_bounds->get_maxq() << "\n";
  r_subdivide(_root, triangles);
}

/**
 *
 */
void GeomTriangleOctree::
r_subdivide(OctreeNode *node, const vector_int &tris) {
  if (tris.empty()) {
    return;
  }

  if (tris.size() <= _min_tris) {
    // Reached minimum triangle count.  Stop here and put the triangles in
    // there.
    node->_triangles = tris;
    return;
  }

  LVecBase3 node_size = node->_bounds->get_maxq() - node->_bounds->get_minq();
  if (node_size[0] <= _min_size[0] && node_size[1] <= _min_size[1] && node_size[2] <= _min_size[2]) {
    // Reached minimum size.  Stop here and put the triangles in there.
    node->_triangles = tris;
    return;
  }

  // Otherwise we need to divide.
  for (int i = 0; i < 8; ++i) {
    LPoint3 this_mins = node->_bounds->get_minq();
    LPoint3 this_maxs = node->_bounds->get_maxq();

    LVector3 size = this_maxs - this_mins;
    size *= 0.5f;

    if (i & 4) {
      this_mins[0] += size[0];
    }
    if (i & 2) {
      this_mins[1] += size[1];
    }
    if (i & 1) {
      this_mins[2] += size[2];
    }

    this_maxs = this_mins + size;
    LVector3 qsize = size * 0.5f;

    // Determine list of triangles in this child octant.
    vector_int child_tris;
    GeomVertexReader reader(_vdata, InternalName::get_vertex());
    for (int tri_index : tris) {
      const int *vertices = _all_tris[tri_index].data();
      reader.set_row(vertices[0]);
      LPoint3 p1 = reader.get_data3f();
      reader.set_row(vertices[1]);
      LPoint3 p2 = reader.get_data3f();
      reader.set_row(vertices[2]);
      LPoint3 p3 = reader.get_data3f();
      if (tri_box_overlap(this_mins + qsize, qsize, p1, p2, p3)) {
        // Triangle is part of this child octant.
        child_tris.push_back(tri_index);
      }
    }

    // Create a child and recurse.
    PT(OctreeNode) child = new OctreeNode;
    child->_bounds = new BoundingBox(this_mins, this_maxs);
    r_subdivide(child, child_tris);
    node->_children[i] = child;
  }
}
