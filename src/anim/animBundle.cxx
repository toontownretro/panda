/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundle.cxx
 * @author drose
 * @date 1999-02-21
 */

#include "animBundle.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle AnimBundle::_type_handle;

/**
 * Creates a new AnimBundle, just like this one, without copying any children.
 * The new copy is added to the indicated parent.  Intended to be called by
 * make_copy() only.
 */
AnimBundle::
AnimBundle(const AnimBundle &copy) :
  AnimGraphNode(copy),
  _fps(copy._fps),
  _num_frames(copy._num_frames),
  _joint_frames(copy._joint_frames),
  _joint_entries(copy._joint_entries),
  _slider_table(copy._slider_table),
  _slider_entries(copy._slider_entries),
  _joint_map(copy._joint_map),
  _has_character_bound(copy._has_character_bound)
{
}

/**
 * Returns a full copy of the bundle and its entire tree of nested AnimGroups.
 * However, the actual data stored in the leaves--that is, animation tables,
 * such as those stored in an AnimChannelMatrixXfmTable--will be shared.
 */
PT(AnimBundle) AnimBundle::
copy_bundle() const {
  //PT(AnimGroup) group = copy_subtree(nullptr);
  //return DCAST(AnimBundle, group.p());
  return nullptr; // FIXME
}

/**
 * Returns the index of the joint channel with the indicated name, or -1 if no
 * such joint channel exists.
 */
int AnimBundle::
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
int AnimBundle::
find_slider_channel(const std::string &name) const {
  for (size_t i = 0; i < _slider_entries.size(); i++) {
    if (_slider_entries[i].name == name) {
      return (int)i;
    }
  }

  return -1;
}

/**
 * Writes a one-line description of the bundle.
 */
void AnimBundle::
output(std::ostream &out) const {
  out << get_type() << " " << get_name() << ", " << get_num_frames()
      << " frames at " << get_base_frame_rate() << " fps";
}

/**
 *
 */
void AnimBundle::
evaluate(AnimGraphEvalContext &context) {
  if (!has_mapped_character()) {
    return;
  }

  // Make sure cycle is within 0-1 range.
  PN_stdfloat cycle = std::max(0.0f, std::min(0.999f, context._cycle));
  int num_frames = get_num_frames();
  // Calculate the floating-point frame.
  PN_stdfloat fframe = cycle * num_frames;
  // Snap to integer frame.
  int frame = (int)floor(fframe);
  int next_frame;
  if (context._looping) {
    next_frame = cmod(frame + 1, num_frames);
  } else {
    next_frame = std::max(0, std::min(num_frames - 1, frame + 1));
  }

  PN_stdfloat frac = fframe - frame;

  if (!context._frame_blend || frame == next_frame) {
    // Hold the current frame until the next one is ready.
    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &xform = context._joints[i];
      CharacterJoint &joint = context._parts[i];
      int anim_joint = get_anim_joint_for_character_joint(i);
      if (anim_joint == -1) {
        continue;
      }
      const JointFrame &jframe = get_joint_frame(anim_joint, frame);

      xform._rotation = jframe.quat;
      xform._position = jframe.pos;
      xform._scale = jframe.scale;
    }

  } else {
    // Frame blending is enabled.  Need to blend between successive frames.

    PN_stdfloat e0 = 1.0f - frac;

    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &t = context._joints[i];
      CharacterJoint &j = context._parts[i];
      int anim_joint = get_anim_joint_for_character_joint(i);
      if (anim_joint == -1) {
        continue;
      }

      const JointEntry &je = get_joint_entry(anim_joint);
      const JointFrame &jf = get_joint_frame(je, frame);
      const JointFrame &jf_next = get_joint_frame(je, next_frame);

      t._position = (jf.pos * e0) + (jf_next.pos * frac);
      t._scale = (jf.scale * e0) + (jf_next.scale * frac);
      LQuaternion::blend(jf.quat, jf_next.quat, frac, t._rotation);
    }
  }
}

/**
 *
 */
void AnimBundle::
evaluate_anims(pvector<AnimBundle *> &anims, vector_stdfloat &weights, PN_stdfloat this_weight) {
  anims.push_back(this);
  weights.push_back(this_weight);
}

/**
 * Returns a copy of this object, and attaches it to the indicated parent
 * (which may be NULL only if this is an AnimBundle).  Intended to be called
 * by copy_subtree() only.
 */
//AnimGroup *AnimBundle::
//make_copy(AnimGroup *parent) const {
//  return new AnimBundle(parent, *this);
//}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void AnimBundle::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());

  me.add_stdfloat(_fps);
  me.add_uint16(_num_frames);

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

}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimBundle::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  _fps = scan.get_stdfloat();
  _num_frames = scan.get_uint16();

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
}

/**
 * Factory method to generate a AnimBundle object
 */
TypedWritable *AnimBundle::
make_AnimBundle(const FactoryParams &params) {
  AnimBundle *me = new AnimBundle;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a AnimBundle object
 */
void AnimBundle::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimBundle);
}
