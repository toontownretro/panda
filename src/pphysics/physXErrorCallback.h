/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXErrorCallback.h
 * @author brian
 * @date 2021-04-13
 */

#ifndef PHYSXERRORCALLBACK_H
#define PHYSXERRORCALLBACK_H

#include "foundation/PxErrorCallback.h"

/**
 * Custom PhysX error callback implementation.  Outputs error information
 * through Panda's notify system.
 */
class PhysXErrorCallback : public physx::PxErrorCallback {
public:
  virtual void reportError(physx::PxErrorCode::Enum code, const char *message,
                           const char *file, int line) override;
};

#endif // PHYSXERRORCALLBACK_H
