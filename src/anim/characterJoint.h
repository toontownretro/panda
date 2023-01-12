/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJoint.h
 * @author brian
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
 * Pose information for a character joint.  Stored separately from
 * CharacterJoint to be more cache-friendly during apply_pose().
 */
class CharacterJointPoseData {
public:
  bool _has_forced_value;

  // Index of joint on the character that this joint should be merged with.
  int _merge_joint;

  LMatrix4 _value;
  LMatrix4 _net_transform;
  int _parent;

  JointVertexTransform *_vertex_transform;
  LMatrix4 _initial_net_transform_inverse;

  LMatrix4 _forced_value;
};

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
  vector_int _children;

  // If non-NULL, the local transform of this node is used as a forced value
  // for the joint.
  PT(PandaNode) _controller;

  LMatrix4 _default_value;
  LVecBase3 _default_pos;
  LVecBase3 _default_scale;
  LVecBase3 _default_shear;
  LQuaternion _default_quat;

  // Should the joint be used to merge with the corresponding joint on a child
  // character?
  bool _merge;

  friend class Character;
};

#include "characterJoint.I"

#endif // CHARACTERJOINT_H
