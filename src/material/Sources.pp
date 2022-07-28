#define LOCAL_LIBS gobj linmath pdx

#begin lib_target
  #define TARGET material
  #define BUILDING_DLL BUILDING_PANDA_MATERIAL

  #define SOURCES \
    config_material.h \
    eyeRefractMaterial.I eyeRefractMaterial.h \
    material.I material.h \
    materialParamBase.I materialParamBase.h \
    materialParamBool.I materialParamBool.h \
    materialParamColor.I materialParamColor.h \
    materialParamFloat.I materialParamFloat.h \
    materialParamMatrix.I materialParamMatrix.h \
    materialParamTexture.I materialParamTexture.h \
    materialParamVector.I materialParamVector.h \
    materialPool.I materialPool.h \
    materialRegistry.I materialRegistry.h \
    skyBoxMaterial.I skyBoxMaterial.h \
    sourceLightmappedMaterial.I sourceLightmappedMaterial.h \
    sourceMaterial.I sourceMaterial.h \
    sourceSkyMaterial.I sourceSkyMaterial.h \
    sourceWaterMaterial.I sourceWaterMaterial.h \
    standardMaterial.I standardMaterial.h \
    twoTextureMaterial.I twoTextureMaterial.h

  #define COMPOSITE_SOURCES \
    config_material.cxx \
    eyeRefractMaterial.cxx \
    material.cxx \
    materialParamBase.cxx \
    materialParamBool.cxx \
    materialParamColor.cxx \
    materialParamFloat.cxx \
    materialParamMatrix.cxx \
    materialParamTexture.cxx \
    materialParamVector.cxx \
    materialPool.cxx \
    materialRegistry.cxx \
    skyBoxMaterial.cxx \
    sourceLightmappedMaterial.cxx \
    sourceMaterial.cxx \
    sourceSkyMaterial.cxx \
    sourceWaterMaterial.cxx \
    standardMaterial.cxx \
    twoTextureMaterial.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_material
  #define LOCAL_LIBS material
  #define SOURCES test_material.cxx
#end test_bin_target
