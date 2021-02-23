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
  Namable("") {
}

/**
 *
 */
CharacterJoint::
CharacterJoint(const std::string &name) :
  Namable(name)
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
  dg.add_string(get_name());

  dg.add_int16(_parent);

  _value.write_datagram(dg);
  _default_value.write_datagram(dg);
  _initial_net_transform_inverse.write_datagram(dg);
}
