/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEvent.h
 * @author brian
 * @date 2021-06-14
 */

#ifndef ANIMEVENT_H
#define ANIMEVENT_H

#include "pandabase.h"
#include "sharedEnum.h"

/**
 * Shared animation event enum.
 */
class EXPCL_PANDA_ANIM AnimEvent : public SharedEnum {
PUBLISHED:
  static AnimEvent *ptr();

protected:
  virtual const ConfigVariableList &get_config_var() const override;

private:
  AnimEvent() = default;

  static AnimEvent *_ptr;
};

#endif // ANIMEVENT_H
