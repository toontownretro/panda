#define LOCAL_LIBS gobj linmath

#begin lib_target
  #define TARGET material
  #define BUILDING_DLL BUILDING_PANDA_MATERIAL

  #define SOURCES \
    config_material.h \
    materialBase.I materialBase.h \
    materialParamBase.I materialParamBase.h \
    materialParamBool.I materialParamBool.h \
    materialParamColor.I materialParamColor.h \
    materialParamFloat.I materialParamFloat.h \
    materialParamTexture.I materialParamTexture.h \
    materialParamVector.I materialParamVector.h \
    materialPool.I materialPool.h \
    materialRegistry.I materialRegistry.h \
    standardMaterial.I standardMaterial.h

  #define COMPOSITE_SOURCES \
    config_material.cxx \
    materialBase.cxx \
    materialParamBase.cxx \
    materialParamBool.cxx \
    materialParamColor.cxx \
    materialParamFloat.cxx \
    materialParamTexture.cxx \
    materialParamVector.cxx \
    materialPool.cxx \
    materialRegistry.cxx \
    standardMaterial.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
