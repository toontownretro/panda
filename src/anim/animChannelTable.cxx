/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelTable.cxx
 * @author brian
 * @date 2021-08-04
 */

#include "animChannelTable.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "animLayer.h"
#include "animEvalContext.h"
#include "characterJoint.h"
#include "character.h"
#include "pbitops.h"
//#include <intrin.h>
#include "pStatCollector.h"

static PStatCollector frameblend_pcollector("*:Animation:FrameBlend");

IMPLEMENT_CLASS(AnimChannelTable);

/**
 * Converts a set of Euler angles in radians to a quaternion.
 * Assumes CS_zup_right.
 */
INLINE void
quat_from_hpr_radians(const LVecBase3 &hpr, LQuaternionf &quat) {
  // FIXME: Figure out how to optimize this.  We shouldn't have to
  // multiply 3 quaternions.

  float s, c;

  csincos(hpr[0] * 0.5f, &s, &c);
  LQuaternionf quat_h(c, 0, 0, s);

  csincos(hpr[1] * 0.5f, &s, &c);
  LQuaternionf quat_p(c, s, 0, 0);

  csincos(hpr[2] * 0.5f, &s, &c);
  LQuaternionf quat_r(c, 0, s, 0);

  quat = quat_r * quat_p * quat_h;
}

/**
 * Returns the index of the joint channel with the indicated name, or -1 if no
 * such joint channel exists.
 */
int AnimChannelTable::
find_joint_channel(const std::string &name) const {
  for (size_t i = 0; i < _joint_names.size(); i++) {
    if (_joint_names[i] == name) {
      return (int)i;
    }
  }

  return -1;
}

/**
 * Returns the index of the slider channel with the indicated name, or -1 if no
 * such slider channel exists.
 */
int AnimChannelTable::
find_slider_channel(const std::string &name) const {
  for (size_t i = 0; i < _num_slider_entries; i++) {
    if (_slider_entries[i].name == name) {
      return (int)i;
    }
  }

  return -1;
}

/**
 * Creates and returns a copy of this AnimChannel.
 */
PT(AnimChannel) AnimChannelTable::
make_copy() const {
  return new AnimChannelTable(*this);
}

/**
 * Returns the duration of the channel.
 */
PN_stdfloat AnimChannelTable::
get_length(Character *character) const {
  return _num_frames / _fps;
}

/**
 * Extracts from the animation table a pose for every joint at the indicated
 * animation frame.
 *
 * Don't use this to extract from frame 0, use extract_frame0_data() instead.
 */
void AnimChannelTable::
extract_frame_data(int frame, AnimEvalData &data, const AnimEvalContext &context, const vector_int &joint_map) const {
  const vector_float &fzero = _frames[0];
  const vector_float &fdata = _frames[frame];

  int zero_ofs = 0;
  int frame_ofs = 0;

  for (int i = 0; i < (int)_joint_formats.size(); ++i) {
    uint16_t format = _joint_formats[i];

    // Extract translation.
    LVecBase3f pos;
    pos[0] = (format & JF_x) ? fdata[frame_ofs++] : fzero[zero_ofs];
    pos[1] = (format & JF_y) ? fdata[frame_ofs++] : fzero[zero_ofs + 1];
    pos[2] = (format & JF_z) ? fdata[frame_ofs++] : fzero[zero_ofs + 2];

    // Extract rotation (euler angles in radians).
    LVecBase3f hpr;
    hpr[0] = (format & JF_h) ? fdata[frame_ofs++] : fzero[zero_ofs + 3];
    hpr[1] = (format & JF_p) ? fdata[frame_ofs++] : fzero[zero_ofs + 4];
    hpr[2] = (format & JF_r) ? fdata[frame_ofs++] : fzero[zero_ofs + 5];

    // Extract scale.
    LVecBase3f scale;
    scale[0] = (format & JF_i) ? fdata[frame_ofs++] : fzero[zero_ofs + 6];
    scale[1] = (format & JF_j) ? fdata[frame_ofs++] : fzero[zero_ofs + 7];
    scale[2] = (format & JF_k) ? fdata[frame_ofs++] : fzero[zero_ofs + 8];

    // Extract shear.
    LVecBase3f shear;
    shear[0] = (format & JF_a) ? fdata[frame_ofs++] : fzero[zero_ofs + 9];
    shear[1] = (format & JF_b) ? fdata[frame_ofs++] : fzero[zero_ofs + 10];
    shear[2] = (format & JF_c) ? fdata[frame_ofs++] : fzero[zero_ofs + 11];

    zero_ofs += 12;

    int cjoint = joint_map[i];
    if (cjoint == -1) {
      continue;
    }

    LQuaternionf quat;
    quat_from_hpr_radians(hpr, quat);

    int group = cjoint / SIMDFloatVector::num_columns;
    int sub = cjoint % SIMDFloatVector::num_columns;
    data._pose[group].pos.set_lvec(sub, pos);
    data._pose[group].scale.set_lvec(sub, scale);
    data._pose[group].shear.set_lvec(sub, shear);
    data._pose[group].quat.set_lquat(sub, quat);
  }
}

