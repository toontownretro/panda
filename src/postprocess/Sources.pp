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
    freezeFrame.h freezeFrame.I \
    fxaa.h \
    hdr.h hdr.I \
    motionBlur.h \
    postProcess.h \
    postProcessDefines.h \
    postProcessEffect.h \
    postProcessFinalOutput.h postProcessFinalOutput.I \
    postProcessPass.h \
    postProcessScenePass.h \
    postProcessSceneStage.h postProcessSceneStage.I \
    ssao.h \
    toneMapping.h toneMapping.I

  #define COMPOSITE_SOURCES \
    bloom.cxx \
    config_postprocess.cxx \
    freezeFrame.cxx \
    fxaa.cxx \
    hdr.cxx \
    motionBlur.cxx \
    postProcess.cxx \
    postProcessEffect.cxx \
    postProcessFinalOutput.cxx \
    postProcessPass.cxx \
    postProcessScenePass.cxx \
    postProcessSceneStage.cxx \
    ssao.cxx \
    toneMapping.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
