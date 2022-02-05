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

IMPLEMENT_CLASS(AnimChannelTable);

/**
 * Returns the index of the joint channel with the indicated name, or -1 if no
 * such joint channel exists.
 */
int AnimChannelTable::
find_joint_channel(const std::string &name) const {
  for (size_t i = 0; i < _num_joint_entries; i++) {
    if (_joint_entries[i].name == name) {
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

  // Every joint should have an entry in the mapping, even if it's
  // to no anim joint.
  //nassertv(context._num_joints == (int)joint_map.size());

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

  if (!context._frame_blend || frame == next_frame || frac == 0.0f) {
    // Hold the current frame until the next one is ready.
    for (int i = 0; i < context._num_joints; i++) {
      if (!CheckBit(context._joint_mask, i)) {
        continue;
      }

      int anim_joint = joint_map[i];
      if (anim_joint < 0) {
        // Invalid anim joint.
        continue;
      }

      const JointFrame &jframe = get_joint_frame(anim_joint, frame);

      AnimEvalData::Joint &pose = data._pose[i];
      pose._position = jframe.pos;
      pose._scale = jframe.scale;
      pose._shear = jframe.shear;
      pose._rotation = jframe.quat;
    }

  } else {
    // Frame blending is enabled.  Need to blend between successive frames.

    PN_stdfloat e0 = 1.0f - frac;

    for (int i = 0; i < context._num_joints; i++) {
      if (!CheckBit(context._joint_mask, i)) {
        continue;
      }

      int anim_joint = joint_map[i];
      if (anim_joint < 0) {
        // Invalid anim joint.
        continue;
      }

      const JointEntry &je = get_joint_entry(anim_joint);
      const JointFrame &jf = get_joint_frame(je, frame);
      const JointFrame &jf_next = get_joint_frame(je, next_frame);

      AnimEvalData::Joint &pose = data._pose[i];

      pose._position = (jf.pos * e0) + (jf_next.pos * frac);
      pose._scale = (jf.scale * e0) + (jf_next.scale * frac);
      pose._shear = (jf.shear * e0) + (jf_next.shear * frac);
      LQuaternion::blend(jf.quat, jf_next.quat, frac, pose._rotation);
    }
  }
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

  me.add_uint8(_num_joint_entries);
  for (size_t i = 0; i < _num_joint_entries; i++) {
    const JointEntry &entry = _joint_entries[i];

    me.add_string(entry.name);

    me.add_int16(entry.first_frame);
    me.add_int16(entry.num_frames);
  }

  me.add_uint16(_joint_frames.size());
  for (size_t i = 0; i < _joint_frames.size(); i++) {
    _joint_frames[i].pos.get_xyz().write_datagram(me);
    _joint_frames[i].quat.write_datagram(me);
    _joint_frames[i].scale.get_xyz().write_datagram(me);
    _joint_frames[i].shear.get_xyz().write_datagram(me);
  }

  me.add_uint8(_num_slider_entries);
  for (size_t i = 0; i < _num_slider_entries; i++) {
    const SliderEntry &entry = _slider_entries[i];

    me.add_string(entry.name);

    me.add_int16(entry.first_frame);
    me.add_int16(entry.num_frames);
  }

  me.add_uint16(_slider_table.size());
  for (size_t i = 0; i < _slider_table.size(); i++) {
    me.add_stdfloat(_slider_table[i]);
  }
}

/**
 *
 */
void AnimChannelTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel::fillin(scan, manager);

  _num_joint_entries = scan.get_uint8();
  for (size_t i = 0; i < _num_joint_entries; i++) {
    _joint_entries[i].name = scan.get_string();
    _joint_entries[i].first_frame = scan.get_int16();
    _joint_entries[i].num_frames = scan.get_int16();
  }

  _joint_frames.resize(scan.get_uint16());
  for (size_t i = 0; i < _joint_frames.size(); i++) {
    LVecBase3 vec3;
    vec3.read_datagram(scan);
    _joint_frames[i].pos.set(vec3[0], vec3[1], vec3[2], 0.0f);
    _joint_frames[i].quat.read_datagram(scan);
    vec3.read_datagram(scan);
    _joint_frames[i].scale.set(vec3[0], vec3[1], vec3[2], 0.0f);
    vec3.read_datagram(scan);
    _joint_frames[i].shear.set(vec3[0], vec3[1], vec3[2], 0.0f);
  }

  _num_slider_entries = scan.get_uint8();
  for (size_t i = 0; i < _num_slider_entries; i++) {
    _slider_entries[i].name = scan.get_string();
    _slider_entries[i].first_frame = scan.get_int16();
    _slider_entries[i].num_frames = scan.get_int16();
  }

  _slider_table.resize(scan.get_uint16());
  for (size_t i = 0; i < _slider_table.size(); i++) {
    _slider_table[i] = scan.get_stdfloat();
  }
}
