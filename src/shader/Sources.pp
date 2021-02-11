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
    lightmappedShader.h lightmappedShader.I \
    shaderBase.h shaderBase.I shaderBase.T \
    shaderEnums.h \
    shaderManager.h shaderManager.I \
    shaderSource.h shaderSource.I \
    shaderStage.h shaderStage.I shaderStage.T \
    vertexLitShader.h vertexLitShader.I

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define COMPOSITE_SOURCES \
    config_shader.cxx \
    csmDepthShader.cxx \
    depthShader.cxx \
    lightmappedShader.cxx \
    shaderBase.cxx \
    shaderEnums.cxx \
    shaderManager.cxx \
    shaderSource.cxx \
    shaderStage.cxx \
    vertexLitShader.cxx

  #define IGATESCAN all

#end lib_target
