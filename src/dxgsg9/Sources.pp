#define BUILD_DIRECTORY $[HAVE_DX9]

#define OTHER_LIBS \
   dtoolutil:c dtoolbase:c dtool:m

#define USE_PACKAGES dx9 cg cgdx9

#begin lib_target
  #define TARGET dxgsg9
  #define LOCAL_LIBS \
    gsgbase gobj display windisplay \
    putil linmath mathutil pnmimage event

  #define BUILDING_DLL BUILDING_PANDADX

  #define C++FLAGS -DUSE_GENERIC_DXERR_LIBRARY

  // build dxGraphicsStateGuardian separately since its so big

  #define SOURCES \
    dxGraphicsStateGuardian9.cxx \
    dxGraphicsDevice9.h \
    wdxGraphicsBuffer9.h wdxGraphicsBuffer9.I \
    wdxGraphicsPipe9.I wdxGraphicsPipe9.h \
    wdxGraphicsWindow9.I wdxGraphicsWindow9.h \
    dxgsg9base.h config_dxgsg9.h dxGraphicsStateGuardian9.I dxGraphicsStateGuardian9.h \
    dxVertexBufferContext9.h dxVertexBufferContext9.I \
    dxIndexBufferContext9.h dxIndexBufferContext9.I \
    dxTextureContext9.h dxTextureContext9.I \
    dxGeomMunger9.h dxGeomMunger9.I \
    dxGraphicsDevice9.h \
    dxOcclusionQueryContext9.h dxOcclusionQueryContext9.I \
    dxShaderContext9.h \
    vertexElementArray.h

  #define COMPOSITE_SOURCES \
    config_dxgsg9.cxx \
    dxVertexBufferContext9.cxx \
    dxIndexBufferContext9.cxx \
    dxTextureContext9.cxx \
    dxGeomMunger9.cxx \
    dxGraphicsDevice9.cxx \
    wdxGraphicsPipe9.cxx \
    wdxGraphicsWindow9.cxx \
    wdxGraphicsBuffer9.cxx \
    dxShaderContext9.cxx \
    dxOcclusionQueryContext9.cxx \
    vertexElementArray.cxx

#end lib_target
