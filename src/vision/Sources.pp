#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#define USE_PACKAGES opencv artoolkit

#define BUILDING_DLL BUILDING_VISION

#begin lib_target
  #define TARGET vision
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil audio movies

  #define BUILDNG_DLL BUILDING_PANDA_VISION

  #define SOURCES \
    arToolKit.I arToolKit.h \
    config_vision.h \
    openCVTexture.I openCVTexture.h \
    webcamVideoCursorOpenCV.h \
    webcamVideoCursorV4L.h \
    webcamVideo.h webcamVideo.I \
    webcamVideoOpenCV.h \
    webcamVideoV4L.h

  #define COMPOSITE_SOURCES \
    arToolKit.cxx \
    config_vision.cxx \
    openCVTexture.cxx \
    webcamVideo.cxx \
    webcamVideoDS.cxx \
    webcamVideoCursorOpenCV.cxx \
    webcamVideoOpenCV.cxx \
    webcamVideoCursorV4L.cxx \
    webcamVideoV4L.cxx

  #define INSTALL_HEADERS \
    arToolKit.I arToolKit.h \
    config_vision.h \
    openCVTexture.I openCVTexture.h \
    webcamVideoCursorOpenCV.h \
    webcamVideoCursorV4L.h \
    webcamVideo.h webcamVideo.I \
    webcamVideoOpenCV.h \
    webcamVideoV4L.h

  #define IGATESCAN all

#end lib_target
