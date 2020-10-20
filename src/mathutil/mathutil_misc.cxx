/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_misc.cxx
 * @author lachbr
 * @date 2020-10-19
 *
 * Miscellaneous math utility functions.
 */

#include "mathutil_misc.h"

// solves for "a, b, c" where "a x^2 + b x + c = y", return true if solution exists
bool
solve_inverse_quadratic(float x1, float y1, float x2, float y2, float x3,
                        float y3, float &a, float &b, float &c) {
  float det = (x1 - x2)*(x1 - x3)*(x2 - x3);

  // FIXME: check with some sort of epsilon
  if ( det == 0.0 )
    return false;

  a = ( x3*( -y1 + y2 ) + x2 * ( y1 - y3 ) + x1 * ( -y2 + y3 ) ) / det;

  b = ( x3*x3*( y1 - y2 ) + x1 * x1*( y2 - y3 ) + x2 * x2*( -y1 + y3 ) ) / det;

  c = ( x1*x3*( -x1 + x3 )*y2 + x2 * x2*( x3*y1 - x1 * y3 ) + x2 * ( -( x3*x3*y1 ) + x1 * x1*y3 ) ) / det;

  return true;
}

bool solve_inverse_quadratic_monotonic(
  float x1, float y1, float x2, float y2, float x3, float y3,
  float &a, float &b, float &c) {
  // use SolveInverseQuadratic, but if the sigm of the derivative at the start point is the wrong
  // sign, displace the mid point

  // first, sort parameters
  if ( x1 > x2 ) {
    swap_floats( x1, x2 );
    swap_floats( y1, y2 );
  }
  if ( x2 > x3 ) {
    swap_floats( x2, x3 );
    swap_floats( y2, y3 );
  }
  if ( x1 > x2 ) {
    swap_floats( x1, x2 );
    swap_floats( y1, y2 );
  }
  // this code is not fast. what it does is when the curve would be non-monotonic, slowly shifts
  // the center point closer to the linear line between the endpoints. Should anyone need htis
  // function to be actually fast, it would be fairly easy to change it to be so.
  for ( float blend_to_linear_factor = 0.0; blend_to_linear_factor <= 1.0; blend_to_linear_factor += 0.05 ) {
    float tempy2 = ( 1 - blend_to_linear_factor )*y2 + blend_to_linear_factor * flerp( y1, y3, x1, x3, x2 );
    if ( !solve_inverse_quadratic( x1, y1, x2, tempy2, x3, y3, a, b, c ) )
      return false;
    float derivative = 2.0*a + b;
    if ( ( y1 < y2 ) && ( y2 < y3 ) )							// monotonically increasing
    {
      if ( derivative >= 0.0 )
        return true;
    }
    else
    {
      if ( ( y1 > y2 ) && ( y2 > y3 ) )							// monotonically decreasing
      {
        if ( derivative <= 0.0 )
          return true;
      }
      else
        return true;
    }
  }
  return true;
}
