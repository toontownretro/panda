/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decalProjector.I
 * @author brian
 * @date 2022-11-05
 */

/**
 *
 */
INLINE DecalProjector::
DecalProjector() :
  _projector_mins(-1.0f),
  _projector_maxs(1.0f),
  _projector_transform(TransformState::make_identity()),
  _decal_state(RenderState::make_empty()),
  _decal_uv_transform(TransformState::make_identity())
{
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_parent(const NodePath &parent) {
  _projector_parent = parent;
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_transform(const TransformState *transform) {
  _projector_transform = transform;
}

/**
 *
 */
INLINE void DecalProjector::
set_projector_bounds(const LPoint3 &mins, const LPoint3 &maxs) {
  _projector_mins = mins;
  _projector_maxs = maxs;
}

/**
 *
 */
INLINE void DecalProjector::
set_decal_parent(const NodePath &parent) {
  _decal_parent = parent;
}

/**
 *
 */
INLINE void DecalProjector::
set_decal_uv_transform(const TransformState *transform) {
  _decal_uv_transform = transform;
}

/**
 *
 */
INLINE void DecalProjector::
set_decal_render_state(const RenderState *state) {
  _decal_state = state;
}
