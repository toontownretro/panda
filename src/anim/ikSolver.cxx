/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikSolver.cxx
 * @author lachbr
 * @date 2021-02-12
 */

#include "ikSolver.h"

#include "mathutil_misc.h"

/**
 *
 */
bool IKSolver::
solve(PN_stdfloat A, PN_stdfloat B, const LPoint3 &P, const LPoint3 &D, LPoint3 &Q) {
  LPoint3 R;
  define_m(P, D);
  R = _inverse.xform_vec(P);
  PN_stdfloat r = R.length();
  PN_stdfloat d = find_d(A, B, r);
  PN_stdfloat e = find_e(A, d);
  LPoint3 S(d, e, 0);
  Q = _forward.xform_vec(S);
  return d > (r - B) && d < A;
}

/**
 * If "knee" position Q needs to be as close as possible to some point D,
 * then choose M such that M(D) is in the y>0 half of the z=0 plane.
 *
 * Given that constraint, define the forward and inverse of M as follows:
 */
void IKSolver::
define_m(const LPoint3 &P, const LPoint3 &D) {
  // Minv defines a coordinate system whose x axis contains P, so X = unit(P).
  LPoint3 X = P.normalized();

  // Its Y axis is perpendicular to P, so Y = unit(E - X(EdotX)).
  PN_stdfloat d_dot_x = D.dot(X);
  LPoint3 Y = (D - d_dot_x * X).normalized();

  // Its Z axis is perpendicular to both X and Y, so Z = cross(X,Y).
  LPoint3 Z = X.cross(Y);

  _inverse = LMatrix3(X, Y, Z);
  _forward.transpose_from(_inverse);
}

/**
 *
 */
PN_stdfloat IKSolver::
find_d(PN_stdfloat a, PN_stdfloat b, PN_stdfloat c) {
  return (c + (a * a - b * b) / c) * 0.5;
}

/**
 *
 */
PN_stdfloat IKSolver::
find_e(PN_stdfloat a, PN_stdfloat d) {
  return std::sqrt(a * a - d * d);
}
