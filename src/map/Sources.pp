#begin lib_target
  #define TARGET map
  #define BUILDING_DLL BUILDING_PANDA_MAP

  #define LOCAL_LIBS pgraph linmath raytrace pgraphnodes jobsystem

  //#define USE_PACKAGES tbb

  #define SOURCES \
    aabbTree.h aabbTree.I \
    bspTree.h bspTree.I \
    config_map.h \
    dynamicVisNode.h dynamicVisNode.I \
    kdTree.h kdTree.I \
    mapCullTraverser.h mapCullTraverser.I \
    mapData.h mapData.I \
    mapEntity.h mapEntity.I \
    mapLightingEffect.h mapLightingEffect.I \
    mapRender.h mapRender.I \
    mapRoot.h mapRoot.I \
    spatialPartition.h spatialPartition.I \
    staticPartitionedObjectNode.h staticPartitionedObjectNode.I

  #define COMPOSITE_SOURCES \
    aabbTree.cxx \
    bspTree.cxx \
    config_map.cxx \
    dynamicVisNode.cxx \
    kdTree.cxx \
    mapCullTraverser.cxx \
    mapData.cxx \
    mapEntity.cxx \
    mapLightingEffect.cxx \
    mapRender.cxx \
    mapRoot.cxx \
    spatialPartition.cxx \
    staticPartitionedObjectNode.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
