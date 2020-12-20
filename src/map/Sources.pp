#begin lib_target
  #define TARGET map
  #define BUILDING_DLL BUILDING_PANDA_MAP

  #define LOCAL_LIBS pgraph linmath

  #define SOURCES \
    config_map.h

  #define COMPOSITE_SOURCES \
    config_map.cxx

  #define INSTALL_HEADERS \
    config_map.h

  #define IGATESCAN all

#end lib_target
