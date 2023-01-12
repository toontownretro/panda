/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulatorDelaunay.I
 * @author brian
 * @date 2021-02-17
 */

/**
 *
 */
INLINE TriangulatorDelaunay::Triangle::
Triangle() {
}

/**
 *
 */
INLINE TriangulatorDelaunay::Triangle::
Triangle(int a, int b, int c) {
  _a = a;
  _b = b;
  _c = c;
}

/**
 *
 */
INLINE TriangulatorDelaunay::Edge::
Edge() {
}

/**
 *
 */
INLINE TriangulatorDelaunay::Edge::
Edge(int a, int b) {
  _a = a;
  _b = b;
}

/**
 *
 */
INLINE TriangulatorDelaunay::
TriangulatorDelaunay() {
}

/**
 * Adds a point.
 */
INLINE void TriangulatorDelaunay::
add_point(const LPoint2 &point) {
  _points.push_back(point);
}

/**
 * Returns the number of points.
 */
INLINE int TriangulatorDelaunay::
get_num_points() const {
  return (int)_points.size();
}

/**
 * Returns the nth point.
 */
INLINE LPoint2 TriangulatorDelaunay::
get_point(int n) const {
  return _points[n];
}

/**
 * Returns the number of triangles produced.
 */
INLINE int TriangulatorDelaunay::
get_num_triangles() const {
  return (int)_triangles.size();
}

/**
 * Returns the nth triangle produced.
 */
INLINE const TriangulatorDelaunay::Triangle &TriangulatorDelaunay::
get_triangle(int n) const {
  return _triangles[n];
}
