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

INLINE float feet_to_meters(float feet);
INLINE float meters_to_feet(float meters);
INLINE float inches_to_meters(float inches);
INLINE float meters_to_inches(float meters);
INLINE float mm_to_meters(float mm);
INLINE float meters_to_mm(float meters);
INLINE float cm_to_meters(float cm);
INLINE float meters_to_cm(float meters);

INLINE float g_to_kg(float g);
INLINE float kg_to_g(float kg);
INLINE float mg_to_kg(float mg);
INLINE float kg_to_mg(float kg);
INLINE float lb_to_kg(float lb);
INLINE float kg_to_lb(float kg);
INLINE float oz_to_kg(float oz);
INLINE float kg_to_oz(float kg);

extern EXPCL_PANDA_PPHYSICS float panda_length_to_physx(float length);
extern EXPCL_PANDA_PPHYSICS float physx_length_to_panda(float length);

extern EXPCL_PANDA_PPHYSICS float panda_mass_to_physx(float mass);
extern EXPCL_PANDA_PPHYSICS float physx_mass_to_panda(float mass);

INLINE physx::PxVec3 panda_vec_to_physx(const LVecBase3 &vec);
INLINE physx::PxExtendedVec3 panda_vec_to_physx_ex(const LVecBase3 &vec);
INLINE LVecBase3 physx_vec_to_panda(const physx::PxVec3 &vec);
INLINE LVecBase3 physx_ex_vec_to_panda(const physx::PxExtendedVec3 &vec);

INLINE LVecBase3 physx_norm_vec_to_panda(const physx::PxVec3 &vec);
INLINE physx::PxVec3 panda_norm_vec_to_physx(const LVecBase3 &vec);

INLINE physx::PxQuat panda_quat_to_physx(const LQuaternion &quat);
INLINE LQuaternion physx_quat_to_panda(const physx::PxQuat &quat);

INLINE float panda_ang_to_physx(float ang);
INLINE physx::PxVec3 panda_ang_to_physx(const LVecBase3 &ang);
INLINE float physx_ang_to_panda(float ang);
INLINE LVecBase3 physx_ang_to_panda(const physx::PxVec3 &ang);

INLINE physx::PxTransform panda_trans_to_physx(const TransformState *trans);
INLINE CPT(TransformState) physx_trans_to_panda(const physx::PxTransform &trans);

#include "physx_utils.I"

#endif // PHYSX_UTILS_H
