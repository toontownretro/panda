/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterSlider.cxx
 * @author lachbr
 * @date 2021-02-22
 */

#include "characterSlider.h"

/**
 *
 */
CharacterSlider::
CharacterSlider() :
  CharacterPart(),
  _value(0.0f),
  _default_value(0.0f)
{
}

/**
 *
 */
CharacterSlider::
CharacterSlider(const std::string &name) :
  CharacterPart(name),
  _value(0.0f),
  _default_value(0.0f)
{
}

/**
 *
 */
void CharacterSlider::
write_datagram(Datagram &dg) {
  CharacterPart::write_datagram(dg);
  dg.add_stdfloat(_value);
  dg.add_stdfloat(_default_value);
}

/**
 *
 */
void CharacterSlider::
read_datagram(DatagramIterator &dgi) {
  CharacterPart::read_datagram(dgi);
  _value = dgi.get_stdfloat();
  _default_value = dgi.get_stdfloat();
}
