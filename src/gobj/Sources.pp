#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
//#define OSX_SYS_LIBS mx
#define USE_PACKAGES zlib squish glslang spirv_tools

#begin lib_target
  #define TARGET gobj
  #define LOCAL_LIBS \
    pstatclient event linmath mathutil pnmimage gsgbase putil

  #define BUILDING_DLL BUILDING_PANDA_GOBJ

  #define SOURCES \
    adaptiveLru.I adaptiveLru.h \
    animateVerticesRequest.I animateVerticesRequest.h \
    bufferContext.I bufferContext.h \
    bufferContextChain.I bufferContextChain.h \
    bufferResidencyTracker.I bufferResidencyTracker.h \
    cg_preamble.cxx \
    config_gobj.h \
    geom.h geom.I \
    geomContext.I geomContext.h \
    geomEnums.h \
    geomMunger.h geomMunger.I \
    geomPrimitive.h geomPrimitive.I \
    geomPatches.h \
    geomTriangles.h \
    geomTrianglesAdjacency.h \
    geomTristrips.h \
    geomTristripsAdjacency.h \
    geomTrifans.h \
    geomLines.h \
    geomLinesAdjacency.h \
    geomLinestrips.h \
    geomLinestripsAdjacency.h \
    geomPoints.h \
    geomVertexArrayData.h geomVertexArrayData.I \
    geomVertexArrayFormat.h geomVertexArrayFormat.I \
    geomCacheEntry.h geomCacheEntry.I \
    geomCacheManager.h geomCacheManager.I \
    geomVertexAnimationSpec.h geomVertexAnimationSpec.I \
    geomVertexData.h geomVertexData.I \
    geomVertexColumn.h geomVertexColumn.I \
    geomVertexFormat.h geomVertexFormat.I \
    geomVertexReader.h geomVertexReader.I \
    geomVertexRewriter.h geomVertexRewriter.I \
    geomVertexWriter.h geomVertexWriter.I \
    GLSL.std.450.h \
    indexBufferContext.I indexBufferContext.h \
    internalName.I internalName.h \
    lens.h lens.I \
    matrixLens.I matrixLens.h \
    occlusionQueryContext.I occlusionQueryContext.h \
    orthographicLens.I orthographicLens.h perspectiveLens.I  \
    paramTexture.I paramTexture.h \
    perspectiveLens.h \
    preparedGraphicsObjects.I preparedGraphicsObjects.h \
    queryContext.I queryContext.h \
    samplerContext.I samplerContext.h \
    samplerState.I samplerState.h \
    savedContext.I savedContext.h \
    shader.I shader.h \
    shaderBuffer.h shaderBuffer.I \
    shaderCompiler.h shaderCompiler.I \
    shaderCompilerGlslang.h \
    shaderCompilerGlslPreProc.h \
    shaderCompilerRegistry.h \
    shaderContext.h shaderContext.I \
    shaderModule.h shaderModule.I \
    shaderModuleGlsl.h \
    shaderModuleSpirV.h shaderModuleSpirV.I \
    shaderType.h shaderType.I \
    simpleAllocator.h simpleAllocator.I \
    simpleLru.h simpleLru.I \
    sliderTable.I sliderTable.h \
    spirv.hpp \
    texture.I texture.h \
    textureCollection.I textureCollection.h \
    textureContext.I textureContext.h \
    texturePeeker.I texturePeeker.h \
    texturePool.I texturePool.h \
    texturePoolFilter.I texturePoolFilter.h \
    textureReloadRequest.I textureReloadRequest.h \
    textureStage.I textureStage.h \
    textureStagePool.I textureStagePool.h \
    timerQueryContext.I timerQueryContext.h \
    transformBlend.I transformBlend.h \
    transformBlendTable.I transformBlendTable.h \
    transformTable.I transformTable.h \
    userVertexSlider.I userVertexSlider.h \
    userVertexTransform.I userVertexTransform.h \
    vertexBufferContext.I vertexBufferContext.h \
    vertexDataBlock.I vertexDataBlock.h \
    vertexDataBook.I vertexDataBook.h \
    vertexDataBuffer.I vertexDataBuffer.h \
    vertexDataPage.I vertexDataPage.h \
    vertexDataSaveFile.I vertexDataSaveFile.h \
    vertexSlider.I vertexSlider.h \
    vertexTransform.I vertexTransform.h \
    videoTexture.I videoTexture.h

  #define COMPOSITE_SOURCES \
    adaptiveLru.cxx \
    animateVerticesRequest.cxx \
    bufferContext.cxx \
    bufferContextChain.cxx \
    bufferResidencyTracker.cxx \
    config_gobj.cxx \
    geomContext.cxx \
    geom.cxx \
    geomEnums.cxx \
    geomMunger.cxx \
    geomPrimitive.cxx \
    geomPatches.cxx \
    geomTriangles.cxx \
    geomTrianglesAdjacency.cxx \
    geomTristrips.cxx \
    geomTristripsAdjacency.cxx \
    geomTrifans.cxx \
    geomLines.cxx \
    geomLinesAdjacency.cxx \
    geomLinestrips.cxx \
    geomLinestripsAdjacency.cxx \
    geomPoints.cxx \
    geomVertexArrayData.cxx \
    geomVertexArrayFormat.cxx \
    geomCacheEntry.cxx \
    geomCacheManager.cxx \
    geomVertexAnimationSpec.cxx \
    geomVertexData.cxx \
    geomVertexColumn.cxx \
    geomVertexFormat.cxx \
    geomVertexReader.cxx \
    geomVertexRewriter.cxx \
    geomVertexWriter.cxx \
    indexBufferContext.cxx \
    internalName.cxx \
    lens.cxx  \
    matrixLens.cxx \
    occlusionQueryContext.cxx \
    orthographicLens.cxx  \
    paramTexture.cxx \
    perspectiveLens.cxx \
    preparedGraphicsObjects.cxx \
    queryContext.cxx \
    samplerContext.cxx \
    samplerState.cxx \
    savedContext.cxx \
    shader.cxx \
    shaderBuffer.cxx \
    shaderCompiler.cxx \
    shaderCompilerGlslang.cxx \
    shaderCompilerGlslPreProc.cxx \
    shaderCompilerRegistry.cxx \
    shaderContext.cxx \
    shaderModule.cxx \
    shaderModuleGlsl.cxx \
    shaderModuleSpirV.cxx \
    shaderType.cxx \
    simpleAllocator.cxx \
    simpleLru.cxx \
    sliderTable.cxx \
    texture.cxx \
    textureCollection.cxx \
    textureContext.cxx \
    texturePeeker.cxx \
    texturePool.cxx \
    texturePoolFilter.cxx \
    textureReloadRequest.cxx \
    textureStage.cxx \
    textureStagePool.cxx \
    timerQueryContext.cxx \
    transformBlend.cxx \
    transformBlendTable.cxx \
    transformTable.cxx \
    userVertexSlider.cxx \
    userVertexTransform.cxx \
    vertexBufferContext.cxx \
    vertexDataBlock.cxx \
    vertexDataBook.cxx \
    vertexDataBuffer.cxx \
    vertexDataPage.cxx \
    vertexDataSaveFile.cxx \
    vertexSlider.cxx \
    vertexTransform.cxx \
    videoTexture.cxx

  #define INSTALL_HEADERS \
    adaptiveLru.I adaptiveLru.h \
    animateVerticesRequest.I animateVerticesRequest.h \
    bufferContext.I bufferContext.h \
    bufferContextChain.I bufferContextChain.h \
    bufferResidencyTracker.I bufferResidencyTracker.h \
    config_gobj.h \
    geom.I geom.h \
    textureContext.I textureContext.h \
    geom.h geom.I \
    geomContext.I geomContext.h \
    geomEnums.h \
    geomMunger.h geomMunger.I \
    geomPrimitive.h geomPrimitive.I \
    geomPatches.h \
    geomTriangles.h \
    geomTrianglesAdjacency.h \
    geomTristrips.h \
    geomTristripsAdjacency.h \
    geomTrifans.h \
    geomLines.h \
    geomLinesAdjacency.h \
    geomLinestrips.h \
    geomLinestripsAdjacency.h \
    geomPoints.h \
    geomVertexArrayData.h geomVertexArrayData.I \
    geomVertexArrayFormat.h geomVertexArrayFormat.I \
    geomCacheEntry.h geomCacheEntry.I \
    geomCacheManager.h geomCacheManager.I \
    geomVertexAnimationSpec.h geomVertexAnimationSpec.I \
    geomVertexData.h geomVertexData.I \
    geomVertexColumn.h geomVertexColumn.I \
    geomVertexFormat.h geomVertexFormat.I \
    geomVertexReader.h geomVertexReader.I \
    geomVertexRewriter.h geomVertexRewriter.I \
    geomVertexWriter.h geomVertexWriter.I \
    indexBufferContext.I indexBufferContext.h \
    internalName.I internalName.h \
    lens.h lens.I \
    matrixLens.I matrixLens.h \
    occlusionQueryContext.I occlusionQueryContext.h \
    orthographicLens.I orthographicLens.h \
    paramTexture.I paramTexture.h \
    perspectiveLens.I perspectiveLens.h \
    preparedGraphicsObjects.I preparedGraphicsObjects.h \
    queryContext.I queryContext.h \
    samplerContext.I samplerContext.h \
    samplerState.I samplerState.h \
    savedContext.I savedContext.h \
    shader.I shader.h \
    shaderBuffer.h shaderBuffer.I \
    shaderCompiler.h shaderCompiler.I \
    shaderCompilerGlslang.h \
    shaderCompilerGlslPreProc.h \
    shaderCompilerRegistry.h \
    shaderContext.h shaderContext.I \
    shaderModule.h shaderModule.I \
    shaderModuleGlsl.h \
    shaderModuleSpirV.h shaderModuleSpirV.I \
    shaderType.h shaderType.I \
    simpleAllocator.h simpleAllocator.I \
    simpleLru.h simpleLru.I \
    sliderTable.I sliderTable.h \
    spirv.hpp \
    texture.I texture.h \
    textureCollection.I textureCollection.h \
    textureContext.I textureContext.h \
    texturePeeker.I texturePeeker.h \
    texturePool.I texturePool.h \
    texturePoolFilter.I texturePoolFilter.h \
    textureReloadRequest.I textureReloadRequest.h \
    textureStage.I textureStage.h \
    textureStagePool.I textureStagePool.h \
    timerQueryContext.I timerQueryContext.h \
    transformBlend.I transformBlend.h \
    transformBlendTable.I transformBlendTable.h \
    transformTable.I transformTable.h \
    userVertexSlider.I userVertexSlider.h \
    userVertexTransform.I userVertexTransform.h \
    vertexBufferContext.I vertexBufferContext.h \
    vertexDataBlock.I vertexDataBlock.h \
    vertexDataBook.I vertexDataBook.h \
    vertexDataBuffer.I vertexDataBuffer.h \
    vertexDataPage.I vertexDataPage.h \
    vertexDataSaveFile.I vertexDataSaveFile.h \
    vertexSlider.I vertexSlider.h \
    vertexTransform.I vertexTransform.h \
    videoTexture.I videoTexture.h

  #define IGATESCAN all

  #define IGATEEXT \
    internalName_ext.cxx \
    internalName_ext.h \
    geomVertexArrayData_ext.cxx \
    geomVertexArrayData_ext.h \
    texture_ext.cxx \
    texture_ext.h \
    textureCollection_ext.cxx \
    textureCollection_ext.h \
    texturePool_ext.cxx \
    texturePool_ext.h \
    pythonTexturePoolFilter.cxx \
    pythonTexturePoolFilter.h

#end lib_target

#concatcxx $[PATH]/cg_preamble.cxx, cg_preamble, $[PATH]/cg_preamble.hlsl
