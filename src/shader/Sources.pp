#begin lib_target
  #define TARGET shader
  #define LOCAL_LIBS pgraph putil gsgbase pgraphnodes postprocess
  #define OTHER_LIBS prc \
    dtool:m dtoolutil:c dtoolbase:c

  #define BUILDING_DLL BUILDING_PANDA_SHADER

  #define SOURCES \
    config_shader.h \
    csmDepthShader.h csmDepthShader.I \
    depthShader.h depthShader.I \
    eyeRefractShader.h eyeRefractShader.I \
    //lightmappedShader.h lightmappedShader.I \
    //noMatShader.h noMatShader.I \
    shaderBase.h shaderBase.I \
    shaderEnums.h \
    shaderManager.h shaderManager.I \
    shaderObject.h shaderObject.I \
    shaderSetup.h shaderSetup.I \
    shaderStage.h shaderStage.I \
    basicShader.h basicShader.I \
    //skyBoxShader.h skyBoxShader.I \
    sourceLightmappedShader.h sourceLightmappedShader.I \
    sourceShader.h sourceShader.I \
    sourceSkyShader.h sourceSkyShader.I \
    sourceWater.h sourceWater.I \
    spriteParticleShader.h spriteParticleShader.I \
    twoTextureShader.h twoTextureShader.I
    //vertexLitShader.h vertexLitShader.I

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define COMPOSITE_SOURCES \
    config_shader.cxx \
    csmDepthShader.cxx \
    depthShader.cxx \
    eyeRefractShader.cxx \
    //lightmappedShader.cxx \
    //noMatShader.cxx \
    shaderBase.cxx \
    shaderEnums.cxx \
    shaderManager.cxx \
    shaderObject.cxx \
    shaderSetup.cxx \
    shaderStage.cxx \
    basicShader.cxx \
    //skyBoxShader.cxx \
    sourceLightmappedShader.cxx \
    sourceShader.cxx \
    sourceSkyShader.cxx \
    sourceWater.cxx \
    spriteParticleShader.cxx \
    twoTextureShader.cxx
    //vertexLitShader.cxx

  #define IGATESCAN all

#end lib_target