/**
 * Extracts a pose for every joint at the first frame of the animation.
 * Every joint is guaranteed to have a pose recorded at frame 0.
 */
void AnimChannelTable::
extract_frame0_data(AnimEvalData &data, const AnimEvalContext &context, const vector_int &joint_map) const {
  const vector_float &fzero = _frames[0];

  LVecBase3f pos, hpr, scale, shear;
  LQuaternionf quat;

  int itr = 0;
  for (int i = 0; i < (int)_joint_formats.size(); ++i) {
    pos[0] = fzero[itr++];
    pos[1] = fzero[itr++];
    pos[2] = fzero[itr++];
    hpr[0] = fzero[itr++];
    hpr[1] = fzero[itr++];
    hpr[2] = fzero[itr++];
    scale[0] = fzero[itr++];
    scale[1] = fzero[itr++];
    scale[2] = fzero[itr++];
    shear[0] = fzero[itr++];
    shear[1] = fzero[itr++];
    shear[2] = fzero[itr++];

    int cjoint = joint_map[i];
    if (cjoint == -1) {
      continue;
    }

    quat_from_hpr_radians(hpr, quat);

    int group = cjoint / SIMDFloatVector::num_columns;
    int sub = cjoint % SIMDFloatVector::num_columns;
    data._pose[group].pos.set_lvec(sub, pos);
    data._pose[group].scale.set_lvec(sub, scale);
    data._pose[group].shear.set_lvec(sub, shear);
    data._pose[group].quat.set_lquat(sub, quat);
  }
}

/**
 * Returns the offset into non-zero frames for the indicated transform
 * component of the indicated joint.
 */
int AnimChannelTable::
get_non0_joint_component_offset(int joint, JointFormat component) const {
  int offset = 0;

  for (int i = 0; i < joint; ++i) {
    offset += count_bits_in_word(_joint_formats[i]);
  }

  // Offset is now at the beginning of the requested joint.
  // Determine the additional offset to the requested component.

  uint16_t format = _joint_formats[joint];
  nassertr(format & component, offset);
  format &= ~flood_bits_up(component);

  offset += count_bits_in_word(format);

  return offset;
}

/**
 *
 */
