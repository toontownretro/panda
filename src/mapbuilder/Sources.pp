#define BUILD_DIRECTORY $[and $[HAVE_MAPBUILDER], $[HAVE_OIDN]]

#define LOCAL_LIBS map pgraph pphysics grutil raytrace shader

#define USE_PACKAGES oidn steam_audio

#begin lib_target
  #define TARGET mapbuilder
  #define BUILDING_DLL BUILDING_PANDA_MAPBUILDER

  #if $[HAVE_STEAM_AUDIO]
    #define C++FLAGS $[C++FLAGS] -DHAVE_STEAM_AUDIO
  #endif

  #define SOURCES \
    area.h area.I \
    config_mapbuilder.h \
    lightBuilder.h lightBuilder.I \
    mapBuilder.h mapBuilder.I \
    mapBuildOptions.h mapBuildOptions.I \
    mapObjects.h mapObjects.I \
    portal.h portal.I \
    threadManager.h threadManager.I \
    visBuilder.h visBuilder.I \
    visBuilderBSP.h visBuilderBSP.I \
    visClusterSampler.h visClusterSampler.I \
    visTile.h visTile.I \
    voxelOctree.h voxelOctree.I \
    voxelSpace.h voxelSpace.I \
    winding.h winding.I

  #define COMPOSITE_SOURCES \
    area.cxx \
    config_mapbuilder.cxx \
    lightBuilder.cxx \
    mapBuilder.cxx \
    mapBuildOptions.cxx \
    mapObjects.cxx \
    portal.cxx \
    threadManager.cxx \
    visBuilder.cxx \
    visBuilderBSP.cxx \
    visClusterSampler.cxx \
    visTile.cxx \
    voxelOctree.cxx \
    voxelSpace.cxx \
    winding.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all
#end lib_target
