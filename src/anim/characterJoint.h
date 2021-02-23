/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJoint.h
 * @author lachbr
 * @date 2021-02-22
 */

#ifndef CHARACTERJOINT_H
#define CHARACTERJOINT_H

#include "pandabase.h"
#include "characterPart.h"
#include "luse.h"
#include "vector_int.h"

class Datagram;
class DatagramIterator;

/**
 * A single joint of a Character.  Receives a matrix each frame that transforms
 * the vertices assigned to the joint.
 */
class EXPCL_PANDA_ANIM CharacterJoint final : public CharacterPart {
private:
  CharacterJoint();
  CharacterJoint(const std::string &name);

  void write_datagram(Datagram &dg);
  void read_datagram(DatagramIterator &dgi);

public:
  int _parent;
  vector_int _children;

  LMatrix4 _value;
  LMatrix4 _default_value;

  // These are filled in as the joint animates.
  LMatrix4 _net_transform;
  LMatrix4 _initial_net_transform_inverse;

  // This is the product of the above; the matrix that gets applied to a
  // vertex (whose coordinates are in the coordinate space of the character
  // in its neutral pose) to transform it from its neutral position to its
  // animated position.
  LMatrix4 _skinning_matrix;

  friend class Character;
};

#include "characterJoint.I"

#endif // CHARACTERJOINT_H