void AnimChannelTable::
do_calc_pose(const AnimEvalContext &context, AnimEvalData &data) {
  auto it = context._character->_channel_bindings.find(this);
  if (it == context._character->_channel_bindings.end()) {
    // There's no mapping of character joints to anim joints on the character
    // for this channel.
    return;
  }

  const vector_int &joint_map = (*it).second._joint_map;

  // Every anim joint should appear in the mapping.
  nassertv(joint_map.size() >= _joint_formats.size());

  // Convert cycles to frame numbers for table lookup.

  // Ensure the cycle is within range.  The cycle can never be 1.0,
  // because the frame index is floor(cycle * num_frames).
  PN_stdfloat cycle = std::clamp(data._cycle, 0.0f, 0.999999f);

  int num_frames = get_num_frames();
  int start_frame = (int)floor(context._start_cycle * num_frames);
  int play_frames = (int)floor(context._play_cycles * num_frames);

  // Calculate the floating-point frame.
  PN_stdfloat fframe = cycle * num_frames;
  // Snap to integer frame.
  int frame = (int)floor(fframe);

  // Determine next frame for inter-frame blending.
  int next_frame;
  switch (context._play_mode) {
  case AnimLayer::PM_pose:
    next_frame = std::min(std::max(frame + 1, 0), num_frames);
    break;
  case AnimLayer::PM_play:
    next_frame = std::min(std::max(frame + 1, 0), play_frames) + start_frame;
    break;
  case AnimLayer::PM_loop:
    {
      if (play_frames == 0) {
        next_frame = std::min(std::max(frame + 1, 0), num_frames);
      } else {
        next_frame = cmod(frame + 1, play_frames + 1) + start_frame;
      }
    }
    break;
  case AnimLayer::PM_pingpong:
    {
      if (play_frames == 0) {
        next_frame = std::min(std::max(frame + 1, 0), num_frames);

      } else {
        next_frame = cmod(frame + 1, play_frames + 1) + start_frame;
        if (next_frame > play_frames) {
          next_frame = (play_frames * 2.0f - next_frame) + start_frame;
        } else {
          next_frame += start_frame;
        }
      }
    }
    break;
  default:
    next_frame = frame;
    break;
  }

  PN_stdfloat frac = fframe - frame;

  if (!context._frame_blend || frame == next_frame) {

    // Hold the current frame until the next one is ready.
    if (frame == 0) {
      extract_frame0_data(data, context, joint_map);
    } else {
      extract_frame_data(frame, data, context, joint_map);
    }

  } else {
    // Frame blending is enabled.  Need to blend between successive frames.

    AnimEvalData next_data;
    if (frame == 0) {
      extract_frame0_data(data, context, joint_map);
    } else {
      extract_frame_data(frame, data, context, joint_map);
    }
    if (next_frame == 0) {
      extract_frame0_data(next_data, context, joint_map);
    } else {
      extract_frame_data(next_frame, next_data, context, joint_map);
    }

    //frameblend_pcollector.start();

#if 1
    // Measured this to take 75 microseconds for 500 characters with 42
    // joints each.  ~6x faster than the scalar version below.  Using slerp()
    // with sleef trig functions takes much longer than basic lerp().

    SIMDFloatVector vfrac = frac;
    SIMDFloatVector ve0 = SIMDFloatVector(1.0f) - vfrac;

    for (int i = 0; i < context._num_joint_groups; ++i) {
      data._pose[i].pos *= ve0;
      data._pose[i].pos.madd_in_place(next_data._pose[i].pos, vfrac);

      data._pose[i].scale *= ve0;
      data._pose[i].scale.madd_in_place(next_data._pose[i].scale, vfrac);

      data._pose[i].shear *= ve0;
      data._pose[i].shear.madd_in_place(next_data._pose[i].shear, vfrac);

      data._pose[i].quat = data._pose[i].quat.align_lerp(next_data._pose[i].quat, vfrac);
    }

#else
    // Scalar version (though compiler optimizes it to use mulss/addss).
    // Measured this to take around 450 microseconds for 500 characters with
    // 42 joints each.
    float e0 = 1.0f - frac;
    for (int i = 0; i < context._num_joints; ++i) {
      int group = i / SIMDFloatVector::num_columns;
      int sub = i % SIMDFloatVector::num_columns;

      LVecBase3f pos = data._pose[group].pos.get_lvec(sub);
      pos *= e0;
      LVecBase3f scale = data._pose[group].scale.get_lvec(sub);
      scale *= e0;
      LVecBase3f shear = data._pose[group].shear.get_lvec(sub);
      shear *= e0;

      pos += next_data._pose[group].pos.get_lvec(sub) * frac;
      scale += next_data._pose[group].scale.get_lvec(sub) * frac;
      shear += next_data._pose[group].shear.get_lvec(sub) * frac;

      LQuaternionf quat;
      LQuaternionf::blend(data._pose[group].quat.get_lquat(sub), next_data._pose[group].quat.get_lquat(sub), frac, quat);

      data._pose[group].pos.set_lvec(sub, pos);
      data._pose[group].scale.set_lvec(sub, scale);
      data._pose[group].shear.set_lvec(sub, shear);
      data._pose[group].quat.set_lquat(sub, quat);
    }
#endif

    //frameblend_pcollector.stop();
  }
}

/**
 *
 */
LVector3 AnimChannelTable::
get_root_motion_vector(Character *character) const {
  return _root_motion_vector;
}

/**
 * Extracts the delta of the indicated transform component of the indicated
 * joint between the first and last frame of the animation, and removes
 * the transform component from the animation table for the joint.
 */
