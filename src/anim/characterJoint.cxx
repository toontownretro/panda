/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJoint.cxx
 * @author lachbr
 * @date 2021-02-22
 */

#include "characterJoint.h"
//#include "animChannelMatrixFixed.h"
#include "characterJointEffect.h"

/**
 * This is a private constructor only used during Bam reading.
 */
CharacterJoint::
CharacterJoint() :
  CharacterPart(),
  _has_forced_value(false),
  _forced_value(LMatrix4::ident_mat()),
  _default_scale(1),
  _default_quat(LQuaternion::ident_quat()) {
}

/**
 *
 */
CharacterJoint::
CharacterJoint(const CharacterJoint &other) :
  CharacterPart(other),
  _parent(other._parent),
  _children(other._children),
  //_vertex_transforms(other._vertex_transforms),
  _default_value(other._default_value),
  _default_pos(other._default_pos),
  _default_scale(other._default_scale),
  _default_quat(other._default_quat),
  _forced_value(other._forced_value),
  _has_forced_value(other._has_forced_value)
{
}

/**
 *
 */
CharacterJoint::
CharacterJoint(CharacterJoint &&other) :
  CharacterPart(std::move(other)),
  _parent(std::move(other._parent)),
  _children(std::move(other._children)),
  //_vertex_transforms(std::move(other._vertex_transforms)),
  _default_value(std::move(other._default_value)),
  _default_pos(std::move(other._default_pos)),
  _default_scale(std::move(other._default_scale)),
  _default_quat(std::move(other._default_quat)),
  _forced_value(std::move(other._forced_value)),
  _has_forced_value(std::move(other._has_forced_value))
{
}

/**
 *
 */
void CharacterJoint::
operator=(const CharacterJoint &other) {
  CharacterPart::operator=(other);
  _parent = other._parent;
  _children = other._children;
  _default_value = other._default_value;
  _default_pos = other._default_pos;
  _default_scale = other._default_scale;
  _default_quat = other._default_quat;
  _forced_value = other._forced_value;
  _has_forced_value = other._has_forced_value;
}

/**
 *
 */
CharacterJoint::
CharacterJoint(const std::string &name) :
  CharacterPart(name)
{
  _default_value = LMatrix4::ident_mat();
  _default_scale = LVecBase3(1);
  _default_quat = LQuaternion::ident_quat();
  _forced_value = LMatrix4::ident_mat();
  _has_forced_value = false;
}

/**
 *
 */
void CharacterJoint::
write_datagram(Datagram &dg) {
  CharacterPart::write_datagram(dg);

  dg.add_int16(_parent);

  dg.add_int16(_children.size());
  for (size_t i = 0; i < _children.size(); i++) {
    dg.add_int16(_children[i]);
  }

  //_value.write_datagram(dg);
  _default_value.write_datagram(dg);
  _default_pos.write_datagram(dg);
  _default_scale.write_datagram(dg);
  _default_quat.write_datagram(dg);
  //_initial_net_transform_inverse.write_datagram(dg);
}

/**
 *
 */
void CharacterJoint::
read_datagram(DatagramIterator &dgi) {
  CharacterPart::read_datagram(dgi);

  _parent = dgi.get_int16();

  _children.resize(dgi.get_int16());
  for (size_t i = 0; i < _children.size(); i++) {
    _children[i] = dgi.get_int16();
  }

  //_value.read_datagram(dgi);
  _default_value.read_datagram(dgi);
  _default_pos.read_datagram(dgi);
  _default_scale.read_datagram(dgi);
  _default_quat.read_datagram(dgi);
  //_initial_net_transform_inverse.read_datagram(dgi);
}

#if 0
/**
 *
 */
AnimChannelBase *CharacterJoint::
make_default_channel() const {
  //LVecBase3 pos, hpr, scale, shear;
  //decompose_matrix(_default_value, pos, hpr, scale, shear);
  //return new AnimChannelMatrixFixed(get_name(), pos, hpr, scale);

  return nullptr;
}
#endif
