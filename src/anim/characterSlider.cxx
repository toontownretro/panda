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

/**
 *
 */
CharacterSlider::
CharacterSlider() :
  CharacterPart(),
  _value(0.0f),
  _default_value(0.0f),
  _vertex_slider(nullptr),
  _val_changed(true)
{
}

/**
 *
 */
CharacterSlider::
CharacterSlider(const CharacterSlider &other) :
  CharacterPart(other),
  _value(other._value),
  _default_value(other._default_value),
  _vertex_slider(nullptr),
  _val_changed(other._val_changed)
{
}

/**
 *
 */
CharacterSlider::
CharacterSlider(CharacterSlider &&other) :
  CharacterPart(std::move(other)),
  _value(std::move(other._value)),
  _default_value(std::move(other._default_value)),
  _vertex_slider(std::move(other._vertex_slider)),
  _val_changed(std::move(other._val_changed))
{
}

/**
 *
 */
void CharacterSlider::
operator=(const CharacterSlider &other) {
  CharacterPart::operator = (other);
  _value = other._value;
  _default_value = other._default_value;
  _vertex_slider = other._vertex_slider;
  _val_changed = other._val_changed;
}

/**
 *
 */
CharacterSlider::
CharacterSlider(const std::string &name) :
  CharacterPart(name),
  _value(0.0f),
  _default_value(0.0f),
  _vertex_slider(nullptr),
  _val_changed(true)
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

/**
 *
 */
void CharacterSlider::
set_value(PN_stdfloat value) {
  if (!_val_changed) {
    _val_changed = value != _value;
  }
  _value = value;
}

/**
 *
 */
bool CharacterSlider::
is_val_changed() const {
  return _val_changed;
}

/**
 *
 */
void CharacterSlider::
clear_val_changed() {
  _val_changed = false;
}

/**
 *
 */
void CharacterSlider::
update(Thread *current_thread) {
  if (_val_changed && _vertex_slider != nullptr) {
    _vertex_slider->mark_modified(current_thread);
    _val_changed = false;
  }
}
