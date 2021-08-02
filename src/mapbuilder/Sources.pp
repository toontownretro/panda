#define LOCAL_LIBS map pgraph pphysics grutil raytrace

#begin lib_target
  #define TARGET mapbuilder
  #define BUILDING_DLL BUILDING_PANDA_MAPBUILDER

  #define SOURCES \
    area.h area.I \
    config_mapbuilder.h \
    mapBuilder.h mapBuilder.I \
    mapBuildOptions.h mapBuildOptions.I \
    mapObjects.h mapObjects.I \
    portal.h portal.I \
    threadManager.h threadManager.I \
    visBuilder.h visBuilder.I \
    visTile.h visTile.I \
    voxelOctree.h voxelOctree.I \
    voxelSpace.h voxelSpace.I \
    winding.h winding.I

  #define COMPOSITE_SOURCES \
    area.cxx \
    config_mapbuilder.cxx \
    mapBuilder.cxx \
    mapBuildOptions.cxx \
    mapObjects.cxx \
    portal.cxx \
    threadManager.cxx \
    visBuilder.cxx \
    visTile.cxx \
    voxelOctree.cxx \
    voxelSpace.cxx \
    winding.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all
#end lib_target
