#define BUILD_DIRECTORY $[and $[IS_OSX],$[HAVE_GL],$[HAVE_COCOA]]

#define OTHER_LIBS p3dtoolutil:c p3dtoolbase:c p3dtool:m

#define OSX_SYS_FRAMEWORKS ApplicationServices AppKit Carbon CoreVideo

#begin lib_target
  #define TARGET p3cocoadisplay
  #define LOCAL_LIBS \
    p3display p3putil p3glgsg

  #define BUILDING_DLL BUILDING_PANDA_COCOADISPLAY

  #define INSTALL_HEADERS \
     config_cocoadisplay.h \
     cocoaGraphicsBuffer.h cocoaGraphicsBuffer.I \
     cocoaGraphicsPipe.h cocoaGraphicsPipe.I \
     cocoaGraphicsWindow.h cocoaGraphicsWindow.I \
     cocoaGraphicsStateGuardian.h cocoaGraphicsStateGuardian.I \
     cocoaPandaApp.h \
     cocoaPandaView.h \
     cocoaPandaWindow.h \
     cocoaPandaWindowDelegate.h \
     cocoaPandaAppDelegate.h

  #define COMPOSITE_SOURCES \
    config_cocoadisplay.mm \
    cocoaGraphicsBuffer.mm \
    cocoaGraphicsPipe.mm \
    cocoaGraphicsStateGuardian.mm \
    cocoaGraphicsWindow.mm \
    cocoaPandaApp.mm \
    cocoaPandaView.mm \
    cocoaPandaWindow.mm \
    cocoaPandaWindowDelegate.mm \
    cocoaPandaAppDelegate.mm

  #define SOURCES \
    $[INSTALL_HEADERS]

#end lib_target
