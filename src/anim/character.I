/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.I
 * @author lachbr
 * @date 2021-02-22
 */

/**
 * Returns the AnimPreloadTable associated with the Character.  This table,
 * if present, can be used for the benefit of load_bind_anim() to allow
 * asynchronous binding.
 */
INLINE CPT(AnimPreloadTable) Character::
get_anim_preload() const {
  return _anim_preload.get_read_pointer();
}

/**
 * Returns a modifiable pointer to the AnimPreloadTable associated with the
 * Character, if any.
 */
INLINE PT(AnimPreloadTable) Character::
modify_anim_preload() {
  return _anim_preload.get_write_pointer();
}

/**
 * Replaces the AnimPreloadTable associated with the Character.
 */
INLINE void Character::
set_anim_preload(AnimPreloadTable *anim_preload) {
  _anim_preload = anim_preload;
}

/**
 * Removes any AnimPreloadTable associated with the Character.
 */
INLINE void Character::
clear_anim_preload() {
  _anim_preload = nullptr;
}

/**
 * Sets the animation graph that the character should evaluate to determine
 * a final pose for a frame of animation.
 */
INLINE void Character::
set_anim_graph(AnimGraphNode *graph) {
  CDWriter cdata(_cycler);
  if (cdata->_anim_graph != graph) {
    cdata->_anim_graph = graph;
    cdata->_anim_changed = true;
  }
}

/**
 * Returns the animation graph that the character should evaluate to determine
 * a final pose for a frame of animation.
 */
INLINE AnimGraphNode *Character::
get_anim_graph() const {
  CDReader cdata(_cycler);
  return cdata->_anim_graph;
}

/**
 * Specifies whether the character interpolates (blends) between two
 * sequential frames of an active animation, showing a smooth intra-frame
 * motion, or whether it holds each frame until the next frame is ready,
 * showing precisely the specified animation.
 *
 * When this value is false, the character holds each frame until the next is
 * ready.  When this is true, the character will interpolate between two
 * consecutive frames of animation for each frame the animation is onscreen,
 * according to the amount of time elapsed between the frames.
 *
 * The default value of this flag is determined by the interpolate-frames
 * Config.prc variable.
 *
 * Use set_blend_type() to change the algorithm that the character uses to
 * interpolate matrix positions.
 */
INLINE void Character::
set_frame_blend_flag(bool frame_blend_flag) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_frame_blend_flag = frame_blend_flag;
}

/**
 * Returns whether the character interpolates (blends) between two sequential
 * animation frames, or whether it holds the current frame until the next one
 * is ready.  See set_frame_blend_flag().
 */
INLINE bool Character::
get_frame_blend_flag() const {
  CDReader cdata(_cycler);
  return cdata->_frame_blend_flag;
}

/**
 * Specifies the transform matrix which is implicitly applied at the root of
 * the animated hierarchy.
 */
INLINE void Character::
set_root_xform(const LMatrix4 &root_xform) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_root_xform = root_xform;
  cdata->_anim_changed = true;
}

/**
 * Returns the transform matrix which is implicitly applied at the root of the
 * animated hierarchy.
 */
INLINE const LMatrix4 &Character::
get_root_xform() const {
  CDReader cdata(_cycler);
  return cdata->_root_xform;
}

/**
 * Returns the number of joints in the Character.
 */
INLINE int Character::
get_num_joints() const {
  return (int)_joints.size();
}

/**
 * Returns the nth joint of the Character.
 */
INLINE CharacterJoint *Character::
get_joint(int n) {
  nassertr(n >= 0 && n < (int)_joints.size(), nullptr);

  return &_joints[n];
}

/**
 * Returns the number of sliders in the Character.
 */
INLINE int Character::
get_num_sliders() const {
  return (int)_sliders.size();
}

/**
 * Returns the nth slider of the Character.
 */
INLINE CharacterSlider *Character::
get_slider(int n) {
  nassertr(n >= 0 && n < (int)_sliders.size(), nullptr);

  return &_sliders[n];
}

/**
 * Specifies the minimum amount of time, in seconds, that should elapse
 * between any two consecutive updates.  This is normally used by
 * Character::set_lod_animation(), and should not be called directly.
 */
INLINE void Character::
set_update_delay(double delay) {
  _update_delay = delay;
}
