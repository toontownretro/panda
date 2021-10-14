#define BUILD_DIRECTORY $[HAVE_GLES]
#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
#define USE_PACKAGES gles
#begin lib_target
  #define TARGET glesgsg
  #define LOCAL_LIBS \
    glstuff gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define BUILDING_DLL BUILDING_PANDA_GLESGSG

  #define SOURCES \
    config_glesgsg.h \
    glesgsg.h

  #define COMPOSITE_SOURCES \
    config_glesgsg.cxx \
    glesgsg.cxx

  #define INSTALL_HEADERS \
    config_glesgsg.h glesgsg.h

#end lib_target
