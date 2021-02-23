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

/**
 * This is a private constructor only used during Bam reading.
 */
CharacterJoint::
CharacterJoint() :
  CharacterPart() {
}

/**
 *
 */
CharacterJoint::
CharacterJoint(const std::string &name) :
  CharacterPart(name)
{
  _value = LMatrix4::ident_mat();
  _default_value = LMatrix4::ident_mat();
  _initial_net_transform_inverse = LMatrix4::ident_mat();
  _net_transform = LMatrix4::ident_mat();
  _skinning_matrix = LMatrix4::ident_mat();
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

  _value.write_datagram(dg);
  _default_value.write_datagram(dg);
  _initial_net_transform_inverse.write_datagram(dg);
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

  _value.read_datagram(dgi);
  _default_value.read_datagram(dgi);
  _initial_net_transform_inverse.read_datagram(dgi);
}
