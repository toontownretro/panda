#define BUILD_DIRECTORY $[HAVE_EMBREE]

#define USE_PACKAGES embree
#define LOCAL_LIBS pgraph putil gobj

#begin lib_target
  #define TARGET raytrace
  #define BUILDING_DLL BUILDING_PANDA_RAYTRACE

  #define SOURCES \
    config_raytrace.h \
    rayTrace.h rayTrace.I \
    rayTraceGeometry.h \
    rayTraceHitResult.h \
    rayTraceHitResult4.h \
    rayTraceScene.h \
    rayTraceTriangleMesh.h

  #define COMPOSITE_SOURCES \
    config_raytrace.cxx \
    rayTrace.cxx \
    rayTraceGeometry.cxx \
    rayTraceScene.cxx \
    rayTraceTriangleMesh.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
