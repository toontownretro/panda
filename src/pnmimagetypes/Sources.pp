#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
#define USE_PACKAGES jpeg tiff png openexr

#begin lib_target
  #define TARGET pnmimagetypes
  #define LOCAL_LIBS \
    pnmimage

  #define BUILDING_DLL BUILDING_PANDA_PNMIMAGETYPES

  #define SOURCES  \
     config_pnmimagetypes.h \
     pnmFileTypeBMP.h  \
     pnmFileTypeEXR.h  \
     pnmFileTypeIMG.h  \
     pnmFileTypePNG.h \
     pnmFileTypePNM.h \
     pnmFileTypePfm.h \
     pnmFileTypeSGI.h pnmFileTypeSoftImage.h  \
     pnmFileTypeTGA.h \
     pnmFileTypeTIFF.h \
     pnmFileTypeJPG.h \
     sgi.h

  #define COMPOSITE_SOURCES  \
     config_pnmimagetypes.cxx  \
     pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx  \
     pnmFileTypeBMP.cxx \
     pnmFileTypeEXR.cxx \
     pnmFileTypeIMG.cxx \
     pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx \
     pnmFileTypePNG.cxx \
     pnmFileTypePNM.cxx \
     pnmFileTypePfm.cxx \
     pnmFileTypeSGI.cxx \
     pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx  \
     pnmFileTypeSoftImage.cxx \
     pnmFileTypeTIFF.cxx \
     pnmFileTypeTGA.cxx

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target
