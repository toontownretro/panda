#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

// Most of the files here are not actually compiled into anything;
// they're just included by various other directories.

#begin interface_target
  #define TARGET glstuff
  #define LOCAL_LIBS \
    gsgbase gobj display \
    putil linmath mathutil pnmimage

  #if $[OSX_PLATFORM]
    // We acknowledge Apple's hostility to open standards so that the SDK won't
    // bother our users about it.
    #define C++FLAGS -DGL_SILENCE_DEPRECATION
  #endif

  #define HEADERS \
    glBufferContext_src.I \
    glBufferContext_src.h \
    glGeomContext_src.I \
    glGeomContext_src.h \
    glGeomMunger_src.I \
    glGeomMunger_src.h \
    glGraphicsStateGuardian_src.I \
    glGraphicsStateGuardian_src.h \
    glGraphicsBuffer_src.I \
    glGraphicsBuffer_src.h \
    glImmediateModeSender_src.I \
    glImmediateModeSender_src.h \
    glIndexBufferContext_src.I \
    glIndexBufferContext_src.h \
    glOcclusionQueryContext_src.I \
    glOcclusionQueryContext_src.h \
    glShaderContext_src.I \
    glShaderContext_src.h \
    glTextureContext_src.I \
    glTextureContext_src.h \
    glVAOs_src.h \
    glVertexBufferContext_src.I \
    glVertexBufferContext_src.h \
    glmisc_src.h \
    glstuff_src.h \
    glstuff_undef_src.h \
    panda_glext.h

  #define SOURCES \
    $[HEADERS] \
    glBufferContext_src.cxx \
    glGeomContext_src.cxx \
    glGeomMunger_src.cxx \
    glGraphicsStateGuardian_src.cxx \
    glGraphicsBuffer_src.cxx \
    glImmediateModeSender_src.cxx \
    glIndexBufferContext_src.cxx \
    glOcclusionQueryContext_src.cxx \
    glShaderContext_src.cxx \
    glTextureContext_src.cxx \
    glVertexBufferContext_src.cxx \
    glmisc_src.cxx \
    glstuff_src.cxx

  #define INSTALL_HEADERS \
     $[HEADERS]

#end interface_target
