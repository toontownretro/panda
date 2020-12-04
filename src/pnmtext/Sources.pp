#define BUILD_DIRECTORY $[HAVE_FREETYPE]

#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#define USE_PACKAGES freetype harfbuzz

#begin lib_target

  #define BUILDING_DLL BUILDING_PANDA_PNMTEXT

  #define TARGET pnmtext
  #define LOCAL_LIBS \
    parametrics pnmimage

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
