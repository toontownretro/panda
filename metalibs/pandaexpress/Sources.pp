// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib

#if $[BUILD_COMPONENTS]
#define BUILDING_DLL BUILDING_PANDAEXPRESS_STUB
#else
#define BUILDING_DLL BUILDING_PANDAEXPRESS
#endif

#define USE_PACKAGES net

#define COMPONENT_LIBS downloader express pandabase
#define OTHER_LIBS prc interrogatedb dtoolutil:c dtoolbase:c dtool:m

#begin metalib_target
  #define TARGET pandaexpress

  #define SOURCES pandaexpress.cxx
  #define WIN_SYS_LIBS \
     advapi32.lib ws2_32.lib $[WIN_SYS_LIBS]

#end metalib_target
