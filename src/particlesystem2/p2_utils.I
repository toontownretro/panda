/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p2_utils.I
 * @author brian
 * @date 2022-04-08
 */

/**
 *
 */
INLINE PN_stdfloat
p2_exponential_decay(PN_stdfloat halflife, PN_stdfloat dt) {
  return std::exp((PN_stdfloat)-0.69314718f / halflife * dt);
}

/**
 *
 */
INLINE PN_stdfloat
p2_exponential_decay(PN_stdfloat decay_to, PN_stdfloat decay_time, PN_stdfloat dt) {
  return std::exp(std::log(decay_to) / decay_time * dt);
}

/**
 * Returns a float in [0, 1].
 */
INLINE PN_stdfloat
p2_normalized_rand() {
  return (PN_stdfloat)rand() / (PN_stdfloat)RAND_MAX;
}

/**
 * Lerps between min and max based on a random fraction.
 */
INLINE PN_stdfloat
p2_random_min_max(PN_stdfloat min, PN_stdfloat max) {
  PN_stdfloat frac = p2_normalized_rand();
  return min * (1.0f - frac) + max * frac;
}

/**
 * Returns a float in [min, min+range].
 */
INLINE PN_stdfloat
p2_random_min_range(PN_stdfloat min, PN_stdfloat range) {
  return min + range * p2_normalized_rand();
}

/**
 * Returns a float in [base-spread, base+spread].
 */
INLINE PN_stdfloat
p2_random_base_spread(PN_stdfloat base, PN_stdfloat spread) {
  return base + (spread - (p2_normalized_rand() * 2.0f * spread));
}

/**
 * generates a random unit vector
 */
INLINE LVector3
p2_random_unit_vector() {
  PN_stdfloat z, r, theta;

  z = 1.0f - (2.0f * p2_normalized_rand());
  r = csqrt(1.0f - (z * z));
  theta = 2.0f * MathNumbers::pi * p2_normalized_rand();

  return LVector3(r * ccos(theta), r * csin(theta), z);
}
