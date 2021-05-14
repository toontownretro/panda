/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weightList.cxx
 * @author brian
 * @date 2021-05-07
 */

#include "weightList.h"
#include "character.h"

TypeHandle WeightList::_type_handle;

/**
 *
 */
WeightList::
WeightList(Character *character, const WeightListDesc &desc) :
  Namable(desc.get_name())
{
  _weights.resize(character->get_num_joints());

  r_fill_weights(character, desc, 0, 0.0);
}

/**
 *
 */
void WeightList::
r_fill_weights(Character *character, const WeightListDesc &desc,
               int joint, PN_stdfloat weight) {
  WeightListDesc::Weights::const_iterator it = desc._weights.find(
    character->get_joint_name(joint));
  if (it != desc._weights.end()) {
    weight = (*it).second;
  }

  _weights[joint] = weight;

  for (int i = 0; i < character->get_joint_num_children(joint); i++) {
    r_fill_weights(character, desc, character->get_joint_child(joint, i), weight);
  }
}
