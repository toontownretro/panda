// Filename: config_pstats.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_PSTATS_H
#define CONFIG_PSTATS_H

#include <pandabase.h>

#include <notifyCategoryProxy.h>

// Configure variables for pstats package.

NotifyCategoryDecl(pstats, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA string get_pstats_name();
extern EXPCL_PANDA double get_pstats_max_rate();
extern EXPCL_PANDA const string pstats_host; 
extern EXPCL_PANDA const int pstats_port;
extern EXPCL_PANDA const double pstats_target_frame_rate;

extern EXPCL_PANDA const bool pstats_scroll_mode;
extern EXPCL_PANDA const double pstats_history;

#endif