float AnimChannelTable::
extract_component_delta(int joint, JointFormat component) {
  nassertr(_joint_formats[joint] & component, 0.0f);

  int ofs = get_non0_joint_component_offset(joint, component);
  int zero_ofs = get_highest_on_bit(component);
  int last_frame = (int)_frames.size() - 1;

  float delta = _frames[last_frame][ofs] - _frames[0][joint * 12 + zero_ofs];

  // Zero out initial data.
  _frames[0][joint * 12 + zero_ofs] = 0.0f;

  // Now remove from non-zero frames.
  for (size_t i = 1; i < _frames.size(); ++i) {
    _frames[i].erase(_frames[i].begin() + ofs);
  }

  // This component is no longer animating.
  _joint_formats[joint] &= ~component;

  return delta;
}

/**
 * Applies an offset to a transform component of a joint across all animation
 * frames.
 */
void AnimChannelTable::
offset_joint_component(int joint, JointFormat component, float offset) {
  _frames[0][joint * 12 + get_highest_on_bit(component)] += offset;

  if (_joint_formats[joint] & component) {
    int ofs = get_non0_joint_component_offset(joint, component);
    for (size_t i = 1; i < _frames.size(); ++i) {
      _frames[i][ofs] += offset;
    }
  }
}

/**
 * Extracts the motion of the indicated translation components of the root
 * joint.  The specified translation components are stripped from the animation
 * of the root joint.
 */
void AnimChannelTable::
calc_root_motion(unsigned int flags, int root_joint) {
  // We currently do one motion keyframe along the entire animation for
  // translation only.  Assumes that the root joint moves on a completely
  // straight and contiguous line for the entire animation.

  // Get frame data for root joint.
  nassertv(root_joint >= 0 && root_joint < (int)_joint_formats.size());

  uint16_t format = _joint_formats[root_joint];

  // Forget about transform components that are static on the root joint,
  // we know the motion vector will be 0 for those.
  if (!(format & JF_x)) {
    flags &= ~MF_linear_x;
  }
  if (!(format & JF_y)) {
    flags &= ~MF_linear_y;
  }
  if (!(format & JF_z)) {
    flags &= ~MF_linear_z;
  }

  // Extract translation vector for requested axes between start and end frame.
  LVector3 translation_vector(0.0f, 0.0f, 0.0f);
  if (flags & MF_linear_x) {
    translation_vector[0] = extract_component_delta(root_joint, JF_x);
  }
  if (flags & MF_linear_y) {
    translation_vector[1] = extract_component_delta(root_joint, JF_y);
  }
  if (flags & MF_linear_z) {
    translation_vector[2] = extract_component_delta(root_joint, JF_z);
  }

  _root_motion_vector = translation_vector;
}

/**
 *
 */
void AnimChannelTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *AnimChannelTable::
make_from_bam(const FactoryParams &params) {
  AnimChannelTable *table = new AnimChannelTable;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  table->fillin(scan, manager);
  return table;
}

/**
 *
 */
void AnimChannelTable::
write_datagram(BamWriter *manager, Datagram &me) {
  AnimChannel::write_datagram(manager, me);

  me.add_uint8(_joint_formats.size());
  for (size_t i = 0; i < _joint_formats.size(); ++i) {
    me.add_string(_joint_names[i]);
    me.add_uint16(_joint_formats[i]);
  }

  me.add_uint16(_frames.size());
  for (const vector_float &frame : _frames) {
    for (const float &data : frame) {
      me.add_float32(data);
    }
  }

  _root_motion_vector.write_datagram(me);
}

/**
 *
 */
void AnimChannelTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel::fillin(scan, manager);

  unsigned int num_joints = scan.get_uint8();
  _joint_formats.resize(num_joints);
  _joint_names.resize(num_joints);
  for (unsigned int i = 0; i < num_joints; ++i) {
    _joint_names[i] = scan.get_string();
    _joint_formats[i] = scan.get_uint16();
  }

  _frames.resize(scan.get_uint16());

  // Fill frame 0.  Every joint has an entry in every component.
  for (unsigned int i = 0; i < num_joints * 12; ++i) {
    _frames[0].push_back(scan.get_float32());
  }

  // Every non-zero frame stores the exact same number of floats, the sum
  // of the number of floats stored for each joint.
  int num_floats = 0;
  for (unsigned int i = 0; i < num_joints; ++i) {
    uint16_t format = _joint_formats[i];
    // Each bit turned on in the format is a transform component that gets
    // stored.
    num_floats += count_bits_in_word(format);
  }

  for (size_t i = 1; i < _frames.size(); ++i) {
    for (unsigned int j = 0; j < num_floats; ++j) {
      _frames[i].push_back(scan.get_float32());
    }
  }

  _root_motion_vector.read_datagram(scan);
}
