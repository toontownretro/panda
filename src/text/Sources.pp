#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define USE_PACKAGES harfbuzz

  #define TARGET text
  #define LOCAL_LIBS \
    putil gobj pgraph linmath \
    pnmtext pnmimage gsgbase mathutil \
    parametrics

  #define BUILDING_DLL BUILDING_PANDA_TEXT

  #define SOURCES \
    config_text.h \
    default_font.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    fontPool.I fontPool.h \
    geomTextGlyph.I geomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    textAssembler.I textAssembler.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    textGraphic.I textGraphic.h \
    textNode.I textNode.h \
    textProperties.I textProperties.h \
    textPropertiesManager.I textPropertiesManager.h \
    cmss12.bam_src.c cmss12.bam.pz_src.c persans.ttf_src.c

  #define COMPOSITE_SOURCES \
    config_text.cxx \
    default_font.cxx \
    dynamicTextFont.cxx \
    dynamicTextGlyph.cxx \
    dynamicTextPage.cxx \
    fontPool.cxx \
    geomTextGlyph.cxx \
    staticTextFont.cxx \
    textAssembler.cxx \
    textFont.cxx textGlyph.cxx \
    textGraphic.cxx \
    textNode.cxx \
    textProperties.cxx \
    textPropertiesManager.cxx

  #define INSTALL_HEADERS \
    config_text.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    fontPool.I fontPool.h \
    geomTextGlyph.I geomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    textAssembler.I textAssembler.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    textGraphic.I textGraphic.h \
    textNode.I textNode.h \
    textProperties.I textProperties.h \
    textPropertiesManager.I textPropertiesManager.h


  #define IGATESCAN all

#end lib_target
