#ifndef CONFIG_KEYVALUES_H
#define CONFIG_KEYVALUES_H

#include "pandabase.h"

#ifdef BUILDING_VIFPARSER
#define EXPCL_VIF EXPORT_CLASS
#define EXPTP_VIF EXPORT_TEMPL
#else
#define EXPCL_VIF IMPORT_CLASS
#define EXPTP_VIF IMPORT_TEMPL
#endif // BUILDING_VIFPARSER

extern EXPCL_VIF void init_libkeyvalues();

#endif // CONFIG_KEYVALUES_H
