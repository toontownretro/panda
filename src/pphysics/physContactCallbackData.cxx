/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physContactCallbackData.cxx
 * @author brian
 * @date 2021-04-24
 */

#include "physContactCallbackData.h"
#include "physEnums.h"

TypeHandle PhysContactCallbackData::_type_handle;

/**
 *
 */
PhysContactCallbackData::
PhysContactCallbackData(const physx::PxContactPairHeader &header) {
  _a = (PhysRigidActorNode *)header.actors[0]->userData;
  _b = (PhysRigidActorNode *)header.actors[1]->userData;

  _contact_pairs.resize(header.nbPairs);
  for (size_t i = 0; i < header.nbPairs; i++) {
    PhysContactPair &pair = _contact_pairs[i];
    const physx::PxContactPair &pxpair = header.pairs[i];

    pair._contact_type = 0;
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)) {
      pair._contact_type |= PhysEnums::CT_found;
    }
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)) {
      pair._contact_type |= PhysEnums::CT_persists;
    }
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_TOUCH_LOST)) {
      pair._contact_type |= PhysEnums::CT_lost;
    }
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_TOUCH_CCD)) {
      pair._contact_type |= PhysEnums::CT_ccd;
    }
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND)) {
      pair._contact_type |= PhysEnums::CT_threshold_force_found;
    }
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS)) {
      pair._contact_type |= PhysEnums::CT_threshold_force_persists;
    }
    if (pxpair.events.isSet(physx::PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST)) {
      pair._contact_type |= PhysEnums::CT_threshold_force_lost;
    }

    pair._shape_a = (PhysShape *)pxpair.shapes[0]->userData;
    pair._shape_b = (PhysShape *)pxpair.shapes[1]->userData;

    pair._contact_points.resize(pxpair.contactCount);
    pxpair.extractContacts(pair._contact_points.data(), pxpair.contactCount);
  }
}
