/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterAttachment.h
 * @author brian
 * @date 2021-06-13
 */

#ifndef CHARACTERATTACHMENT_H
#define CHARACTERATTACHMENT_H

#include "pandabase.h"
#include "namable.h"
#include "pandaNode.h"
#include "luse.h"
#include "pmap.h"

/**
 *
 */
class EXPCL_PANDA_ANIM CharacterAttachment : public Namable {
public:
  class ParentInfluence {
  public:
    int _parent;
    // Offset transform relative to parent joint.
    LMatrix4 _offset;
    // Weight of influence.
    float _weight;

    LMatrix4 _transform;
  };

  CharacterAttachment() = default;
  CharacterAttachment(const std::string &name);
  CharacterAttachment(const CharacterAttachment &copy);
  CharacterAttachment(CharacterAttachment &&other);
  void operator = (const CharacterAttachment &copy);
  void operator = (CharacterAttachment &&other);

  void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);
  int complete_pointers(int pi, TypedWritable **p_list, BamReader *manager);

private:
  // An attachment may be influenced by multiple parent joints with weights.
  typedef pmap<int, ParentInfluence> ParentInfluences;
  ParentInfluences _parents;

  // Current transform of the attachment relative to the root of the character.
  CPT(TransformState) _curr_transform;

  // If not empty, this is updated with the current transform.
  PT(PandaNode) _node;

  friend class Character;
};

#include "characterAttachment.I"

#endif // CHARACTERATTACHMENT_H
