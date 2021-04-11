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
#include "ordered_vector.h"
#include "jointVertexTransform.h"
#include "nodePathCollection.h"
#include "pandaNode.h"

class Datagram;
class DatagramIterator;

/**
 * A single joint of a Character.  Receives a matrix each frame that transforms
 * the vertices assigned to the joint.
 */
class EXPCL_PANDA_ANIM CharacterJoint final : public CharacterPart {
public:
  CharacterJoint();
  CharacterJoint(const CharacterJoint &other);
  CharacterJoint(CharacterJoint &&other);

  void operator = (const CharacterJoint &other);

private:
  CharacterJoint(const std::string &name);

  void write_datagram(Datagram &dg);
  void read_datagram(DatagramIterator &dgi);

  //AnimChannelBase *make_default_channel() const;

public:
  int _parent;
  vector_int _children;

  LMatrix4 _forced_value;
  bool _has_forced_value;

  LMatrix4 _default_value;

  friend class Character;
};

#include "characterJoint.I"

#endif // CHARACTERJOINT_H
