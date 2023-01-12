/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulatorDelaunay.h
 * @author brian
 * @date 2021-02-17
 */

#ifndef TRIANGULATORDELAUNAY_H
#define TRIANGULATORDELAUNAY_H

#include "pandabase.h"
#include "pvector.h"
#include "luse.h"

/**
 * Class that triangulates a set of discrete 2D points using the Delaunay
 * Triangulation algorithm.
 */
class EXPCL_PANDA_MATHUTIL TriangulatorDelaunay {
PUBLISHED:
  class Triangle {
  PUBLISHED:
    int _a, _b, _c;
    bool _bad = false;

    INLINE Triangle();
    INLINE Triangle(int a, int b, int c);
  };

  class Edge {
  PUBLISHED:
    int _a, _b;
    bool _bad = false;

    INLINE Edge();
    INLINE Edge(int a, int b);
  };

  INLINE TriangulatorDelaunay();

  INLINE void add_point(const LPoint2 &point);
  INLINE int get_num_points() const;
  INLINE LPoint2 get_point(int n) const;

  INLINE int get_num_triangles() const;
  INLINE const Triangle &get_triangle(int n) const;

  void triangulate();

public:
  bool circum_circle_contains(const pvector<LPoint2> &points, const Triangle &tri, size_t j);
  bool edge_compare(const pvector<LPoint2> &points, const Edge &a, const Edge &b);

private:
  pvector<LPoint2> _points;

  typedef pvector<Triangle> Triangles;
  Triangles _triangles;
};

#include "triangulatorDelaunay.I"

#endif // TRIANGULATORDELAUNAY_H
