/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lquaternion_src.cxx
 */

#include "config_linmath.h"
#include "lmatrix.h"
#include "luse.h"

TypeHandle FLOATNAME(LQuaternion)::_type_handle;

const FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::_ident_quat =
  FLOATNAME(LQuaternion)(1.0f, 0.0f, 0.0f, 0.0f);

/**
 *
 */
FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
pure_imaginary(const FLOATNAME(LVector3) &v) {
  return FLOATNAME(LQuaternion)(0, v[0], v[1], v[2]);
}

/**
 * Returns a new quaternion that represents this quaternion raised to the
 * given power.
 */
FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
__pow__(FLOATTYPE power) const {
  if (IS_NEARLY_ZERO(power)) {
    return FLOATNAME(LQuaternion)(1, 0, 0, 0);
  }

  FLOATTYPE l = length();
  FLOATTYPE norm = _v(0) / l;
  if (IS_NEARLY_EQUAL(cabs(norm), (FLOATTYPE)1)) {
    return FLOATNAME(LQuaternion)(cpow(_v(0), power), 0, 0, 0);
  }

  FLOATTYPE angle = acos(norm);
  FLOATTYPE angle2 = angle * power;
  FLOATTYPE mag = cpow(l, power - 1);
  FLOATTYPE mult = mag * (sin(angle2) / sin(angle));
  return FLOATNAME(LQuaternion)(cos(angle2) * mag * l, _v(1) * mult, _v(2) * mult, _v(3) * mult);
}

/**
 * Based on the quat lib from VRPN.
 */
