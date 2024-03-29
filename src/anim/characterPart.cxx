/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterPart.cxx
 * @author brian
 * @date 2021-02-23
 */

#include "characterPart.h"

/**
 *
 */
CharacterPart::
CharacterPart() :
  Namable(""),
  _index(-1)
{
}

/**
 *
 */
CharacterPart::
CharacterPart(const CharacterPart &other) :
  Namable(other),
  _index(other._index)
{
}

/**
 *
 */
CharacterPart::
CharacterPart(CharacterPart &&other) :
  Namable(std::move(other)),
  _index(std::move(other._index))
{
}

/**
 *
 */
void CharacterPart::
operator=(const CharacterPart &other) {
  set_name(other.get_name());
  _index = other._index;
}

/**
 *
 */
CharacterPart::
CharacterPart(const std::string &name) :
  Namable(name),
  _index(-1)
{
}

/**
 *
 */
void CharacterPart::
write_datagram(Datagram &dg) {
  dg.add_string(get_name());
  dg.add_int16(_index);
}

/**
 *
 */
void CharacterPart::
read_datagram(DatagramIterator &dgi) {
  set_name(dgi.get_string());
  _index = dgi.get_int16();
}
