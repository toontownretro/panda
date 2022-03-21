/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterSlider.h
 * @author lachbr
 * @date 2021-02-22
 */

#ifndef CHARACTERSLIDER_H
#define CHARACTERSLIDER_H

#include "pandabase.h"
#include "characterPart.h"
#include "pset.h"

class CharacterVertexSlider;
class Datagram;
class DatagramIterator;
class Thread;

/**
 * A single slider of a Character.  Corresponds to a morph target on a mesh.
 * Receives a floating point value each frame that determines the influence of
 * a particular morph target.
 */
class EXPCL_PANDA_ANIM CharacterSlider final : public CharacterPart {
public:
  CharacterSlider();
  CharacterSlider(const CharacterSlider &other);
  CharacterSlider(CharacterSlider &&other);

  void operator = (const CharacterSlider &other);

private:
  CharacterSlider(const std::string &name);

  void write_datagram(Datagram &dg);
  void read_datagram(DatagramIterator &dgi);

  void set_value(PN_stdfloat value);
  PN_stdfloat get_value() const;

public:
  PN_stdfloat _default_value;

  CharacterVertexSlider *_vertex_slider;

  friend class Character;
};

#include "characterSlider.I"

#endif // CHARACTERSLIDER_H
