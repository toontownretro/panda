#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
#define USE_PACKAGES zlib

#begin lib_target
  #define TARGET pnmimage
  #define LOCAL_LIBS \
    linmath putil express mathutil

  #define BUILDING_DLL BUILDING_PANDA_PNMIMAGE

  #define SOURCES \
     config_pnmimage.h \
     convert_srgb.I convert_srgb.h \
     convert_srgb_sse2.cxx \
     pfmFile.I pfmFile.h \
     pnmbitio.h \
     pnmBrush.h pnmBrush.I \
     pnmFileType.h pnmFileTypeRegistry.h pnmImage.I  \
     pnmImage.h pnmImageHeader.I pnmImageHeader.h  \
     pnmPainter.h pnmPainter.I \
     pnmReader.I \
     pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h \
     pnmReaderEmscripten.h \
     ppmcmap.h

  #define COMPOSITE_SOURCES \
     config_pnmimage.cxx \
     convert_srgb.cxx \
     pfmFile.cxx \
     pnm-image-filter.cxx \
     pnmbitio.cxx \
     pnmBrush.cxx \
     pnmFileType.cxx  \
     pnmFileTypeRegistry.cxx pnmImage.cxx pnmImageHeader.cxx  \
     pnmPainter.cxx \
     pnmReader.cxx pnmWriter.cxx pnmimage_base.cxx \
     pnmReaderEmscripten.cxx \
     ppmcmap.cxx

  #define INSTALL_HEADERS \
     config_pnmimage.h \
     convert_srgb.I convert_srgb.h \
     pfmFile.I pfmFile.h \
     pnmBrush.h pnmBrush.I \
     pnmFileType.h pnmFileTypeRegistry.h pnmImage.I \
     pnmImage.h pnmImageHeader.I pnmImageHeader.h \
     pnmPainter.h pnmPainter.I \
     pnmReader.I \
     pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h \
     pnmReaderEmscripten.h

  #define IGATESCAN all

  #define IGATEEXT \
   pfmFile_ext.cxx \
   pfmFile_ext.h

#end lib_target
