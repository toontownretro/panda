// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAGLES2
#define BUILD_DIRECTORY $[HAVE_GLES2]

#define COMPONENT_LIBS \
    gles2gsg egldisplay_gles2

#define LOCAL_LIBS gsgbase display express
#define OTHER_LIBS interrogatedb  \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin metalib_target
  #if $[HAVE_X11]
    #define C++FLAGS $[C++FLAGS] -DUSE_X11
  #else
    #define C++FLAGS $[C++FLAGS] -DEGL_NO_X11
  #endif

  #define TARGET pandagles2
  #define SOURCES pandagles2.cxx pandagles2.h
  #define INSTALL_HEADERS pandagles2.h
#end metalib_target
