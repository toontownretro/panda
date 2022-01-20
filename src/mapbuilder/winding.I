/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winding.I
 * @author brian
 * @date 2021-07-07
 */

/**
 * Initializes a winding from the indicated plane.
 */
template<int MaxPoints>
BaseWinding<MaxPoints>::
BaseWinding(const LPlane &plane) {
  // Project a really big axis-aligned box onto the plane.

  LVector3 normal = plane.get_normal();
  PN_stdfloat dist = plane.get_distance();

  LVector3 x;
  // Find the major axis.
  if (cabs(normal[0]) >= cabs(normal[1]) && cabs(normal[0]) >= cabs(normal[2])) {
    x = LVector3::unit_x();
  } else if (cabs(normal[1]) >= cabs(normal[2])) {
    x = LVector3::unit_y();
  } else {
    x = LVector3::unit_z();
  }

  LVector3 up = (x == LVector3::unit_z()) ? LVector3::unit_x() : LVector3::unit_z();

  PN_stdfloat v = up.dot(normal);
  up += normal * -v;
  up.normalize();

  LPoint3 org = normal * dist;

  LVector3 right = up.cross(normal);

  up = up * 65536;
  right = right * 65536;

  _num_points = 4;
  // Counter-clockwise ordering.
  _points[0] = (org - right) + up;
  _points[1] = (org + right) + up;
  _points[2] = (org + right) - up;
  _points[3] = (org - right) - up;

//#ifndef NDEBUG
//  LPlane winding_plane = get_plane();
//  if (winding_plane != plane) {
//    mapbuilder_cat.error()
//      << "BaseWinding plane does not match input plane\n";
//    mapbuilder_cat.error(false)
//      << "Input plane: " << plane << "\n"
//      << "BaseWinding plane: " << winding_plane << "\n";
//  }
//#endif
}

/**
 *
 */
template<int MaxPoints>
BaseWinding<MaxPoints>::
BaseWinding(const BaseWinding &copy) :
  _num_points(copy._num_points)
{
  std::copy(copy._points, copy._points + copy._num_points, _points);
}

/**
 *
 */
template<int MaxPoints>
BaseWinding<MaxPoints>::
BaseWinding(BaseWinding &&other) :
  _num_points(std::move(other._num_points))
{
  std::move(other._points, other._points + _num_points, _points);
}

/**
 *
 */
template<int MaxPoints>
BaseWinding<MaxPoints>::
BaseWinding() :
  _num_points(0)
{
}

/**
 *
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
operator = (const BaseWinding &copy) {
  _num_points = copy._num_points;
  std::copy(copy._points, copy._points + copy._num_points, _points);
}

/**
 *
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
operator = (BaseWinding &&other) {
  _num_points = std::move(other._num_points);
  std::move(other._points, other._points + _num_points, _points);
}

/**
 * Returns the area of the winding.
 */
template<int MaxPoints>
PN_stdfloat BaseWinding<MaxPoints>::
get_area() const {
  PN_stdfloat total = 0.0f;
  for (size_t i = 2; i < _num_points; i++) {
    LVector3 d1 = _points[i - 1] - _points[0];
    LVector3 d2 = _points[i] - _points[0];
    LVector3 cross = d1.cross(d2);
    total += cross.length();
  }

  return total * 0.5f;
}

/**
 * Returns the center of the winding.
 */
template<int MaxPoints>
LPoint3 BaseWinding<MaxPoints>::
get_center() const {
  LPoint3 center(0);
  for (size_t i = 0; i < _num_points; i++) {
    center += _points[i];
  }
  center /= _num_points;
  return center;
}

/**
 * Returns the plane that the winding lies on.
 */
template<int MaxPoints>
LPlane BaseWinding<MaxPoints>::
get_plane() const {
  return LPlane(_points[1], _points[0], _points[2]);
}

/**
 * Fills the indicated points with the minimum and maximum bounds of the
 * winding.
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
get_bounds(LPoint3 &mins, LPoint3 &maxs) const {
  mins.set(1e+9, 1e+9, 1e+9);
  maxs.set(-1e+9, -1e+9, -1e+9);

  for (int i = 0; i < _num_points; i++) {
    const LPoint3 &point = _points[i];
    mins[0] = std::min(point[0], mins[0]);
    mins[1] = std::min(point[1], mins[1]);
    mins[2] = std::min(point[2], mins[2]);
    maxs[0] = std::max(point[0], maxs[0]);
    maxs[1] = std::max(point[1], maxs[1]);
    maxs[2] = std::max(point[2], maxs[2]);
  }
}

/**
 * Returns the winding's area and area-weighted center.
 */
