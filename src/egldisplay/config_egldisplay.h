/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_egldisplay.h
 * @author cary
 * @date 2009-05-21
 */

#ifndef CONFIG_EGLDISPLAY_H
#define CONFIG_EGLDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

#if defined(OPENGLES_1) && defined(OPENGLES_2)
  #error OPENGLES_1 and OPENGLES_2 cannot be defined at the same time!
#endif

#ifdef OPENGLES_2
  NotifyCategoryDeclNoExport(egldisplay);

  extern void init_libegldisplay();
  extern const std::string get_egl_error_string(int error);
#elif defined(OPENGLES_1)
  NotifyCategoryDeclNoExport(egldisplay);

  extern void init_libegldisplay();
  extern const std::string get_egl_error_string(int error);
#else
  NotifyCategoryDeclNoExport(egldisplay);

  extern void init_libegldisplay();
  extern const std::string get_egl_error_string(int error);
#endif

#endif
