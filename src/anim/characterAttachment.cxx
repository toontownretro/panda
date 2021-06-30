/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterAttachment.cxx
 * @author brian
 * @date 2021-06-13
 */

#include "characterAttachment.h"

/**
 *
 */
CharacterAttachment::
CharacterAttachment(const std::string &name) :
  Namable(name),
  _curr_transform(TransformState::make_identity())
{
}

/**
 *
 */
CharacterAttachment::
CharacterAttachment(const CharacterAttachment &copy) :
  Namable(copy),
  _curr_transform(copy._curr_transform),
  _parents(copy._parents),
  _node(copy._node)
{
}

/**
 *
 */
CharacterAttachment::
CharacterAttachment(CharacterAttachment &&other) :
  Namable(std::move(other)),
  _curr_transform(std::move(other._curr_transform)),
  _parents(std::move(other._parents)),
  _node(std::move(other._node))
{
}

/**
 *
 */
void CharacterAttachment::
operator = (const CharacterAttachment &other) {
  Namable::operator = (other);
  _curr_transform = other._curr_transform;
  _parents = other._parents;
  _node = other._node;
}

/**
 *
 */
void CharacterAttachment::
operator = (CharacterAttachment &&other) {
  Namable::operator = (std::move(other));
  _curr_transform = std::move(other._curr_transform);
  _parents = std::move(other._parents);
  _node = std::move(other._node);
}

/**
 *
 */
void CharacterAttachment::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  me.add_uint8(_parents.size());
  for (auto it = _parents.begin(); it != _parents.end(); ++it) {
    me.add_uint16((*it).first);
    (*it).second._offset.write_datagram(me);
    me.add_float32((*it).second._weight);
  }
  manager->write_pointer(me, (TypedWritable *)_curr_transform.p());
  manager->write_pointer(me, (TypedWritable *)_node.p());
}

/**
 *
 */
void CharacterAttachment::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  size_t num_parents = scan.get_uint8();
  for (size_t i = 0; i < num_parents; i++) {
    int parent = scan.get_uint16();
    ParentInfluence inf;
    inf._parent = parent;
    inf._offset.read_datagram(scan);
    inf._weight = scan.get_float32();
    _parents[parent] = inf;
  }
  manager->read_pointer(scan); // current transform
  manager->read_pointer(scan); // node
}

/**
 *
 */
int CharacterAttachment::
complete_pointers(int pi, TypedWritable **p_list, BamReader *manager) {
  _curr_transform = DCAST(TransformState, p_list[pi++]);
  _node = DCAST(PandaNode, p_list[pi++]);
  return pi;
}