template<int MaxPoints>
PN_stdfloat BaseWinding<MaxPoints>::
get_area_and_balance_point(LPoint3 &balance_point) const {
  PN_stdfloat total = 0.0f;
  for (size_t i = 2; i < _num_points; i++) {
    LVector3 d1 = _points[i - 1] - _points[0];
    LVector3 d2 = _points[i] - _points[0];
    LVector3 cross = d1.cross(d2);
    PN_stdfloat area = cross.length();
    total += area;

    // Center of triangle, weighted by area.
    balance_point += _points[i - 1] * (area / 3.0f);
    balance_point += _points[i] * (area / 3.0f);
    balance_point += _points[0] * (area / 3.0f);
  }

  if (total != 0.0f) {
    balance_point /= total;
  }

  return total * 0.5f;
}

/**
 *
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
clip_epsilon(const LPlane &plane, PN_stdfloat epsilon, BaseWinding &front, BaseWinding &back) const {
  PN_stdfloat dists[MaxPoints + 4];
  int sides[MaxPoints + 4];
  int counts[3] = { 0, 0, 0 };

  LVector3 normal = plane.get_normal();
  PN_stdfloat distance = plane.get_distance();

  // Determine sides for each point.
  for (size_t i = 0; i < _num_points; i++) {
    PN_stdfloat dist = plane.dist_to_plane(_points[i]);
    dists[i] = dist;
    if (dist > epsilon) {
      sides[i] = PS_front;

    } else if (dist < -epsilon) {
      sides[i] = PS_back;

    } else {
      sides[i] = PS_on;
    }
    counts[sides[i]]++;
  }

  dists[_num_points] = dists[0];
  sides[_num_points] = sides[0];

  front.clear();
  back.clear();

  if (!counts[PS_front]) {
    back = BaseWinding(*this);
    return;
  }
  if (!counts[PS_back]) {
    front = BaseWinding(*this);
    return;
  }

  for (size_t i = 0; i < _num_points; i++) {
    const LPoint3 &p1 = _points[i];

    if (sides[i] == PS_on) {
      front.add_point(p1);
      back.add_point(p1);
      continue;

    } else if (sides[i] == PS_front) {
      front.add_point(p1);

    } else if (sides[i] == PS_back) {
      back.add_point(p1);
    }

    if (sides[i + 1] == PS_on || sides[i + 1] == sides[i]) {
      continue;
    }

    // Generate a split point.
    const LPoint3 &p2 = _points[(i + 1) % _num_points];

    LPoint3 mid(0);
    PN_stdfloat dot = dists[i] / (dists[i] - dists[i + 1]);
    for (size_t j = 0; j < 3; j++) {
      if (normal[j] == 1) {
        mid[j] = distance;
      } else if (normal[j] == -1) {
        mid[j] = -distance;
      } else {
        mid[j] = p1[j] + dot * (p2[j] - p1[j]);
      }
    }

    front.add_point(mid);
    back.add_point(mid);
  }

  front.round_points();
  back.round_points();

  //front->remove_colinear_points();
  //back->remove_colinear_points();
}


/**
 *
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
clip_epsilon_offset(const LPlane &plane, PN_stdfloat epsilon,
                    BaseWinding &front, BaseWinding &back,
                    const LVecBase3 &offset) {
  translate(offset);
  LPlane trans_plane = plane;
  trans_plane[3] += offset.dot(plane.get_normal());
  clip_epsilon(trans_plane, epsilon, front, back);
  translate(-offset);
  if (!front.is_empty()) {
    front.translate(-offset);
  }
  if (!back.is_empty()) {
    back.translate(-offset);
  }
}

/**
 * Returns the fragment of the winding that is on the front side of the clipping
 * plane.
 */
template<int MaxPoints>
BaseWinding<MaxPoints> BaseWinding<MaxPoints>::
chop(const LPlane &plane) const {
  BaseWinding<MaxPoints> front, back;
  clip_epsilon(plane, ON_EPSILON, front, back);
  return front;
}

/**
 * Returns true if the winding contains the indicated point, false otherwise.
 */
