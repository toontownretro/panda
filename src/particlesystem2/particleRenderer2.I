/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleRenderer2.I
 * @author brian
 * @date 2022-04-04
 */

/**
 *
 */
INLINE NodePath SpriteParticleRenderer2::
get_geom_np() const {
  return _geom_np;
}

/**
 *
 */
INLINE GeomNode *SpriteParticleRenderer2::
get_geom_node() const {
  return _geom_node;
}

/**
 *
 */
INLINE const RenderState *SpriteParticleRenderer2::
get_render_state() const {
  return _render_state;
}
