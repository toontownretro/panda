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

/**
 *
 */
INLINE bool SpriteParticleRenderer2::
get_fit_animations_to_particle_lifespan() const {
  return _fit_anim_to_particle_lifespan;
}

/**
 *
 */
INLINE PN_stdfloat SpriteParticleRenderer2::
get_animation_play_rate() const {
  return _anim_play_rate;
}
