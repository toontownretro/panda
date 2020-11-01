/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pgraphnodes.h
 * @author drose
 * @date 2008-11-05
 */

#ifndef CONFIG_PGRAPHNODES_H
#define CONFIG_PGRAPHNODES_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"
#include "configVariableString.h"
#include "lodNodeType.h"

class DSearchPath;

ConfigureDecl(config_pgraphnodes, EXPCL_PANDA_PGRAPHNODES, EXPTP_PANDA_PGRAPHNODES);
NotifyCategoryDecl(pgraphnodes, EXPCL_PANDA_PGRAPHNODES, EXPTP_PANDA_PGRAPHNODES);

extern ConfigVariableEnum<LODNodeType> default_lod_type;
extern ConfigVariableBool support_fade_lod;
extern ConfigVariableDouble lod_fade_time;
extern ConfigVariableString lod_fade_bin_name;
extern ConfigVariableInt lod_fade_bin_draw_order;
extern ConfigVariableInt lod_fade_state_override;
extern ConfigVariableBool verify_lods;

extern ConfigVariableInt parallax_mapping_samples;
extern ConfigVariableDouble parallax_mapping_scale;

extern ConfigVariableDouble csm_distance;
extern ConfigVariableInt csm_num_cascades;
extern ConfigVariableDouble csm_sun_distance;
extern ConfigVariableDouble csm_log_factor;
extern ConfigVariableDouble csm_border_bias;
extern ConfigVariableBool csm_fixed_film_size;

extern ConfigVariableInt shadow_buffer_sort;
extern ConfigVariableInt shadow_map_size;
extern ConfigVariableDouble shadow_depth_bias;
extern ConfigVariableDouble shadow_normal_offset_scale;
extern ConfigVariableDouble shadow_softness_factor;
extern ConfigVariableBool shadow_normal_offset_uv_space;

extern EXPCL_PANDA_PGRAPHNODES void init_libpgraphnodes();

#endif
