#begin lib_target
  #define TARGET map
  #define BUILDING_DLL BUILDING_PANDA_MAP

  #define LOCAL_LIBS pgraph linmath

  #define SOURCES \
    aabbTree.h aabbTree.I \
    config_map.h \
    kdTree.h kdTree.I \
    mapData.h mapData.I \
    mapEntity.h mapEntity.I \
    mapModel.h mapModel.I \
    mapRoot.h mapRoot.I

  #define COMPOSITE_SOURCES \
    aabbTree.cxx \
    config_map.cxx \
    kdTree.cxx \
    mapData.cxx \
    mapEntity.cxx \
    mapModel.cxx \
    mapRoot.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
