#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc
//#define OSX_SYS_LIBS mx

#begin lib_target
  #define TARGET pandafx
  #define LOCAL_LIBS \
    display pgraph gobj linmath

  #define BUILDING_DLL BUILDING_PANDAFX

  #define SOURCES config_distort.h \
    projectionScreen.h projectionScreen.I \
    cylindricalLens.h cylindricalLens.I \
    fisheyeLens.h fisheyeLens.I \
    nonlinearImager.h nonlinearImager.I \
    oSphereLens.h oSphereLens.I \
    pSphereLens.h pSphereLens.I

  #define COMPOSITE_SOURCES \
    config_distort.cxx cylindricalLens.cxx fisheyeLens.cxx nonlinearImager.cxx \
    projectionScreen.cxx oSphereLens.cxx pSphereLens.cxx

  #define INSTALL_HEADERS

  #define IGATESCAN all

#end lib_target
