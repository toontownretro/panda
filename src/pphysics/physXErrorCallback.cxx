/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXErrorCallback.cxx
 * @author brian
 * @date 2021-04-13
 */

#include "physXErrorCallback.h"
#include "config_pphysics.h"

void
do_report(std::ostream &out, const char *message, const char *file, int line) {
  out << "PhysX: " << std::string(message) << " (line " << line << " of "
      << std::string(file) << "\n";
}

/**
 * Handles an error message reported by PhysX.  Outputs the message through
 * Panda's notify system.
 */
void PhysXErrorCallback::
reportError(physx::PxErrorCode::Enum code, const char *message,
            const char *file, int line) {
  switch (code) {
  case physx::PxErrorCode::eDEBUG_INFO:
  case physx::PxErrorCode::eDEBUG_WARNING:
    if (pphysics_cat.is_debug()) {
      do_report(pphysics_cat.debug(), message, file, line);
    }
    break;

  case physx::PxErrorCode::ePERF_WARNING:
    do_report(pphysics_cat.warning(), message, file, line);
    break;

  case physx::PxErrorCode::eINVALID_PARAMETER:
  case physx::PxErrorCode::eINVALID_OPERATION:
  case physx::PxErrorCode::eINTERNAL_ERROR:
    do_report(pphysics_cat.error(), message, file, line);
    nassert_raise("PhysX error");
    break;

  case physx::PxErrorCode::eOUT_OF_MEMORY:
  case physx::PxErrorCode::eABORT:
    do_report(pphysics_cat.fatal(), message, file, line);
    nassert_raise("PhysX error");
    break;

  default:
    break;
  }
}
