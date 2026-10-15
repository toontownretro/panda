#define OTHER_LIBS interrogatedb:c \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET lerp

  #define BUILDING_DLL BUILDING_PANDA_LERP

  #define LOCAL_LIBS \
    event linmath putil

  #define SOURCES \
    config_lerp.h lerp.h lerpblend.h lerpfunctor.h

  #define COMPOSITE_SOURCES \
    config_lerp.cxx lerp.cxx lerpblend.cxx lerpfunctor.cxx

  #define INSTALL_HEADERS \
    config_lerp.h lerp.h lerpblend.h lerpfunctor.h

  #define IGATESCAN all

#end lib_target
