// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILD_DIRECTORY $[HAVE_EGG]
#define BUILDING_DLL BUILDING_PANDAEGG

#define COMPONENT_LIBS \
    egg2pg egg2sg egg

#define LOCAL_LIBS putil express
#define OTHER_LIBS interrogatedb \
    dtoolbase:c dtoolutil:c dtool:m prc

#begin metalib_target
  #define TARGET pandaegg

  #define SOURCES pandaegg.cxx pandaegg.h
  #define INSTALL_HEADERS pandaegg.h
#end metalib_target
