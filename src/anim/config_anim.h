/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_anim.h
 * @author lachbr
 * @date 2021-02-22
 */

#ifndef CONFIG_ANIM_H
#define CONFIG_ANIM_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableList.h"

ConfigureDecl(config_anim, EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM);
NotifyCategoryDecl(anim, EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM);

EXPCL_PANDA_ANIM extern ConfigVariableBool compress_channels;
EXPCL_PANDA_ANIM extern ConfigVariableInt compress_chan_quality;
EXPCL_PANDA_ANIM extern ConfigVariableBool read_compressed_channels;
EXPCL_PANDA_ANIM extern ConfigVariableBool interpolate_frames;
EXPCL_PANDA_ANIM extern ConfigVariableBool restore_initial_pose;
EXPCL_PANDA_ANIM extern ConfigVariableInt async_bind_priority;
EXPCL_PANDA_ANIM extern ConfigVariableBool even_animation;
EXPCL_PANDA_ANIM extern ConfigVariableList anim_events;
EXPCL_PANDA_ANIM extern ConfigVariableList anim_activities;
EXPCL_PANDA_ANIM extern ConfigVariableBool source_delta_anims;

extern EXPCL_PANDA_ANIM void init_libanim();

#endif // CONFIG_ANIM_H
