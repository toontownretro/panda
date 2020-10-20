#define LOCAL_LIBS mathutil

#begin lib_target
  #define TARGET ssemath

  #define BUILDING_DLL BUILDING_PANDA_SSEMATH

  #define SOURCES \
    config_ssemath.h \
    sse.h \
    ssemath.h
  #define COMPOSITE_SOURCES \
    config_ssemath.cxx \
    powsse.cxx \
    sse.cxx \
    sseconst.cxx
  #define INSTALL_HEADERS \
    $[SOURCES]

#end lib_target
