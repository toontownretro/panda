#define BUILD_DIRECTORY $[HAVE_GLX]

#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES gl glx
#if $[LINK_IN_GLXGETPROCADDRESS]
  #define EXTRA_CDEFS LINK_IN_GLXGETPROCADDRESS
#endif

#begin lib_target
  #define TARGET glxdisplay
  #define LOCAL_LIBS \
    glgsg x11display

  #if $[HAVE_X11]
    #define C++FLAGS $[C++FLAGS] -DUSE_X11
  #endif

  #define SOURCES \
    config_glxdisplay.h \
    glxGraphicsBuffer.h glxGraphicsBuffer.I \
    glxGraphicsPipe.h glxGraphicsPipe.I \
    glxGraphicsPixmap.h glxGraphicsPixmap.I \
    glxGraphicsWindow.h glxGraphicsWindow.I \
    glxGraphicsStateGuardian.h glxGraphicsStateGuardian.I \
    posixGraphicsStateGuardian.h posixGraphicsStateGuardian.I \
    panda_glxext.h

  #define COMPOSITE_SOURCES \
    config_glxdisplay.cxx \
    glxGraphicsBuffer.cxx \
    glxGraphicsPipe.cxx \
    glxGraphicsPixmap.cxx \
    glxGraphicsWindow.cxx \
    glxGraphicsStateGuardian.cxx \
    posixGraphicsStateGuardian.cxx

  #define INSTALL_HEADERS \
    glxGraphicsBuffer.I glxGraphicsBuffer.h \
    glxGraphicsPipe.h glxGraphicsPipe.I \
    glxGraphicsWindow.h glxGraphicsWindow.I

#end lib_target
