/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulatorDelaunay.cxx
 * @author lachbr
 * @date 2021-02-17
 */

#include "triangulatorDelaunay.h"

/**
 * Produces a set of triangles from the given points.
 */
void TriangulatorDelaunay::
triangulate() {
  // Build a rectangle around all the points.
  LPoint2 rect_position;
  LPoint2 rect_size(-9999999, -9999999);

  pvector<LPoint2> points = _points;

  for (size_t i = 0; i < _points.size(); i++) {
    if (i == 0) {
      rect_position = _points[i];

    } else {
      LPoint2 begin = rect_position;
      LPoint2 end = begin + rect_size;
      LPoint2 point = _points[i];

      if (point[0] < begin[0]) {
        begin[0] = point[0];
      }

      if (point[1] < begin[1]) {
        begin[1] = point[1];
      }

      if (point[0] > end[0]) {
        end[0] = point[0];
      }

      if (point[1] > end[1]) {
        end[1] = point[1];
      }

      rect_position = begin;
      rect_size = end - begin;
    }
  }

  PN_stdfloat delta_max = std::max(rect_size[0], rect_size[1]);
  LPoint2 center = rect_position + rect_size * 0.5f;

  points.push_back(LPoint2(center[0] - 20 * delta_max, center[1] - delta_max));
  points.push_back(LPoint2(center[0], center[1] + 20 * delta_max));
  points.push_back(LPoint2(center[0] + 20 * delta_max, center[1] - delta_max));

  _triangles.push_back(Triangle(_points.size(), _points.size() + 1, _points.size() + 2));

  for (size_t i = 0; i < _points.size(); i++) {
    pvector<Edge> polygon;

    for (size_t j = 0; j < _triangles.size(); j++) {
      if (circum_circle_contains(points, _triangles[j], i)) {
        _triangles[j]._bad = true;
        polygon.push_back(Edge(_triangles[j]._a, _triangles[j]._b));
        polygon.push_back(Edge(_triangles[j]._b, _triangles[j]._c));
        polygon.push_back(Edge(_triangles[j]._c, _triangles[j]._a));
      }
    }

    Triangles::iterator ti;
    for (ti = _triangles.begin(); ti != _triangles.end();) {
      if ((*ti)._bad) {
        ti = _triangles.erase(ti);

      } else {
        ++ti;
      }
    }

    for (size_t j = 0; j < polygon.size(); j++) {
      for (size_t k = j + 1; k < polygon.size(); k++) {
        if (edge_compare(points, polygon[j], polygon[k])) {
          polygon[j]._bad = true;
          polygon[k]._bad = true;
        }
      }
    }

    for (size_t j = 0; j < polygon.size(); j++) {
      if (polygon[j]._bad) {
        continue;
      }

      _triangles.push_back(Triangle(polygon[j]._a, polygon[j]._b, i));
    }
  }

  Triangles::iterator ti;
  for (ti = _triangles.begin(); ti != _triangles.end();) {
    Triangle &tri = *ti;

    if (tri._a >= _points.size() ||
        tri._b >= _points.size() ||
        tri._c >= _points.size()) {

      // Invalid triangle.
      ti = _triangles.erase(ti);

    } else {
      ++ti;
    }
  }
}

/**
 *
 */
bool TriangulatorDelaunay::
circum_circle_contains(const pvector<LPoint2> &points,
                       const TriangulatorDelaunay::Triangle &tri,
                       size_t n) {
  LPoint2 p1 = points[tri._a];
  LPoint2 p2 = points[tri._b];
  LPoint2 p3 = points[tri._c];

  PN_stdfloat ab = p1[0] * p1[0] + p1[1] * p1[1];
  PN_stdfloat cd = p2[0] * p2[0] + p2[1] * p2[1];
  PN_stdfloat ef = p3[0] * p3[0] + p3[1] * p3[1];

  LPoint2 circum(
    (ab * (p3[1] - p2[1]) + cd * (p1[1] - p3[1]) + ef * (p2[1] - p1[1])) / (p1[0] * (p3[1] - p2[1]) + p2[0] * (p1[1] - p3[1]) + p3[0] * (p2[1] - p1[1])),
		(ab * (p3[0] - p2[0]) + cd * (p1[0] - p3[0]) + ef * (p2[0] - p1[0])) / (p1[1] * (p3[0] - p2[0]) + p2[1] * (p1[0] - p3[0]) + p3[1] * (p2[0] - p1[0]))
  );
  circum *= 0.5f;

  PN_stdfloat r = (circum - p1).length_squared();
  PN_stdfloat d = (circum - points[n]).length_squared();
  return d <= r;
}

/**
 *
 */
bool TriangulatorDelaunay::
edge_compare(const pvector<LPoint2> &points, const Edge &a, const Edge &b) {
  if (points[a._a].almost_equal(points[b._a]) &&
      points[a._b].almost_equal(points[b._b])) {
    return true;
  }

  if (points[a._a].almost_equal(points[b._b]) &&
      points[a._b].almost_equal(points[b._a])) {
    return true;
  }

  return false;
}
