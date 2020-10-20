/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ssemath.h
 * @author lachbr
 * @date 2020-10-20
 */

#ifndef CONFIG_SSEMATH_H
#define CONFIG_SSEMATH_H

#include "dconfig.h"

#ifdef BUILDING_PANDA_SSEMATH
#define EXPCL_PANDA_SSEMATH EXPORT_CLASS
#define EXPTP_PANDA_SSEMATH EXPORT_TEMPL
#else
#define EXPCL_PANDA_SSEMATH IMPORT_CLASS
#define EXPTP_PANDA_SSEMATH IMPORT_TEMPL
#endif

ConfigureDecl(config_ssemath, EXPCL_PANDA_SSEMATH, EXPTP_PANDA_SSEMATH);

extern EXPCL_PANDA_SSEMATH void init_libssemath();

#endif // CONFIG_SSEMATH_H
