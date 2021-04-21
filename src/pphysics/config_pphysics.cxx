/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pphysics.cxx
 * @author lachbr
 * @date 2021-04-13
 */

#include "config_pphysics.h"
#include "physRigidActorNode.h"
#include "physRigidBodyNode.h"
#include "physRigidDynamicNode.h"
#include "physRigidStaticNode.h"
#include "physTriggerCallbackData.h"
#include "physSleepStateCallbackData.h"

ConfigureDef(config_pphysics);
ConfigureFn(config_pphysics) {
  init_libpphysics();
}

NotifyCategoryDef(pphysics, "");

ConfigVariableBool phys_enable_pvd
("phys-enable-pvd", false,
 PRC_DESC("If true, enables the PhysX Visual Debugger (PVD) when the physics "
          "system is initialized.  Default is false."));

ConfigVariableString phys_pvd_host
("phys-pvd-host", "localhost",
 PRC_DESC("Specifies the host address of the PhysX Visual Debugger application "
          "(PVD)."));

ConfigVariableInt phys_pvd_port
("phys-pvd-port", 5425,
 PRC_DESC("Specifies the port number of the PhysX Visual Debugger application "
          "(PVD)."));

ConfigVariableDouble phys_tolerance_scale
("phys-tolerance-scale", 3.28084,
 PRC_DESC("Controls the scale at which the physics simulation runs.  The "
          "default value is set up for a simulation that is done in feet, "
          "Panda's default unit of measurement."));

ConfigVariableBool phys_track_allocations
("phys-track-allocations", false,
 PRC_DESC("If true, PhysX will track its memory allocations.  Useful for "
          "debugging.  Default is false."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpphysics() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  PhysRigidActorNode::init_type();
  PhysRigidBodyNode::init_type();
  PhysRigidDynamicNode::init_type();
  PhysRigidStaticNode::init_type();
  PhysTriggerCallbackData::init_type();
  PhysSleepStateCallbackData::init_type();
}
