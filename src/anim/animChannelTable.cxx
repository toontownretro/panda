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

IMPLEMENT_CLASS(JointEntry);
IMPLEMENT_CLASS(JointFrame);
IMPLEMENT_CLASS(SliderEntry);

template class PointerToBase<ReferenceCountedVector<JointEntry> >;
template class PointerToArrayBase<JointEntry>;
template class PointerToArray<JointEntry>;
template class ConstPointerToArray<JointEntry>;

template class PointerToBase<ReferenceCountedVector<JointFrame> >;
template class PointerToArrayBase<JointFrame>;
template class PointerToArray<JointFrame>;
template class ConstPointerToArray<JointFrame>;

template class PointerToBase<ReferenceCountedVector<SliderEntry> >;
template class PointerToArrayBase<SliderEntry>;
template class PointerToArray<SliderEntry>;
template class ConstPointerToArray<SliderEntry>;

IMPLEMENT_CLASS(AnimChannelTable);

/**
 * Returns the index of the joint channel with the indicated name, or -1 if no
 * such joint channel exists.
 */
int AnimChannelTable::
find_joint_channel(const std::string &name) const {
  for (size_t i = 0; i < _joint_entries.size(); i++) {
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
  for (size_t i = 0; i < _slider_entries.size(); i++) {
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
  if (!has_mapped_character()) {
    return;
  }

  // Make sure cycle is within 0-1 range.
  PN_stdfloat cycle = std::max(0.0f, std::min(1.0f, data._cycle));

  int num_frames = get_num_frames();
  int start_frame = (int)floor(context._start_cycle * (num_frames - 1));
  int play_frames = (int)floor(context._play_cycles * (num_frames - 1));

  // Calculate the floating-point frame.
  PN_stdfloat fframe = cycle * (num_frames - 1);
  // Snap to integer frame.
  int frame = (int)floor(fframe);

  // Determine next frame for inter-frame blending.
  int next_frame;
  switch (context._play_mode) {
  case AnimLayer::PM_pose:
    next_frame = std::min(std::max(frame + 1, 0), num_frames - 1);
    break;
  case AnimLayer::PM_play:
    next_frame = std::min(std::max(frame + 1, 0), play_frames) + start_frame;
    break;
  case AnimLayer::PM_loop:
    {
      if (play_frames == 0) {
        next_frame = std::min(std::max(frame + 1, 0), num_frames - 1);
      } else {
        next_frame = cmod(frame + 1, play_frames) + start_frame;
      }
    }
    break;
  case AnimLayer::PM_pingpong:
    {
      if (play_frames == 0) {
        next_frame = std::min(std::max(frame + 1, 0), num_frames - 1);

      } else {
        next_frame = cmod(frame + 1, play_frames) + start_frame;
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
    for (int i = 0; i < context._num_joints; i++) {
      if (!context._joint_mask.get_bit(i)) {
        continue;
      }
      CharacterJoint &joint = context._joints[i];
      int anim_joint = get_anim_joint_for_character_joint(i);
      if (anim_joint == -1) {
        continue;
      }
      const JointFrame &jframe = get_joint_frame(anim_joint, frame);

      data._rotation[i] = jframe.quat;
      data._position[i] = jframe.pos;
      data._scale[i] = jframe.scale;
    }

  } else {
    // Frame blending is enabled.  Need to blend between successive frames.

    PN_stdfloat e0 = 1.0f - frac;

    for (int i = 0; i < context._num_joints; i++) {
      if (!context._joint_mask.get_bit(i)) {
        continue;
      }
      CharacterJoint &j = context._joints[i];
      int anim_joint = get_anim_joint_for_character_joint(i);
      if (anim_joint == -1) {
        continue;
      }

      const JointEntry &je = get_joint_entry(anim_joint);
      const JointFrame &jf = get_joint_frame(je, frame);
      const JointFrame &jf_next = get_joint_frame(je, next_frame);

      data._position[i] = (jf.pos * e0) + (jf_next.pos * frac);
      data._scale[i] = (jf.scale * e0) + (jf_next.scale * frac);
      LQuaternion::blend(jf.quat, jf_next.quat, frac, data._rotation[i]);
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

  me.add_uint16(_joint_entries.size());
  for (size_t i = 0; i < _joint_entries.size(); i++) {
    const JointEntry &entry = _joint_entries[i];

    me.add_string(entry.name);

    me.add_int16(entry.first_frame);
    me.add_int16(entry.num_frames);
  }

  me.add_uint16(_joint_frames.size());
  for (size_t i = 0; i < _joint_frames.size(); i++) {
    _joint_frames[i].pos.write_datagram(me);
    _joint_frames[i].quat.write_datagram(me);
    _joint_frames[i].scale.write_datagram(me);
  }

  me.add_uint16(_slider_entries.size());
  for (size_t i = 0; i < _slider_entries.size(); i++) {
    const SliderEntry &entry = _slider_entries[i];

    me.add_string(entry.name);

    me.add_int16(entry.first_frame);
    me.add_int16(entry.num_frames);
  }

  me.add_uint16(_slider_table.size());
  for (size_t i = 0; i < _slider_table.size(); i++) {
    me.add_stdfloat(_slider_table[i]);
  }

  me.add_uint16(_joint_map.size());
  for (size_t i = 0; i < _joint_map.size(); i++) {
    me.add_int16(_joint_map[i]);
  }

  me.add_uint16(_slider_map.size());
  for (size_t i = 0; i < _slider_map.size(); i++) {
    me.add_int16(_slider_map[i]);
  }

  me.add_bool(_has_character_bound);
}

/**
 *
 */
void AnimChannelTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel::fillin(scan, manager);

  _joint_entries.resize(scan.get_uint16());
  for (size_t i = 0; i < _joint_entries.size(); i++) {
    _joint_entries[i].name = scan.get_string();
    _joint_entries[i].first_frame = scan.get_int16();
    _joint_entries[i].num_frames = scan.get_int16();
  }

  _joint_frames.resize(scan.get_uint16());
  for (size_t i = 0; i < _joint_frames.size(); i++) {
    _joint_frames[i].pos.read_datagram(scan);
    _joint_frames[i].quat.read_datagram(scan);
    _joint_frames[i].scale.read_datagram(scan);
  }

  _slider_entries.resize(scan.get_uint16());
  for (size_t i = 0; i < _slider_entries.size(); i++) {
    _slider_entries[i].name = scan.get_string();
    _slider_entries[i].first_frame = scan.get_int16();
    _slider_entries[i].num_frames = scan.get_int16();
  }

  _slider_table.resize(scan.get_uint16());
  for (size_t i = 0; i < _slider_table.size(); i++) {
    _slider_table[i] = scan.get_stdfloat();
  }

  _joint_map.resize(scan.get_uint16());
  for (size_t i = 0; i < _joint_map.size(); i++) {
    _joint_map[i] = scan.get_int16();
  }

  _slider_map.resize(scan.get_uint16());
  for (size_t i = 0; i < _slider_map.size(); i++) {
    _slider_map[i] = scan.get_int16();
  }

  _has_character_bound = scan.get_bool();
}
