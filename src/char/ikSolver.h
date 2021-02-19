/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikSolver.h
 * @author lachbr
 * @date 2021-02-12
 */

#ifndef IKSOLVER_H
#define IKSOLVER_H

#include "pandabase.h"
#include "luse.h"

/**
 * Two-link inverse kinematics solver.
 *
 * Given a two link joint from [0,0,0] to end effector position P,
 * let link lengths be a and b, and let norm |P| = c.  Clearly a+b <= c.
 *
 * Problem: find a "knee" position Q such that |Q| = a and |P-Q| = b.
 *
 * In the case of a point on the x axis R = [c,0,0], there is a
 * closed form solution S = [d,e,0], where |S| = a and |R-S| = b:
 *
 *    d2+e2 = a2                  -- because |S| = a
 *    (c-d)2+e2 = b2              -- because |R-S| = b
 *
 *    c2-2cd+d2+e2 = b2           -- combine the two equations
 *    c2-2cd = b2 - a2
 *    c-2d = (b2-a2)/c
 *    d - c/2 = (a2-b2)/c / 2
 *
 *    d = (c + (a2-b2/c) / 2      -- to solve for d and e.
 *    e = sqrt(a2-d2)
 */
class EXPCL_PANDA_CHAR IKSolver {
PUBLISHED:
  bool solve(PN_stdfloat a, PN_stdfloat b, const LPoint3 &p, const LPoint3 &d, LPoint3 &q);

  void define_m(const LPoint3 &p, const LPoint3 &d);

  float find_d(PN_stdfloat a, PN_stdfloat b, PN_stdfloat c);
  float find_e(PN_stdfloat a, PN_stdfloat d);

private:
  LMatrix3 _forward;
  LMatrix3 _inverse;
};

#endif // IKSOLVER_H
