#define BUILD_DIRECTORY $[HAVE_FREETYPE]
#define USE_PACKAGES freetype

#define OTHER_LIBS p3interrogatedb:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m

#begin lib_target
  #define USE_PACKAGES freetype

  #define BUILDING_DLL BUILDING_PANDA_PNMTEXT

  #define TARGET p3pnmtext
  #define LOCAL_LIBS \
    p3parametrics p3pnmimage

  #define SOURCES \
    config_pnmtext.h \
    freetypeFace.h freetypeFace.I \
    freetypeFont.h freetypeFont.I \
    pnmTextGlyph.h pnmTextGlyph.I \
    pnmTextMaker.h pnmTextMaker.I

  #define COMPOSITE_SOURCES \
    config_pnmtext.cxx \
    freetypeFace.cxx \
    freetypeFont.cxx \
    pnmTextGlyph.cxx \
    pnmTextMaker.cxx

  #define INSTALL_HEADERS \
    config_pnmtext.h \
    freetypeFace.h freetypeFace.I \
    freetypeFont.h freetypeFont.I \
    pnmTextGlyph.h pnmTextGlyph.I \
    pnmTextMaker.h pnmTextMaker.I

  #define IGATESCAN all

#end lib_target
