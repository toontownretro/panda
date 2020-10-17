#begin lib_target
  #define TARGET shader
  #define LOCAL_LIBS pgraph putil
  #define OTHER_LIBS prc \
    dtool:m dtoolutil:c dtoolbase:c

  #define BUILDING_DLL BUILDING_PANDA_SHADER

  #define SOURCES \
    config_shader.h \
    shaderBase.h shaderBase.I \
    shaderManager.h shaderManager.I

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define COMPOSITE_SOURCES \
    config_shader.cxx \
    shaderBase.cxx \
    shaderManager.cxx

  #define IGATESCAN all

#end lib_target
