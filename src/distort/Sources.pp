#define OTHER_LIBS p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m
//#define OSX_SYS_LIBS mx

#begin lib_target
  #define TARGET pandafx
  #define LOCAL_LIBS \
    p3display p3pgraph p3gobj p3linmath

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
