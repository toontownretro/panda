#define BUILD_DIRECTORY $[and $[HAVE_EGL]]

#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET egldisplay_gles1
  #define BUILD_TARGET $[HAVE_GLES]
  #define USE_PACKAGES gles egl x11
  #define EXTRA_CDEFS OPENGLES_1
  #define LOCAL_LIBS \
    glesgsg x11display

  #if $[HAVE_X11]
    #define C++FLAGS $[C++FLAGS] -DUSE_X11
  #else
    #define C++FLAGS $[C++FLAGS] -DEGL_NO_X11
  #endif

  #define BUILDING_DLL BUILDING_PANDAGLES

  #define SOURCES \
    config_egldisplay.cxx config_egldisplay.h \
    eglGraphicsBuffer.h eglGraphicsBuffer.cxx \
    eglGraphicsPipe.I eglGraphicsPipe.cxx eglGraphicsPipe.h \
    eglGraphicsPixmap.h eglGraphicsPixmap.cxx \
    eglGraphicsWindow.h eglGraphicsWindow.cxx \
    eglGraphicsStateGuardian.h eglGraphicsStateGuardian.cxx

  #define INSTALL_HEADERS \
    eglGraphicsBuffer.h eglGraphicsPixmap.h \
    eglGraphicsPipe.I eglGraphicsPipe.h \
    eglGraphicsWindow.I eglGraphicsWindow.h

#end lib_target

#begin lib_target
  #define TARGET egldisplay_gles2
  #define BUILD_TARGET $[HAVE_GLES2]
  #define USE_PACKAGES gles2 egl x11
  #define EXTRA_CDEFS OPENGLES_2
  #define LOCAL_LIBS \
    gles2gsg x11display

  #if $[HAVE_X11]
    #define C++FLAGS $[C++FLAGS] -DUSE_X11
  #else
    #define C++FLAGS $[C++FLAGS] -DEGL_NO_X11
  #endif

  #define BUILDING_DLL BUILDING_PANDAGLES2

  #define SOURCES \
    config_egldisplay.cxx config_egldisplay.h \
    eglGraphicsBuffer.h eglGraphicsBuffer.cxx \
    eglGraphicsPipe.I eglGraphicsPipe.cxx eglGraphicsPipe.h \
    eglGraphicsPixmap.h eglGraphicsPixmap.cxx \
    eglGraphicsWindow.h eglGraphicsWindow.cxx \
    eglGraphicsStateGuardian.h eglGraphicsStateGuardian.cxx

  #define INSTALL_HEADERS \
    eglGraphicsBuffer.h eglGraphicsPixmap.h \
    eglGraphicsPipe.I eglGraphicsPipe.h \
    eglGraphicsWindow.I eglGraphicsWindow.h

#end lib_target
