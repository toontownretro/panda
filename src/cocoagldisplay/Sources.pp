#define BUILD_DIRECTORY $[and $[IS_OSX],$[HAVE_GL],$[HAVE_COCOA]]

#begin lib_target
  #define TARGET cocoagldisplay

  #define LOCAL_LIBS glgsg cocoadisplay

  #define BUILDING_DLL BUILDING_PANDA_COCOAGLDISPLAY

  #define SOURCES \
    cocoaGLGraphicsBuffer.h cocoaGLGraphicsBuffer.I \
    cocoaGLGraphicsPipe.h cocoaGLGraphicsPipe.I \
    cocoaGLGraphicsStateGuardian.h cocoaGLGraphicsStateGuardian.I \
    cocoaGLGraphicsWindow.h cocoaGLGraphicsWindow.I \
    config_cocoagldisplay.h

  #define COMPOSITE_SOURCES \
    cocoaGLGraphicsBuffer.mm \
    cocoaGLGraphicsPipe.mm \
    cocoaGLGraphicsStateGuardian.mm \
    cocoaGLGraphicsWindow.mm \
    config_cocoagldisplay.mm

#end lib_target
