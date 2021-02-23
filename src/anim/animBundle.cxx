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
  _num_frames(copy._num_frames)
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

  me.add_int16(get_num_joint_channels());
  for (int i = 0; i < get_num_joint_channels(); i++) {
    manager->write_pointer(me, get_joint_channel(i));
  }

  me.add_int16(get_num_slider_channels());
  for (int i = 0; i < get_num_slider_channels(); i++) {
    manager->write_pointer(me, get_slider_channel(i));
  }

  me.add_stdfloat(_fps);
  me.add_uint16(_num_frames);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimBundle::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  _joint_channels.resize(scan.get_int16());
  manager->read_pointers(scan, (int)_joint_channels.size());

  _slider_channels.resize(scan.get_int16());
  manager->read_pointers(scan, (int)_slider_channels.size());

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
