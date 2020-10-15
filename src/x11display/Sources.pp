#define BUILD_DIRECTORY $[HAVE_X11]

#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES x11

#begin lib_target
  #define TARGET x11display
  #define LOCAL_LIBS display

  #define BUILDING_DLL BUILDING_PANDAX11

  #define SOURCES \
    config_x11display.cxx config_x11display.h \
    x11GraphicsPipe.I x11GraphicsPipe.cxx x11GraphicsPipe.h \
    x11GraphicsWindow.h x11GraphicsWindow.I x11GraphicsWindow.cxx

  #define INSTALL_HEADERS \
    x11GraphicsPipe.I x11GraphicsPipe.h \
    x11GraphicsWindow.I x11GraphicsWindow.h

#end lib_target
