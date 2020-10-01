#define BUILD_DIRECTORY $[HAVE_GLES]
#define OTHER_LIBS p3interrogatedb \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc
#define USE_PACKAGES gles
#begin lib_target
  #define TARGET p3glesgsg
  #define LOCAL_LIBS \
    p3glstuff p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage

  #define BUILDING_DLL BUILDING_PANDAGLES

  #define SOURCES \
    config_glesgsg.h \
    glesgsg.h

  #define COMPOSITE_SOURCES \
    config_glesgsg.cxx \
    glesgsg.cxx

  #define INSTALL_HEADERS \
    config_glesgsg.h glesgsg.h

#end lib_target
