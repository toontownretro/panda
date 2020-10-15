#define BUILD_DIRECTORY $[HAVE_WGL]

#define USE_PACKAGES gl cg


#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET wgldisplay
  #define LOCAL_LIBS \
    display putil windisplay glgsg

  #define BUILDING_DLL BUILDING_PANDA_WGLDISPLAY

  #define INSTALL_HEADERS \
    config_wgldisplay.h \
    wglGraphicsBuffer.I wglGraphicsBuffer.h \
    wglGraphicsPipe.I wglGraphicsPipe.h \
    wglGraphicsStateGuardian.I wglGraphicsStateGuardian.h \
    wglGraphicsWindow.I wglGraphicsWindow.h \
    wglext.h

  #define COMPOSITE_SOURCES \
    config_wgldisplay.cxx \
    wglGraphicsBuffer.cxx \
    wglGraphicsPipe.cxx \
    wglGraphicsStateGuardian.cxx \
    wglGraphicsWindow.cxx

  #define SOURCES \
    $[INSTALL_HEADERS]


#end lib_target
