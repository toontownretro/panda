// Filename: config_wgldisplay.h
// Created by:  mike (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_WGLDISPLAY_H__
#define __CONFIG_WGLDISPLAY_H__

#include <pandabase.h>
#include <filename.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(wgldisplay, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern Filename get_icon_filename_();
extern bool gl_show_fps_meter;
extern float gl_fps_meter_update_interval;
extern bool gl_sync_video;
extern int gl_forced_pixfmt;

extern EXPCL_PANDAGL void init_libwgldisplay();

#endif /* __CONFIG_WGLDISPLAY_H__ */
