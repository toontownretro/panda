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
  TypedWritableReferenceCount(copy),
  Namable(copy),
  _fps(copy._fps),
  _num_frames(copy._num_frames),
  _joint_frames(copy._joint_frames),
  _joint_entries(copy._joint_entries),
  _slider_table(copy._slider_table),
  _slider_entries(copy._slider_entries)
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

  /*
  me.add_uint16(_joint_entries.size());
  for (size_t i = 0; i < _joint_entries.size(); i++) {
    const JointEntry &entry = _joint_entries[i];

    me.add_string(entry.name);

    me.add_int16(entry.first_pos_frame);
    me.add_int16(entry.num_pos_frames);

    me.add_int16(entry.first_scale_frame);
    me.add_int16(entry.num_scale_frames);

    me.add_int16(entry.first_quat_frame);
    me.add_int16(entry.num_quat_frames);
  }

  me.add_uint16(_joint_pos.size());
  for (size_t i = 0; i < _joint_pos.size(); i++) {
    _joint_pos[i].write_datagram(me);
  }

  me.add_uint16(_joint_quat.size());
  for (size_t i = 0; i < _joint_quat.size(); i++) {
    _joint_quat[i].write_datagram(me);
  }

  me.add_uint16(_joint_scale.size());
  for (size_t i = 0; i < _joint_scale.size(); i++) {
    _joint_scale[i].write_datagram(me);
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
  */
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimBundle::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  /*
  PTA(JointFrameData) joint_frames;
  joint_frames.resize(scan.get_uint32());

  for (size_t i = 0; i < joint_frames.size(); i++) {
    joint_frames[i].pos.read_datagram(scan);
    joint_frames[i].quat.read_datagram(scan);
    joint_frames[i].scale.read_datagram(scan);
  }

  _joint_frames = joint_frames;

  size_t size = scan.get_uint16();
  for (size_t i = 0; i < size; i++) {
    std::string name = scan.get_string();
    int channel = scan.get_uint16();
    _joint_channel_names[name] = channel;
  }

  PTA_stdfloat slider_frames;
  slider_frames.resize(scan.get_uint32());

  for (size_t i = 0; i < slider_frames.size(); i++) {
    slider_frames[i] = scan.get_stdfloat();
  }

  _slider_frames = slider_frames;

  size = scan.get_uint16();
  for (size_t i = 0; i < size; i++) {
    std::string name = scan.get_string();
    int channel = scan.get_uint16();
    _slider_channel_names[name] = channel;
  }
  */

  _fps = scan.get_stdfloat();
  _num_frames = scan.get_uint16();
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
