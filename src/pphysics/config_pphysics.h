/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pphysics.h
 * @author brian
 * @date 2021-04-13
 */

#ifndef CONFIG_PPHYSICS_H
#define CONFIG_PPHYSICS_H

#include "pandabase.h"
#include "dconfig.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableEnum.h"

ConfigureDecl(config_pphysics, EXPCL_PANDA_PPHYSICS, EXPTP_PANDA_PPHYSICS);
NotifyCategoryDecl(pphysics, EXPCL_PANDA_PPHYSICS, EXPTP_PANDA_PPHYSICS);

enum PhysSolverType {
  PST_pgs, // Projected Gauss-Seidel
  PST_tgs, // Temporal Gauss-Seidel
};

/**
 * The unit of measurement that Panda is using.  All values will be converted
 * from this unit of measurement to meters when passed into PhysX.
 */
enum PhysPandaLengthUnit {
  PPLU_meters,
  PPLU_feet,
  PPLU_inches,
  PPLU_centimeters,
  PPLU_millimeters,
};

/**
 * The unit of mass that Panda is using.  All values will be converted from
 * this unit of mass to kilograms when passed into PhysX.
 */
enum PhysPandaMassUnit {
  PPMU_kilograms,
  PPMU_grams,
  PPMU_milligrams,
  PPMU_pounds,
  PPMU_ounces
};

EXPCL_PANDA_PPHYSICS std::ostream &operator << (std::ostream &out, PhysSolverType st);
EXPCL_PANDA_PPHYSICS std::istream &operator >> (std::istream &in, PhysSolverType &st);
EXPCL_PANDA_PPHYSICS std::ostream &operator << (std::ostream &out, PhysPandaLengthUnit pu);
EXPCL_PANDA_PPHYSICS std::istream &operator >> (std::istream &in, PhysPandaLengthUnit &pu);
EXPCL_PANDA_PPHYSICS std::ostream &operator << (std::ostream &out, PhysPandaMassUnit pu);
EXPCL_PANDA_PPHYSICS std::istream &operator >> (std::istream &in, PhysPandaMassUnit &pu);

extern EXPCL_PANDA_PPHYSICS ConfigVariableBool phys_enable_pvd;
extern EXPCL_PANDA_PPHYSICS ConfigVariableString phys_pvd_host;
extern EXPCL_PANDA_PPHYSICS ConfigVariableInt phys_pvd_port;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_tolerance_length;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_tolerance_speed;
extern EXPCL_PANDA_PPHYSICS ConfigVariableBool phys_track_allocations;
extern EXPCL_PANDA_PPHYSICS ConfigVariableEnum<PhysSolverType> phys_solver;
extern EXPCL_PANDA_PPHYSICS ConfigVariableEnum<PhysPandaLengthUnit> phys_panda_length_unit;
extern EXPCL_PANDA_PPHYSICS ConfigVariableEnum<PhysPandaMassUnit> phys_panda_mass_unit;

extern EXPCL_PANDA_PPHYSICS ConfigVariableBool phys_ragdoll_projection;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_ragdoll_contact_distance_ratio;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_ragdoll_projection_linear_tolerance;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_ragdoll_projection_angular_tolerance;
extern EXPCL_PANDA_PPHYSICS ConfigVariableInt phys_ragdoll_pos_iterations;
extern EXPCL_PANDA_PPHYSICS ConfigVariableInt phys_ragdoll_vel_iterations;
extern EXPCL_PANDA_PPHYSICS ConfigVariableDouble phys_ragdoll_max_depenetration_vel;

extern EXPCL_PANDA_PPHYSICS void init_libpphysics();

#endif // CONFIG_PPHYSICS_H
