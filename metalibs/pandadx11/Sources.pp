#define BUILD_DIRECTORY $[WINDOWS_PLATFORM]

// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define LOCAL_LIBS gsgbase display express gobj
#define OTHER_LIBS dtool:m dtoolbase:c dtoolutil:c

#define DIR_TYPE metalib

#if $[BUILD_COMPONENTS]
#define BUILDING_DLL BUILDING_PANDADX11_STUB
#else
#define BUILDING_DLL BUILDING_PANDADX11
#endif

#define COMPONENT_LIBS \
    dxgsg11

#begin metalib_target
  #define TARGET pandadx11
  #define SOURCES pandadx11.cxx pandadx11.h
#end metalib_target
