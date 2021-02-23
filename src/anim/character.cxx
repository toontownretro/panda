/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.cxx
 * @author lachbr
 * @date 2021-02-22
 */

#include "character.h"
#include "bamReader.h"
#include "config_anim.h"
#include "bamWriter.h"

TypeHandle Character::_type_handle;

/**
 *
 */
Character::
Character(const std::string &name) :
  Namable(name)
{
}

/**
 * Creates and returns a new CharacterJoint with the indicated name and parent
 * joint.
 *
 * The pointer returned by this method may become invalid if another
 * joint is added to the character, so don't store it anywhere persistently.
 */
CharacterJoint *Character::
make_joint(const std::string &name, int parent) {
  CharacterJoint joint(name);
  joint._parent = parent;

  size_t joint_index = _joints.size();
  _joints.push_back(std::move(joint));

  return &_joints[joint_index];
}

/**
 * Creates and returns a new CharacterSlider with the indicated name.
 *
 * The pointer returned by this method may become invalid if another
 * slider is added to the character, so don't store it anywhere persistently.
 */
CharacterSlider *Character::
make_slider(const std::string &name) {
  CharacterSlider slider(name);
  size_t slider_index = _sliders.size();
  _sliders.push_back(std::move(slider));
  return &_sliders[slider_index];
}

/**
 *
 */
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void Character::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());

  me.add_int16((int)_joints.size());
  for (int i = 0; i < (int)_joints.size(); i++) {
    _joints[i].write_datagram(me);
  }

  me.add_int16((int)_sliders.size());
  for (int i = 0; i < (int)_sliders.size(); i++) {
    _sliders[i].write_datagram(me);
  }

  manager->write_cdata(me, _cycler);
}

/**
 *
 */
Character::CData::
CData() :
  _frame_blend_flag(interpolate_frames),
  _anim_changed(false),
  _last_update(0.0),
  _root_xform(LMatrix4::ident_mat())
{
}

/**
 *
 */
Character::CData::
CData(const Character::CData &copy) :
  _frame_blend_flag(copy._frame_blend_flag),
  _root_xform(copy._root_xform),
  _anim_changed(copy._anim_changed),
  _last_update(copy._last_update)
{
  // Note that this copy constructor is not used by the PartBundle copy
  // constructor!  Any elements that must be copied between PartBundles should
  // also be explicitly copied there.
}

/**
 *
 */
CycleData *Character::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Character::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_bool(_frame_blend_flag);
  _root_xform.write_datagram(dg);

  // The remaining members are strictly dynamic.
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PartBundle.
 */
void Character::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _frame_blend_flag = scan.get_bool();
  _root_xform.read_datagram(scan);
}
