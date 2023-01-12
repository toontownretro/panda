/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_raytrace.h
 * @author brian
 * @date 2020-09-21
 */

#ifndef CONFIG_RAYTRACE_H
#define CONFIG_RAYTRACE_H

#include "dconfig.h"
#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(raytrace, EXPCL_PANDA_RAYTRACE, EXPTP_PANDA_RAYTRACE);

ConfigureDecl(config_raytrace, EXPCL_PANDA_RAYTRACE, EXPTP_PANDA_RAYTRACE);

// embree forward decls
struct RTCDeviceTy;
typedef struct RTCDeviceTy* RTCDevice;
struct RTCSceneTy;
typedef struct RTCSceneTy* RTCScene;
struct RTCGeometryTy;
typedef struct RTCGeometryTy* RTCGeometry;

extern EXPCL_PANDA_RAYTRACE void init_libraytrace();

#endif // CONFIG_RAYTRACE_H