void FLOATNAME(LQuaternion)::
extract_to_matrix(FLOATNAME(LMatrix3) &m) const {
  FLOATTYPE N = this->dot(*this);
  FLOATTYPE s = (N == 0.0f) ? 0.0f : (2.0f / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _v(1) * s;   ys = _v(2) * s;   zs = _v(3) * s;
  wx = _v(0) * xs;  wy = _v(0) * ys;  wz = _v(0) * zs;
  xx = _v(1) * xs;  xy = _v(1) * ys;  xz = _v(1) * zs;
  yy = _v(2) * ys;  yz = _v(2) * zs;  zz = _v(3) * zs;

  m.set((1.0f - (yy + zz)), (xy + wz), (xz - wy),
        (xy - wz), (1.0f - (xx + zz)), (yz + wx),
        (xz + wy), (yz - wx), (1.0f - (xx + yy)));
}

/**
 * Based on the quat lib from VRPN.
 */
void FLOATNAME(LQuaternion)::
extract_to_matrix(FLOATNAME(LMatrix4) &m) const {
  FLOATTYPE N = this->dot(*this);
  FLOATTYPE s = (N == 0.0f) ? 0.0f : (2.0f / N);
  FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _v(1) * s;   ys = _v(2) * s;   zs = _v(3) * s;
  wx = _v(0) * xs;  wy = _v(0) * ys;  wz = _v(0) * zs;
  xx = _v(1) * xs;  xy = _v(1) * ys;  xz = _v(1) * zs;
  yy = _v(2) * ys;  yz = _v(2) * zs;  zz = _v(3) * zs;

  m.set((1.0f - (yy + zz)), (xy + wz), (xz - wy), 0.0f,
        (xy - wz), (1.0f - (xx + zz)), (yz + wx), 0.0f,
        (xz + wy), (yz - wx), (1.0f - (xx + yy)), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * Sets the quaternion as the unit quaternion that is equivalent to these
 * Euler angles.  (from Real-time Rendering, p.49)
 */
void FLOATNAME(LQuaternion)::
set_hpr(const FLOATNAME(LVecBase3) &hpr, CoordinateSystem cs) {
  FLOATNAME(LQuaternion) quat_h, quat_p, quat_r;

  FLOATNAME(LVector3) v;
  FLOATTYPE a, s, c;

  v = FLOATNAME(LVector3)::up(cs);
  a = deg_2_rad(hpr[0] * 0.5f);
  csincos(a, &s, &c);
  quat_h.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = FLOATNAME(LVector3)::right(cs);
  a = deg_2_rad(hpr[1] * 0.5f);
  csincos(a, &s, &c);
  quat_p.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = FLOATNAME(LVector3)::forward(cs);
  a = deg_2_rad(hpr[2] * 0.5f);
  csincos(a, &s, &c);
  quat_r.set(c, v[0] * s, v[1] * s, v[2] * s);

  if (is_right_handed(cs)) {
    (*this) = quat_r * quat_p * quat_h;
  } else {
    (*this) = invert(quat_h * quat_p * quat_r);
  }

#ifndef NDEBUG
  if (paranoid_hpr_quat) {
    FLOATNAME(LMatrix3) mat;
    compose_matrix(mat, FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f), hpr, cs);
    FLOATNAME(LQuaternion) compare;
    compare.set_from_matrix(mat);
    if (!compare.almost_equal(*this) && !compare.almost_equal(-(*this))) {
      linmath_cat.warning()
        << "hpr-to-quat of " << hpr << " computed " << *this
        << " instead of " << compare << "\n";
      (*this) = compare;
    }
  }
#endif  // NDEBUG
}

/**
 * Extracts the equivalent Euler angles from the unit quaternion.
 */
FLOATNAME(LVecBase3) FLOATNAME(LQuaternion)::
get_hpr(CoordinateSystem cs) const {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  FLOATNAME(LVecBase3) hpr;

  if (cs == CS_zup_right) {
    FLOATTYPE N =
        (_v(0) * _v(0)) +
        (_v(1) * _v(1)) +
        (_v(2) * _v(2)) +
        (_v(3) * _v(3));
    FLOATTYPE s = (N == 0.0f) ? 0.0f : (2.0f / N);
    FLOATTYPE xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, c1, c2, c3, c4;
    FLOATTYPE cr, sr, cp, sp, ch, sh;

    xs = _v(1) * s;   ys = _v(2) * s;   zs = _v(3) * s;
    wx = _v(0) * xs;  wy = _v(0) * ys;  wz = _v(0) * zs;
    xx = _v(1) * xs;  xy = _v(1) * ys;  xz = _v(1) * zs;
    yy = _v(2) * ys;  yz = _v(2) * zs;  zz = _v(3) * zs;
    c1 = xz - wy;
    c2 = 1.0f - (xx + yy);
    c3 = 1.0f - (yy + zz);
    c4 = xy + wz;

    if (c1 == 0.0f) {  // (roll = 0 or 180) or (pitch = +/- 90)
      if (c2 >= 0.0f) {
        hpr[2] = 0.0f;
        ch = c3;
        sh = c4;
        cp = c2;
      } else {
        hpr[2] = 180.0f;
        ch = -c3;
        sh = -c4;
        cp = -c2;
      }
    } else {
      // this should work all the time, but the above saves some trig
      // operations
      FLOATTYPE roll = catan2(-c1, c2);
      csincos(roll, &sr, &cr);
      hpr[2] = rad_2_deg(roll);
      ch = (cr * c3) + (sr * (xz + wy));
      sh = (cr * c4) + (sr * (yz - wx));
      cp = (cr * c2) - (sr * c1);
    }
    sp = yz + wx;
    hpr[0] = rad_2_deg(catan2(sh, ch));
    hpr[1] = rad_2_deg(catan2(sp, cp));

  } else {
    // The code above implements quat-to-hpr for CS_zup_right only.  For other
    // coordinate systems, someone is welcome to extend the implementation;
    // I'm going to choose the lazy path till then.
    FLOATNAME(LMatrix3) mat;
    extract_to_matrix(mat);
    FLOATNAME(LVecBase3) scale;
    decompose_matrix(mat, scale, hpr, cs);
    return hpr;
  }


#ifndef NDEBUG
  if (paranoid_hpr_quat) {
    FLOATNAME(LMatrix3) mat;
    extract_to_matrix(mat);
    FLOATNAME(LVecBase3) scale, compare_hpr;
    decompose_matrix(mat, scale, compare_hpr, cs);
    if (!compare_hpr.almost_equal(hpr)) {
      linmath_cat.warning()
        << "quat-to-hpr of " << *this << " computed " << hpr << " instead of "
        << compare_hpr << "\n";
      hpr = compare_hpr;
    }
  }
#endif  // NDEBUG

  return hpr;
}

/**
 *
 */
FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
find_between_normals(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b) {
  const FLOATTYPE norm_ab = 1.0f;
  FLOATTYPE w = norm_ab + a.dot(b);

  FLOATNAME(LQuaternion) quat;

  if (w >= 1e-6f * norm_ab) {
    quat.set(w,
             a[2] * b[3] - a[3] * b[2],
             a[3] * b[1] - a[1] * b[3],
             a[2] * b[2] - a[2] * b[1]);
  } else {
    // A and B in opposite directions.
    w = 0.0f;
    quat = std::abs(a[1]) > std::abs(a[2])
           ? FLOATNAME(LQuaternion)(w, -a[3], 0.0f, a[3])
           : FLOATNAME(LQuaternion)(w, 0.0f, -a[3], a[2]);
  }

  quat.normalize();
  return quat;

}

/**
 * Makes sure this quaternion is within 180 degrees of the given quaternion.
 * If not, reverses this quaternion.
 */
void FLOATNAME(LQuaternion)::
align(const FLOATNAME(LQuaternion) &p, const FLOATNAME(LQuaternion) &q, FLOATNAME(LQuaternion) &qt) {
  int i;
  FLOATTYPE a = 0.0f;
  FLOATTYPE b = 0.0f;
  for (i = 0; i < 4; i++) {
    a += (p[i] - q[i]) * (p[i] - q[i]);
    b += (p[i] + q[i]) * (p[i] + q[i]);
  }

  if (a > b) {
    qt = -q;
  } else if (&qt != &q) {
    qt = q;
  }
}

/**
 * Do a piecewise addition of the quaternion elements.  This makes little
 * mathematical sense, but it's a cheap way to simulate a slerp.
 */
void FLOATNAME(LQuaternion)::
blend(const FLOATNAME(LQuaternion) &p, const FLOATNAME(LQuaternion) &q, FLOATTYPE t,
      FLOATNAME(LQuaternion) &qt) {
  // Decide if one of the quaternions is backwards.
  FLOATNAME(LQuaternion) q2;
  align(p, q, q2);
  blend_no_align(p, q2, t, qt);
}

/**
 * Piecewise addition of quaternion elements without aligning this quaternion
 * to the other.
 */
void FLOATNAME(LQuaternion)::
blend_no_align(const FLOATNAME(LQuaternion) &p, const FLOATNAME(LQuaternion) &q,
               FLOATTYPE t, FLOATNAME(LQuaternion) &qt) {
  FLOATTYPE sclp, sclq;

  // 0.0 returns p, 1.0 returns q.
  sclp = 1.0f - t;
  sclq = t;
  qt = sclp * p + sclq * q;
  qt.normalize();
}

/**
 *
 */
void FLOATNAME(LQuaternion)::
identity_blend(const FLOATNAME(LQuaternion) &p, FLOATTYPE t, FLOATNAME(LQuaternion) &qt) {
  FLOATTYPE sclp;
  sclp = 1.0f - t;

  qt[1] = p[1] * sclp;
  qt[2] = p[2] * sclp;
  qt[3] = p[3] * sclp;

  if (qt[0] < 0.0f) {
    qt[0] = p[0] * sclp - t;
  } else {
    qt[0] = p[0] * sclp + t;
  }

  qt.normalize();
}

/**
 *
 */
void FLOATNAME(LQuaternion)::
slerp(const FLOATNAME(LQuaternion) &p, const FLOATNAME(LQuaternion) &q, FLOATTYPE t,
      FLOATNAME(LQuaternion) &qt) {
  FLOATNAME(LQuaternion) q2;
  align(p, q, q2);
  slerp_no_align(p, q2, t, qt);
}

/**
 *
 */
void FLOATNAME(LQuaternion)::
slerp_no_align(const FLOATNAME(LQuaternion) &p, const FLOATNAME(LQuaternion) &q,
               FLOATTYPE t, FLOATNAME(LQuaternion) &qt) {
  FLOATTYPE omega, cosom, sinom, sclp, sclq;
  int i;

  // 0.0 return sp, 1.0 returns q.

  cosom = p.dot(q);

  if ((1.0f + cosom) > 0.000001f) {
    if ((1.0f - cosom) > 0.000001f) {
      omega = std::acos(cosom);
      sinom = std::sin(omega);
      sclp = std::sin((1.0f - t) * omega) / sinom;
      sclq = std::sin(t * omega) / sinom;

    } else {
      sclp = 1.0f - t;
      sclq = t;
    }
    qt = sclp * p + sclq * q;

  } else {
    qt[1] = -q[2];
    qt[2] = q[1];
    qt[3] = -q[0];
    qt[0] = q[3];
    sclp = std::sin((1.0f - t) * (0.5f * M_PI));
    sclq = std::sin(t * (0.5f * M_PI));
    for (i = 1; i < 4; i++) {
      qt[i] = sclp * p[i] + sclq * qt[i];
    }
  }
}

/**
 * Sets the quaternion according to the rotation represented by the matrix.
 * Originally we tried an algorithm presented by Do-While Jones, but that
 * turned out to be broken.  This is based on the quat lib from UNC.
 */
void FLOATNAME(LQuaternion)::
set_from_matrix(const FLOATNAME(LMatrix3) &m) {
  FLOATTYPE m00, m01, m02, m10, m11, m12, m20, m21, m22;

  m00 = m(0, 0);
  m10 = m(1, 0);
  m20 = m(2, 0);
  m01 = m(0, 1);
  m11 = m(1, 1);
  m21 = m(2, 1);
  m02 = m(0, 2);
  m12 = m(1, 2);
  m22 = m(2, 2);

  FLOATTYPE trace = m00 + m11 + m22;

  if (trace > 0.0f) {
    // The easy case.
    FLOATTYPE S = csqrt(trace + 1.0f);
    _v(0) = S * 0.5f;
    S = 0.5f / S;
    _v(1) = (m12 - m21) * S;
    _v(2) = (m20 - m02) * S;
    _v(3) = (m01 - m10) * S;

  } else {
    // The harder case.  First, figure out which column to take as root.  This
    // will be the column with the largest value.

    // It is tempting to try to compare the absolute values of the diagonal
    // values in the code below, instead of their normal, signed values.
    // Don't do it.  We are actually maximizing the value of S, which must
    // always be positive, and is therefore based on the diagonal whose actual
    // value--not absolute value--is greater than those of the other two.

    // We already know that m00 + m11 + m22 <= 0 (because we are here in the
    // harder case).

    if (m00 > m11 && m00 > m22) {
      // m00 is larger than m11 and m22.
      FLOATTYPE S = 1.0f + m00 - (m11 + m22);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v(1) = S * 0.5f;
      S = 0.5f / S;
      _v(2) = (m01 + m10) * S;
      _v(3) = (m02 + m20) * S;
      _v(0) = (m12 - m21) * S;

    } else if (m11 > m22) {
      // m11 is larger than m00 and m22.
      FLOATTYPE S = 1.0f + m11 - (m22 + m00);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v(2) = S * 0.5f;
      S = 0.5f / S;
      _v(3) = (m12 + m21) * S;
      _v(1) = (m10 + m01) * S;
      _v(0) = (m20 - m02) * S;

    } else {
      // m22 is larger than m00 and m11.
      FLOATTYPE S = 1.0f + m22 - (m00 + m11);
      nassertv(S > 0.0f);
      S = csqrt(S);
      _v(3) = S * 0.5f;
      S = 0.5f / S;
      _v(1) = (m20 + m02) * S;
      _v(2) = (m21 + m12) * S;
      _v(0) = (m01 - m10) * S;
    }
  }
}

/**
 *
 */
void FLOATNAME(LQuaternion)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase4)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LQuaternion),
                  FLOATNAME(LVecBase4)::get_class_type());
  }
}
