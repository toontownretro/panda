#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET gsgbase
  #define LOCAL_LIBS \
    putil linmath

  #define BUILDING_DLL BUILDING_PANDA_GSGBASE

  #define SOURCES \
    config_gsgbase.h \
    graphicsOutputBase.I graphicsOutputBase.h \
    graphicsStateGuardianBase.h \
    shaderManagerBase.I shaderManagerBase.h

  #define COMPOSITE_SOURCES \
    config_gsgbase.cxx \
    graphicsOutputBase.cxx \
    graphicsStateGuardianBase.cxx \
    shaderManagerBase.cxx

  #define INSTALL_HEADERS \
    config_gsgbase.h \
    graphicsOutputBase.I graphicsOutputBase.h \
    graphicsStateGuardianBase.h \
    shaderManagerBase.I shaderManagerBase.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_gsgbase
  #define LOCAL_LIBS \
    gsgbase

  #define SOURCES \
    test_gsgbase.cxx

#end test_bin_target
