/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physx_utils.h
 * @author brian
 * @date 2021-04-21
 */

#ifndef PHYSX_UTILS_H
#define PHYSX_UTILS_H

#include "pandabase.h"
#include "luse.h"
#include "transformState.h"
#include "physx_includes.h"

INLINE LVecBase3 PxVec3_to_Vec3(const physx::PxVec3 &vec);
INLINE physx::PxVec3 Vec3_to_PxVec3(const LVecBase3 &vec);

INLINE LQuaternion PxQuat_to_Quat(const physx::PxQuat &quat);
INLINE physx::PxQuat Quat_to_PxQuat(const LQuaternion &quat);

INLINE CPT(TransformState) PxTransform_to_TransformState(const physx::PxTransform &trans);
INLINE physx::PxTransform TransformState_to_PxTransform(const TransformState *state);

#include "physx_utils.I"

#endif // PHYSX_UTILS_H
