/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p2_utils.h
 * @author brian
 * @date 2022-04-08
 */

#ifndef P2_UTILS_H
#define P2_UTILS_H

#include "pandabase.h"
#include "mathNumbers.h"
#include "numeric_types.h"
#include "luse.h"

INLINE PN_stdfloat p2_exponential_decay(PN_stdfloat halflife, PN_stdfloat dt);
INLINE PN_stdfloat p2_exponential_decay(PN_stdfloat decay_to, PN_stdfloat decay_time,
                                        PN_stdfloat dt);

/**
 * Returns a float in [0, 1]
 */
INLINE PN_stdfloat p2_normalized_rand();
INLINE PN_stdfloat p2_normalized_rand(PN_stdfloat bias);

/**
 * Returns a float in [min, max]
 */
INLINE PN_stdfloat p2_random_min_max(PN_stdfloat min, PN_stdfloat max);
INLINE PN_stdfloat p2_random_min_max(PN_stdfloat min, PN_stdfloat max, PN_stdfloat bias);

/**
 * Returns a float in [min, min+range]
 */
INLINE PN_stdfloat p2_random_min_range(PN_stdfloat min, PN_stdfloat range);
INLINE PN_stdfloat p2_random_min_range(PN_stdfloat min, PN_stdfloat range, PN_stdfloat bias);
/**
 * Returns a float in [base-spread, base+spread]
 */
INLINE PN_stdfloat p2_random_base_spread(PN_stdfloat base, PN_stdfloat spread);
INLINE PN_stdfloat p2_random_base_spread(PN_stdfloat base, PN_stdfloat spread, PN_stdfloat bias);

/**
 * generates a random unit vector
 */
INLINE LVector3 p2_random_unit_vector();

#include "p2_utils.I"

#endif // P2_UTILS_H
