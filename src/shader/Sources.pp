#begin lib_target
  #define TARGET shader
  #define LOCAL_LIBS pgraph putil gsgbase
  #define OTHER_LIBS prc \
    dtool:m dtoolutil:c dtoolbase:c

  #define BUILDING_DLL BUILDING_PANDA_SHADER

  #define SOURCES \
    config_shader.h \
    defaultShader.h defaultShader.I \
    shaderBase.h shaderBase.I shaderBase.T \
    shaderManager.h shaderManager.I \
    shaderSource.h shaderSource.I \
    shaderStage.h shaderStage.I shaderStage.T

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define COMPOSITE_SOURCES \
    config_shader.cxx \
    defaultShader.cxx \
    shaderBase.cxx \
    shaderManager.cxx \
    shaderSource.cxx \
    shaderStage.cxx

  #define IGATESCAN all

#end lib_target
