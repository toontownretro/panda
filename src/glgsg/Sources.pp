#define BUILD_DIRECTORY $[HAVE_GL]
#define OTHER_LIBS p3interrogatedb:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m
#define USE_PACKAGES gl cg cggl
#begin lib_target
  #define TARGET p3glgsg
  #define LOCAL_LIBS \
    p3glstuff p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage

  #define SOURCES \
    config_glgsg.h \
    glgsg.h

  #define COMPOSITE_SOURCES \
    config_glgsg.cxx \
    glgsg.cxx

  #define INSTALL_HEADERS \
    config_glgsg.h glgsg.h

#end lib_target
