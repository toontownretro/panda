#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc
#define LOCAL_LIBS \
    pgraph

#begin lib_target
  #define TARGET cull

  #define BUILDING_DLL BUILDING_PANDA_CULL

  #define SOURCES \
    binCullHandler.h binCullHandler.I \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    drawCullHandler.h drawCullHandler.I

  #define COMPOSITE_SOURCES \
    binCullHandler.cxx \
    config_cull.cxx \
    cullBinBackToFront.cxx \
    cullBinFixed.cxx \
    cullBinFrontToBack.cxx \
    cullBinStateSorted.cxx \
    cullBinUnsorted.cxx \
    drawCullHandler.cxx

  #define INSTALL_HEADERS \
    binCullHandler.h binCullHandler.I \
    config_cull.h \
    cullBinBackToFront.h cullBinBackToFront.I \
    cullBinFixed.h cullBinFixed.I \
    cullBinFrontToBack.h cullBinFrontToBack.I \
    cullBinStateSorted.h cullBinStateSorted.I \
    cullBinUnsorted.h cullBinUnsorted.I \
    drawCullHandler.h drawCullHandler.I

  #define IGATESCAN all

#end lib_target
