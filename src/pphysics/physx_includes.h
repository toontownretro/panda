/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physx_includes.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSX_INCLUDES_H
#define PHYSX_INCLUDES_H

#if !defined(NDEBUG) && !defined(_DEBUG)
#define NDEBUG
#include "PxPhysicsAPI.h"
#undef NDEBUG
#else
#include "PxPhysicsAPI.h"
#endif

#endif // PHYSX_INCLUDES_H
