#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc
#define LOCAL_LIBS \
    pgraph

#begin lib_target
  #define TARGET cull

  #define BUILDING_DLL BUILDING_PANDA_CULL

  #define SOURCES \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I

  #define COMPOSITE_SOURCES \
    config_cull.cxx \
    cullBinBackToFront.cxx \
    cullBinFixed.cxx \
    cullBinFrontToBack.cxx \
    cullBinStateSorted.cxx \
    cullBinUnsorted.cxx

  #define INSTALL_HEADERS \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I

  #define IGATESCAN all

#end lib_target