template<int MaxPoints>
bool BaseWinding<MaxPoints>::
contains_point(const LPoint3 &point) const {
  // Get the first normal to test.
  LVector3 to_pt = point - _points[0];
  LVector3 edge = _points[1] - _points[0];
  LVector3 test_cross = edge.cross(to_pt);
  test_cross.normalize();

  for (size_t i = 0; i < _num_points; i++) {
    to_pt = point - _points[i];
    edge = _points[(i + 1) % _num_points] - _points[i];
    LVector3 cross = edge.cross(to_pt);
    cross.normalize();

    if (cross.dot(test_cross) < 0.0f) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
translate(const LVecBase3 &offset) {
  for (size_t i = 0; i < _num_points; i++) {
    _points[i] += offset;
  }
}

/**
 * Reverses the winding in place.
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
reverse() {
  std::reverse(_points, _points + _num_points);
}

/**
 * Does as it says.
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
remove_colinear_points() {
  LPoint3 new_points[MaxPoints];

  int count = 0;
  for (size_t i = 0; i < _num_points; i++) {
    size_t j = (i + 1) % _num_points;
    size_t k = (i + _num_points - 1) % _num_points;
    LVector3 v1 = _points[j] - _points[i];
    LVector3 v2 = _points[i] - _points[k];
    v1.normalize();
    v2.normalize();

    if (v1.dot(v2) < 0.999f) {
      new_points[count++] = _points[i];
    }
  }

  _num_points = count;
  std::move(new_points, new_points + count, _points);
}

/**
 * Rounds all winding points that are within "epsilon" of integer values.
 */
template<int MaxPoints>
void BaseWinding<MaxPoints>::
round_points(PN_stdfloat epsilon) {
  //for (int i = 0; i < _num_points; i++) {
  //  for (int j = 0; j < 3; j++) {
  //    PN_stdfloat v = _points[i][j];
  //    PN_stdfloat v1 = std::round(v);
  //    if ((v != v1) && std::abs(v - v1) < epsilon) {
  //      _points[i][j] = v1;
  //    }
  //  }
  //}
}

/**
 * Returns the side of the indicated plane that the winding lies on.
 */
template<int MaxPoints>
PlaneSide BaseWinding<MaxPoints>::
get_plane_side(const LPlane &plane) const {
  bool front = false;
  bool back = false;

  for (size_t i = 0; i < _num_points; i++) {
    PN_stdfloat d = plane.dist_to_plane(_points[i]);
    if (d < -ON_EPSILON) {
      if (front) {
        return PS_cross;
      }
      back = true;

    } else if (d > ON_EPSILON) {
      if (back) {
        return PS_cross;
      }
      front = true;
    }
  }

  if (back) {
    return PS_back;

  } else if (front) {
    return PS_front;

  } else {
    return PS_on;
  }
}


/**
 * Adds a point to the winding.
 */
template<int MaxPoints>
INLINE void BaseWinding<MaxPoints>::
add_point(const LPoint3 &point) {
  nassertv(_num_points < MaxPoints);
  _points[_num_points++] = point;
}

/**
 * Returns the number of points in the winding.
 */
template<int MaxPoints>
INLINE int BaseWinding<MaxPoints>::
get_num_points() const {
  return _num_points;
}

/**
 * Returns the nth point of the winding.
 */
template<int MaxPoints>
INLINE const LPoint3 &BaseWinding<MaxPoints>::
get_point(int n) const {
  assert(n >= 0 && n < _num_points);
  return _points[n];
}

/**
 * Returns the winding's vector of points.
 */
template<int MaxPoints>
INLINE const LPoint3 *BaseWinding<MaxPoints>::
get_points() const {
  return _points;
}

/**
 * Returns true if the winding has no points.
 */
template<int MaxPoints>
INLINE bool BaseWinding<MaxPoints>::
is_empty() const {
  return _num_points == 0;
}

/**
 * Clears all the points in the winding.
 */
template<int MaxPoints>
INLINE void BaseWinding<MaxPoints>::
clear() {
  _num_points = 0;
}

/**
 * Returns the index of the point in the winding that is closest to the
 * indicated point.
 */
template<int MaxPoints>
INLINE int BaseWinding<MaxPoints>::
get_closest_point(const LPoint3 &point) {
  if (_num_points == 0) {
    return -1;

  } else if (_num_points == 1) {
    return 0;

  } else {
    int index = 0;
    PN_stdfloat dist = (point - _points[0]).length();
    for (int i = 1; i < _num_points; i++) {
      PN_stdfloat this_dist = (point - _points[i]).length();
      if (this_dist < dist) {
        index = i;
        dist = this_dist;
      }
    }

    return index;
  }
}

/**
 *
 */
template<int MaxPoints>
INLINE bool BaseWinding<MaxPoints>::
is_tiny() {
  int edges = 0;
  for (int i = 0; i < _num_points; ++i) {
    int j = i == _num_points - 1 ? 0 : i + 1;
    LVector3 delta = _points[j] - _points[i];
    PN_stdfloat len = delta.length();
    if (len > 0.2f) {
      if (++edges == 3) {
        return false;
      }
    }
  }

  return true;
}
