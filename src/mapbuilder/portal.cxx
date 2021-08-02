/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file portal.cxx
 * @author brian
 * @date 2021-07-13
 */

#include "portal.h"

/**
 *
 */
Portal::
~Portal() {
  if (_portal_front != nullptr) {
    delete[] _portal_front;
    _portal_front = nullptr;
  }
  if (_portal_flood != nullptr) {
    delete[] _portal_flood;
    _portal_flood = nullptr;
  }
  if (_portal_vis != nullptr) {
    delete[] _portal_vis;
    _portal_vis = nullptr;
  }
}

#ifndef CPPPARSER

/**
 * Returns a counter clock-wise quadrilateral that represents the geometry
 * of the portal.
 */
std::array<LPoint3, 4> Portal::
get_quad(const LVecBase3 &voxel_size, const LPoint3 &scene_min) const {
  LPoint3 min_point, max_point;
  min_point.set(_min_voxel[0], _min_voxel[1], _min_voxel[2]);
  max_point.set(_max_voxel[0], _max_voxel[1], _max_voxel[2]);

  LVecBase3 voxel_half = voxel_size * 0.5f;

  min_point.componentwise_mult(voxel_size);
  max_point.componentwise_mult(voxel_size);

  min_point += scene_min;
  max_point += scene_min;

  // Move the position to the middle of the voxel.
  min_point += voxel_half;
  max_point += voxel_half;

  // If the portal is facing into one direction, move towards that face of the
  // voxel.
  min_point[0] += voxel_half[0] * _plane[0];
  min_point[1] += voxel_half[1] * _plane[1];
  min_point[2] += voxel_half[2] * _plane[2];

  max_point[0] += voxel_half[0] * _plane[0];
  max_point[1] += voxel_half[1] * _plane[1];
  max_point[2] += voxel_half[2] * _plane[2];

  // Move half voxel for the extreme points.
  if (_plane[0] == 0) {
    min_point[0] -= voxel_half[0];
    max_point[0] += voxel_half[0];
  }
  if (_plane[1] == 0) {
    min_point[1] -= voxel_half[1];
    max_point[1] += voxel_half[1];
  }
  if (_plane[2] == 0) {
    min_point[2] -= voxel_half[2];
    max_point[2] += voxel_half[2];
  }

  LVector3 rect_size = max_point - min_point;

  std::array<LPoint3, 4> quad_points;

  // Define the diagonal, that indicates the max and min points of the quad.
  quad_points[0] = min_point;
  quad_points[2] = max_point;

  // Given the two extreme points, calculate the other two points that make a
  // quad.  They have to be counter clock-wise respect of the facing normal.

  quad_points[1] = min_point;
  quad_points[3] = min_point;

  // +X
  if (_plane[0] > 0) {
    quad_points[1][2] += rect_size[2];
    quad_points[3][1] += rect_size[1];
  }

  // -X
  if (_plane[0] < 0) {
    quad_points[1][1] += rect_size[1];
    quad_points[3][2] += rect_size[2];
  }

  // +Y
  if (_plane[1] > 0) {
    quad_points[1][0] += rect_size[0];
    quad_points[3][2] += rect_size[2];
  }

  // -Y
  if (_plane[1] < 0) {
    quad_points[1][2] += rect_size[2];
    quad_points[3][0] += rect_size[0];
  }

  // +Z
  if (_plane[2] > 0) {
    quad_points[1][1] += rect_size[1];
    quad_points[3][0] += rect_size[0];
  }

  // -Z
  if (_plane[2] < 0) {
    quad_points[1][0] += rect_size[0];
    quad_points[3][1] += rect_size[1];
  }

  return quad_points;
}

#endif

/**
 * Calculates the approximate radius of the portal.
 */
void Portal::
calc_radius() {
  PortalWinding *w = &_winding;
  LVector3 total(0);
  for (int i = 0; i < w->get_num_points(); i++) {
    total += w->get_point(i);
  }
  total /= w->get_num_points();

  PN_stdfloat best = 0;
  for (int i = 0; i < w->get_num_points(); i++) {
    LVector3 dist = w->get_point(i) - total;
    PN_stdfloat r = dist.length();
    if (r > best) {
      best = r;
    }
  }

  _radius = best;
}
