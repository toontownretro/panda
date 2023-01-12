/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterPart.h
 * @author brian
 * @date 2021-02-23
 */

#ifndef CHARACTERPART_H
#define CHARACTERPART_H

#include "pandabase.h"
#include "namable.h"
#include "pvector.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "vector_int.h"

/**
 * Base class for CharacterJoint and CharacterSlider.
 */
class EXPCL_PANDA_ANIM CharacterPart : public Namable {
public:
  CharacterPart();
  CharacterPart(const CharacterPart &other);
  CharacterPart(CharacterPart &&other);
  void operator = (const CharacterPart &other);

protected:
  CharacterPart(const std::string &name);

  void write_datagram(Datagram &dg);
  void read_datagram(DatagramIterator &dgi);

protected:
  // The index of this part into the Character's list of parts of this type.
  int _index;

  friend class Character;
};

#include "characterPart.I"

#endif // CHARACTERPART_H
