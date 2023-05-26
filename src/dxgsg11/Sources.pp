#define BUILD_DIRECTORY $[WINDOWS_PLATFORM]

#begin lib_target
  #define TARGET dxgsg11

  #define LOCAL_LIBS \
    gsgbase gobj display windisplay \
    putil linmath mathutil pnmimage event
  #define OTHER_LIBS \
   dtoolutil:c dtoolbase:c dtool:m

  #define BUILDING_DLL BUILDING_PANDA_DXGSG11

  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] d3d11.lib DXGI.lib

  #define SOURCES \
    config_dxgsg11.h \
    dxBufferBase11.I dxBufferBase11.h \
    dxGeomMunger11.I dxGeomMunger11.h \
    dxGraphicsDevice11.I dxGraphicsDevice11.h \
    dxGraphicsStateGuardian11.I dxGraphicsStateGuardian11.h \
    dxIndexBufferContext11.I dxIndexBufferContext11.h \
    dxSamplerContext11.I dxSamplerContext11.h \
    dxShaderContext11.I dxShaderContext11.h \
    dxTextureContext11.I dxTextureContext11.h \
    dxVertexBufferContext11.I dxVertexBufferContext11.h \
    wdxGraphicsBuffer11.I wdxGraphicsBuffer11.h \
    wdxGraphicsPipe11.I wdxGraphicsPipe11.h \
    wdxGraphicsWindow11.I wdxGraphicsWindow11.h

  #define COMPOSITE_SOURCES \
    config_dxgsg11.cxx \
    dxBufferBase11.cxx \
    dxGeomMunger11.cxx \
    dxGraphicsDevice11.cxx \
    dxGraphicsStateGuardian11.cxx \
    dxIndexBufferContext11.cxx \
    dxSamplerContext11.cxx \
    dxShaderContext11.cxx \
    dxTextureContext11.cxx \
    dxVertexBufferContext11.cxx \
    wdxGraphicsBuffer11.cxx \
    wdxGraphicsPipe11.cxx \
    wdxGraphicsWindow11.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

#end lib_target

