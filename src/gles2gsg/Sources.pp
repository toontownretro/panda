#define BUILD_DIRECTORY $[HAVE_GLES2]
#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
#define USE_PACKAGES gles2
#begin lib_target
  #define TARGET gles2gsg
  #define LOCAL_LIBS \
    glstuff gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define BUILDING_DLL BUILDING_PANDA_GLES2GSG

  #define SOURCES \
    config_gles2gsg.h \
    gles2gsg.h

  #define COMPOSITE_SOURCES \
    config_gles2gsg.cxx \
    gles2gsg.cxx

  #define INSTALL_HEADERS \
    config_gles2gsg.h gles2gsg.h

#end lib_target
