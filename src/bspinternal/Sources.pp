#begin lib_target
  #define TARGET bspinternal
  #define LOCAL_LIBS keyvalues pgraph gobj

  #define BUILDING_DLL BUILDING_BSPINTERNAL

  #define SOURCES \
    bspMaterial.h \
    bspMaterialAttrib.h \
    config_bspinternal.h \
    textureStages.h

  #define COMPOSITE_SOURCES \
    bspMaterial.cxx \
    bspMaterialAttrib.cxx \
    config_bspinternal.cxx \
    textureStages.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
