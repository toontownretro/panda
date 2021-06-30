/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animActivity.h
 * @author brian
 * @date 2021-06-14
 */

#ifndef ANIMACTIVITY_H
#define ANIMACTIVITY_H

#include "pandabase.h"
#include "sharedEnum.h"

/**
 * Shared animation activity enum.
 */
class EXPCL_PANDA_ANIM AnimActivity : public SharedEnum {
PUBLISHED:
  static AnimActivity *ptr();

protected:
  virtual const ConfigVariableList &get_config_var() const override;

private:
  AnimActivity() = default;

  static AnimActivity *_ptr;
};

#endif // ANIMACTIVITY_H
