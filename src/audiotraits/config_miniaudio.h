/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_miniaudio.h
 * @author brian
 * @date 2022-09-06
 */

#ifndef CONFIG_MINIAUDIO_H
#define CONFIG_MINIAUDIO_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "audioManager.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDeclNoExport(miniaudio);

extern ConfigVariableBool miniaudio_load_and_decode;
extern ConfigVariableBool miniaudio_decode_to_device_format;
extern ConfigVariableInt miniaudio_preload_threshold;
extern ConfigVariableInt miniaudio_sample_rate;
extern ConfigVariableInt miniaudio_num_channels;

extern "C" void init_libpminiaudio();
extern "C" Create_AudioManager_proc *get_audio_manager_func_pminiaudio();

#endif // CONFIG_MINIAUDIO_H
