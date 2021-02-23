/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterPart.h
 * @author lachbr
 * @date 2021-02-23
 */

#ifndef CHARACTERPART_H
#define CHARACTERPART_H

#include "pandabase.h"
#include "namable.h"
#include "animChannelBase.h"
#include "pvector.h"

/**
 * Base class for CharacterJoint and CharacterSlider.
 */
class EXPCL_PANDA_ANIM CharacterPart : public Namable {

protected:
  // The index of this part into the Character's list of parts of this type.
  int _index;

  // This is the vector of all channels bound to this part.
  typedef pvector< PT(AnimChannelBase) > Channels;
  Channels _channels;
};

#include "characterPart.I"

#endif // CHARACTERPART_H
