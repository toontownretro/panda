// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define BUILD_DIRECTORY $[and $[HAVE_GLES],$[HAVE_EGL]]

#define LOCAL_LIBS gsgbase display express
#define OTHER_LIBS interrogatedb \
  dtoolutil:c dtoolbase:c dtool:m prc
#define COMPONENT_LIBS \
  glesgsg egldisplay_gles1

#if $[HAVE_X11]
  #define C++FLAGS $[C++FLAGS] -DUSE_X11
#else
  #define C++FLAGS $[C++FLAGS] -DEGL_NO_X11
#endif

#if $[BUILD_COMPONENTS]
  #define BUILDING_DLL BUILDING_PANDAGLES_STUB
#else
  #define BUILDING_DLL BUILDING_PANDAGLES
#endif

#define DIR_TYPE metalib

#begin metalib_target
  #define TARGET pandagles
  #define SOURCES pandagles.cxx pandagles.h
  #define INSTALL_HEADERS pandagles.h
#end metalib_target
