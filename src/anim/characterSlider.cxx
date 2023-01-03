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
#include "characterVertexSlider.h"
#include "thread.h"
#include "luse.h"

/**
 *
 */
CharacterSlider::
CharacterSlider() :
  CharacterPart(),
  _default_value(0.0f),
  _vertex_slider(nullptr)
{
}

/**
 *
 */
CharacterSlider::
CharacterSlider(const CharacterSlider &other) :
  CharacterPart(other),
  _default_value(other._default_value),
  _vertex_slider(nullptr)
{
}

/**
 *
 */
CharacterSlider::
CharacterSlider(CharacterSlider &&other) :
  CharacterPart(std::move(other)),
  _default_value(std::move(other._default_value)),
  _vertex_slider(std::move(other._vertex_slider))
{
}

/**
 *
 */
void CharacterSlider::
operator=(const CharacterSlider &other) {
  CharacterPart::operator = (other);
  _default_value = other._default_value;
  _vertex_slider = other._vertex_slider;
}

/**
 *
 */
CharacterSlider::
CharacterSlider(const std::string &name) :
  CharacterPart(name),
  _default_value(0.0f),
  _vertex_slider(nullptr)
{
}

/**
 *
 */
void CharacterSlider::
write_datagram(Datagram &dg) {
  CharacterPart::write_datagram(dg);
  dg.add_stdfloat(0.0f);
  //dg.add_stdfloat(_value);
  dg.add_stdfloat(_default_value);
}

/**
 *
 */
void CharacterSlider::
read_datagram(DatagramIterator &dgi) {
  CharacterPart::read_datagram(dgi);
  dgi.get_stdfloat();
  //_value = dgi.get_stdfloat();
  _default_value = dgi.get_stdfloat();
}

#if 0
/**
 *
 */
void CharacterSlider::
set_value(PN_stdfloat value) {
  // We set it directly on the VertexSlider as it is properly
  // cycled.
  nassertv(_vertex_slider != nullptr);
  PN_stdfloat current_value = _vertex_slider->get_slider();
  if (!IS_THRESHOLD_EQUAL(current_value, value, NEARLY_ZERO(PN_stdfloat))) {
    _vertex_slider->set_slider(value);
  }
}

/**
 *
 */
PN_stdfloat CharacterSlider::
get_value() const {
  // We go directly through the VertexSlider to get the value
  // as it is properly cycled.
  nassertr(_vertex_slider != nullptr, 0.0f);
  return _vertex_slider->get_slider();
}
#endif

/**
 *
 */
bool CharacterSlider::
mark_tables_modified(Thread *current_thread) {
  if (_vertex_slider != nullptr) {
    _vertex_slider->mark_tables_modified(current_thread);
    return true;
  }
  return false;
}
