// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib

#if $[BUILD_COMPONENTS]
#define BUILDING_DLL BUILDING_PANDA_STUB
#else
#define BUILDING_DLL BUILDING_PANDA
#endif

#define COMPONENT_LIBS \
  anim collide cull device dgraph display dxml \
  event gobj grutil gsgbase linmath mathutil \
  movies parametrics pgraph pgraphnodes pgui pipeline \
  pnmimage pnmimagetypes pstatclient putil recorder text tform \
  nativenet net audio pnmtext steamnet shader \
  postprocess raytrace material pdx map particlesystem2 \
  jobsystem

#define LOCAL_LIBS \
  downloader express pandabase
#define OTHER_LIBS \
  pandaexpress:m \
  interrogatedb  \
  dtoolutil:c dtoolbase:c dtool:m prc

#begin metalib_target
  #define TARGET panda

  #define SOURCES panda.cxx panda.h
  #define INSTALL_HEADERS panda.h
#end metalib_target
