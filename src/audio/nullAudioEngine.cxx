/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nullAudioEngine.cxx
 * @author brian
 * @date 2022-09-21
 */

#include "nullAudioEngine.h"
#include "nullAudioManager.h"

IMPLEMENT_CLASS(NullAudioEngine);

/**
 *
 */
PT(AudioManager) NullAudioEngine::
make_manager(const std::string &, AudioManager *) {
  return new NullAudioManager;
}

/**
 *
 */
void NullAudioEngine::
set_3d_listener_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) {
}

/**
 *
 */
LPoint3 NullAudioEngine::
get_3d_listener_pos() const {
  return LPoint3(0.0f);
}

/**
 *
 */
LQuaternion NullAudioEngine::
get_3d_listener_quat() const {
  return LQuaternion::ident_quat();
}

/**
 *
 */
LVector3 NullAudioEngine::
get_3d_listener_velocity() const {
  return LVector3(0.0f);
}

/**
 *
 */
void NullAudioEngine::
set_3d_unit_scale(PN_stdfloat scale) {
}

/**
 *
 */
PN_stdfloat NullAudioEngine::
get_3d_unit_scale() const {
  return 1.0f;
}

/**
 *
 */
bool NullAudioEngine::
initialize() {
  return true;
}

/**
 *
 */
void NullAudioEngine::
update() {
}
