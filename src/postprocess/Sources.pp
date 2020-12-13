#define LOCAL_LIBS pgraph gobj putil grutil display
#define OTHER_LIBS dtool:m dtoolbase:c dtoolutil:c \
  prc

#begin lib_target
  #define TARGET postprocess
  #define BUILDING_DLL BUILDING_PANDA_POSTPROCESS

  #define SOURCES \
    bloom.h \
    blurPasses.h \
    config_postprocess.h \
    fxaa.h \
    hdr.h hdr.I \
    motionBlur.h \
    postProcess.h \
    postProcessDefines.h \
    postProcessEffect.h \
    postProcessPass.h \
    postProcessScenePass.h \
    ssao.h

  #define COMPOSITE_SOURCES \
    bloom.cxx \
    config_postprocess.cxx \
    fxaa.cxx \
    hdr.cxx \
    motionBlur.cxx \
    postProcess.cxx \
    postProcessEffect.cxx \
    postProcessPass.cxx \
    postProcessScenePass.cxx \
    ssao.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
