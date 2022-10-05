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
#include "refCallbackData.h"
#include "physTriggerCallbackData.h"
#include "physSleepStateCallbackData.h"
#include "physContactCallbackData.h"
#include "physShape.h"
#include "string_utils.h"
#include "physXAllocator.h"

/**
 *
 */
std::ostream &
operator << (std::ostream &out, PhysSolverType st) {
  switch(st) {
  case PST_pgs:
    out << "pgs";
    return out;
  case PST_tgs:
    out << "tgs";
    return out;
  default:
    out << "**invalid** (" << (int)st << ")";
    return out;
  }
}

/**
 *
 */
std::istream &
operator >> (std::istream &in, PhysSolverType &st) {
  std::string word;
  in >> word;

  if (cmp_nocase(word, "pgs") == 0) {
    st = PST_pgs;
  } else if (cmp_nocase(word, "tgs") == 0) {
    st = PST_tgs;
  } else {
    pphysics_cat.error()
      << "Invalid PhysSolverType: " << word << "\n";
    st = PST_pgs;
  }

  return in;
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, PhysPandaLengthUnit st) {
  switch(st) {
  case PPLU_meters:
    out << "meters";
    return out;
  case PPLU_feet:
    out << "feet";
    return out;
  case PPLU_inches:
    out << "inches";
    return out;
  case PPLU_millimeters:
    out << "millimeters";
    return out;
  case PPLU_centimeters:
    out << "centimeters";
    return out;
  default:
    out << "**invalid** (" << (int)st << ")";
    return out;
  }
}

/**
 *
 */
std::istream &
operator >> (std::istream &in, PhysPandaLengthUnit &st) {
  std::string word;
  in >> word;

  if (cmp_nocase(word, "meters") == 0) {
    st = PPLU_meters;
  } else if (cmp_nocase(word, "feet") == 0) {
    st = PPLU_feet;
  } else if (cmp_nocase(word, "inches") == 0) {
    st = PPLU_inches;
  } else if (cmp_nocase(word, "centimeters") == 0) {
    st = PPLU_centimeters;
  } else if (cmp_nocase(word, "millimeters") == 0) {
    st = PPLU_centimeters;
  } else {
    pphysics_cat.error()
      << "Invalid PhysPandaLengthUnit: " << word << ", defaulting to feet\n";
    st = PPLU_feet;
  }

  return in;
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, PhysPandaMassUnit st) {
  switch(st) {
  case PPMU_kilograms:
    out << "kilograms";
    return out;
  case PPMU_grams:
    out << "grams";
    return out;
  case PPMU_milligrams:
    out << "milligrams";
    return out;
  case PPMU_pounds:
    out << "pounds";
    return out;
  case PPMU_ounces:
    out << "ounces";
    return out;
  default:
    out << "**invalid** (" << (int)st << ")";
    return out;
  }
}

/**
 *
 */
std::istream &
operator >> (std::istream &in, PhysPandaMassUnit &st) {
  std::string word;
  in >> word;

  if (cmp_nocase(word, "kilograms") == 0) {
    st = PPMU_kilograms;
  } else if (cmp_nocase(word, "grams") == 0) {
    st = PPMU_grams;
  } else if (cmp_nocase(word, "milligrams") == 0) {
    st = PPMU_milligrams;
  } else if (cmp_nocase(word, "pounds") == 0) {
    st = PPMU_pounds;
  } else if (cmp_nocase(word, "ounces") == 0) {
    st = PPMU_ounces;
  } else {
    pphysics_cat.error()
      << "Invalid PhysPandaMassUnit: " << word << ", defaulting to kilograms\n";
    st = PPMU_kilograms;
  }

  return in;
}

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

ConfigVariableDouble phys_tolerance_length
("phys-tolerance-length", 1,
 PRC_DESC("Controls the scale at which the physics simulation runs.  The "
          "default value is set up for a simulation that is done in feet, "
          "Panda's default unit of measurement."));

ConfigVariableDouble phys_tolerance_speed
("phys-tolerance-speed", 10,
 PRC_DESC("Controls the scale at which the physics simulation runs.  The "
          "default value is set up for a simulation that is done in feet, "
          "Panda's default unit of measurement."));

ConfigVariableBool phys_track_allocations
("phys-track-allocations", false,
 PRC_DESC("If true, PhysX will track its memory allocations.  Useful for "
          "debugging.  Default is false."));

ConfigVariableEnum<PhysSolverType> phys_solver
("phys-solver", PST_pgs,
 PRC_DESC("The physics solver type to use.  Default is Projected Gauss-Seidel "
          "(PGS)."));

ConfigVariableEnum<PhysPandaLengthUnit> phys_panda_length_unit
("phys-panda-length-unit", PPLU_feet,
 PRC_DESC("Specifies the unit of length that Panda/game code uses.  Lengths "
          "will be converted from this unit to PhysX units (meters) when passed "
          "into the API.  The default is feet."));

ConfigVariableEnum<PhysPandaMassUnit> phys_panda_mass_unit
("phys-panda-mass-unit", PPMU_kilograms,
 PRC_DESC("Specifies the unit of mass that Panda/game code uses.  Masses will "
          "will be converted from this unit to PhysX units (kilograms) when "
          "passed into the API.  The default is kilograms."));

ConfigVariableBool phys_ragdoll_projection
("phys-ragdoll-projection", true,
 PRC_DESC("If true, enables projection on ragdoll joints."));

ConfigVariableDouble phys_ragdoll_contact_distance_ratio
("phys-ragdoll-contact-distance-ratio", 0.99,
 PRC_DESC("Ragdoll joint contact distance ratio."));

ConfigVariableDouble phys_ragdoll_projection_angular_tolerance
("phys-ragdoll-projection-angular-tolerance", 15.0,
 PRC_DESC("Ragdoll joint angular projection threshold (in degrees)."));

ConfigVariableDouble phys_ragdoll_projection_linear_tolerance
("phys-ragdoll-projection-linear-tolerance", 8.0,
 PRC_DESC("Ragdoll joint linear projection threshold (in Panda units)."));

ConfigVariableInt phys_ragdoll_pos_iterations
("phys-ragdoll-pos-iterations", 20,
 PRC_DESC("Number of ragdoll limb solver position iterations."));

ConfigVariableInt phys_ragdoll_vel_iterations
("phys-ragdoll-vel-iterations", 20,
 PRC_DESC("Number of ragdoll limb solver velocity iterations."));

ConfigVariableDouble phys_ragdoll_max_depenetration_vel
("phys-ragdoll-max-depenetration-vel", 1000.0,
 PRC_DESC("Max ragdoll limb depenetration velocity (in Panda units)."));

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

  RefCallbackData::init_type();
  PhysRigidActorNode::init_type();
  PhysRigidBodyNode::init_type();
  PhysRigidDynamicNode::init_type();
  PhysRigidStaticNode::init_type();
  PhysTriggerCallbackData::init_type();
  PhysSleepStateCallbackData::init_type();
  PhysContactCallbackData::init_type();
  PhysShape::init_type();
  PhysXAllocator::init_type();
}
