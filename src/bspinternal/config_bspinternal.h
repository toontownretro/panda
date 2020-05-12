#pragma once

#include "pandabase.h"

#ifdef BUILDING_BSPINTERNAL
#define EXPCL_BSPINTERNAL EXPORT_CLASS
#define EXPTP_BSPINTERNAL EXPORT_TEMPL
#else
#define EXPCL_BSPINTERNAL IMPORT_CLASS
#define EXPTP_BSPINTERNAL IMPORT_TEMPL
#endif

extern EXPCL_BSPINTERNAL void init_libbspinternal();
