/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterial.I
 * @author lachbr
 * @date 2021-01-11
 */

/**
 * Returns the Material referenced by this EggMaterial.
 */
INLINE Material *EggMaterial::
get_material() const {
  return _material;
}

/**
 *
 */
INLINE UniqueEggMaterials::
UniqueEggMaterials(int eq) : _eq(eq) {
}

/**
 *
 */
INLINE bool UniqueEggMaterials::
operator ()(const EggMaterial *t1, const EggMaterial *t2) const {
  return t1->sorts_less_than(*t2, _eq);
}
