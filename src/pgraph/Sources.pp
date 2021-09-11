#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
#define LOCAL_LIBS \
    event gsgbase gobj putil linmath \
    downloader express pandabase pstatclient material

#begin lib_target
  #define TARGET pgraph

  // This directory is too big to combine into a single composite
  // file--at least on Windows.
  //#define DONT_COMPOSITE $[WINDOWS_PLATFORM]

  #define BUILDING_DLL BUILDING_PANDA_PGRAPH

  #define SOURCES \
    accumulatedAttribs.I accumulatedAttribs.h \
    alphaTestAttrib.I alphaTestAttrib.h \
    antialiasAttrib.I antialiasAttrib.h \
    attribNodeRegistry.I attribNodeRegistry.h \
    auxSceneData.I auxSceneData.h \
    auxBitplaneAttrib.I auxBitplaneAttrib.h \
    bamFile.I bamFile.h \
    billboardEffect.I billboardEffect.h \
    cacheStats.I cacheStats.h \
    camera.I camera.h \
    clipPlaneAttrib.I clipPlaneAttrib.h \
    colorAttrib.I colorAttrib.h \
    colorBlendAttrib.I colorBlendAttrib.h \
    colorScaleAttrib.I colorScaleAttrib.h \
    colorWriteAttrib.I colorWriteAttrib.h \
    compassEffect.I compassEffect.h \
    config_pgraph.h \
    cullBin.I cullBin.h \
    cullBinEnums.h \
    cullBinAttrib.I cullBinAttrib.h \
    cullBinManager.I cullBinManager.h \
    cullFaceAttrib.I cullFaceAttrib.h \
    cullHandler.I cullHandler.h \
    cullLights.I cullLights.h \
    cullPlanes.I cullPlanes.h \
    cullResult.I cullResult.h \
    cullTraverser.I cullTraverser.h \
    cullTraverserData.I cullTraverserData.h \
    cullableObject.I cullableObject.h \
    decalEffect.I decalEffect.h \
    depthBiasAttrib.I depthBiasAttrib.h \
    depthOffsetAttrib.I depthOffsetAttrib.h \
    depthTestAttrib.I depthTestAttrib.h \
    depthWriteAttrib.I depthWriteAttrib.h \
    eventStorePandaNode.I eventStorePandaNode.h \
    findApproxLevelEntry.I findApproxLevelEntry.h \
    findApproxPath.I findApproxPath.h \
    fog.I fog.h \
    fogAttrib.I fogAttrib.h \
    geomDrawCallbackData.I geomDrawCallbackData.h \
    geomNode.I geomNode.h \
    geomTransformer.I geomTransformer.h \
    instanceList.I instanceList.h \
    instancedNode.I instancedNode.h \
    internalNameCollection.I internalNameCollection.h \
    lensNode.I lensNode.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightRampAttrib.I lightRampAttrib.h \
    loader.I loader.h  \
    loaderFileType.h \
    loaderFileTypeBam.h  \
    loaderFileTypeRegistry.h \
    logicOpAttrib.I logicOpAttrib.h \
    materialAttrib.I materialAttrib.h \
    materialCollection.I materialCollection.h \
    modelFlattenRequest.I modelFlattenRequest.h \
    modelLoadRequest.I modelLoadRequest.h \
    modelSaveRequest.I modelSaveRequest.h \
    modelNode.I modelNode.h \
    modelPool.I modelPool.h \
    modelRoot.I modelRoot.h \
    nodePath.I nodePath.h nodePath.cxx \
    nodePathCollection.I nodePathCollection.h \
    nodePathComponent.I nodePathComponent.h \
    occluderEffect.I occluderEffect.h \
    occluderNode.I occluderNode.h \
    pandaNode.I pandaNode.h \
    pandaNodeChain.I pandaNodeChain.h \
    paramNodePath.I paramNodePath.h \
    planeNode.I planeNode.h \
    polylightEffect.I polylightEffect.h \
    polylightNode.I polylightNode.h \
    portalNode.I portalNode.h \
    portalClipper.I portalClipper.h \
    renderAttrib.I renderAttrib.h \
    renderAttribRegistry.I renderAttribRegistry.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    rescaleNormalAttrib.I rescaleNormalAttrib.h \
    sceneGraphReducer.I sceneGraphReducer.h \
    sceneSetup.I sceneSetup.h \
    scissorAttrib.I scissorAttrib.h \
    scissorEffect.I scissorEffect.h \
    shadeModelAttrib.I shadeModelAttrib.h \
    shaderAttrib.I shaderAttrib.h \
    shaderInput.I shaderInput.h \
    shaderPool.I shaderPool.h \
    showBoundsEffect.I showBoundsEffect.h \
    stateMunger.I stateMunger.h \
    stencilAttrib.I stencilAttrib.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    texProjectorEffect.I texProjectorEffect.h \
    textureAttrib.I textureAttrib.h \
    texGenAttrib.I texGenAttrib.h \
    textureStageCollection.I textureStageCollection.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h \
    weakNodePath.I weakNodePath.h \
    workingNodePath.I workingNodePath.h \

  #define COMPOSITE_SOURCES \
    accumulatedAttribs.cxx \
    alphaTestAttrib.cxx \
    antialiasAttrib.cxx \
    attribNodeRegistry.cxx \
    auxBitplaneAttrib.cxx \
    auxSceneData.cxx \
    bamFile.cxx \
    billboardEffect.cxx \
    cacheStats.cxx \
    camera.cxx \
    clipPlaneAttrib.cxx \
    colorAttrib.cxx \
    colorBlendAttrib.cxx \
    colorScaleAttrib.cxx \
    colorWriteAttrib.cxx \
    compassEffect.cxx \
    config_pgraph.cxx \
    cullBin.cxx \
    cullBinAttrib.cxx \
    cullBinManager.cxx \
    cullFaceAttrib.cxx \
    cullHandler.cxx \
    cullLights.cxx \
    cullPlanes.cxx \
    cullResult.cxx \
    cullTraverser.cxx \
    cullTraverserData.cxx \
    cullableObject.cxx \
    decalEffect.cxx \
    depthBiasAttrib.cxx \
    depthOffsetAttrib.cxx \
    depthTestAttrib.cxx \
    depthWriteAttrib.cxx \
    eventStorePandaNode.cxx \
    findApproxLevelEntry.cxx \
    findApproxPath.cxx \
    fog.cxx \
    fogAttrib.cxx \
    geomDrawCallbackData.cxx \
    geomNode.cxx \
    geomTransformer.cxx \
    instanceList.cxx \
    instancedNode.cxx \
    internalNameCollection.cxx \
    lensNode.cxx \
    light.cxx \
    lightAttrib.cxx \
    lightRampAttrib.cxx \
    loader.cxx \
    loaderFileType.cxx  \
    loaderFileTypeBam.cxx \
    loaderFileTypeRegistry.cxx  \
    logicOpAttrib.cxx \
    materialAttrib.cxx \
    materialCollection.cxx \
    modelFlattenRequest.cxx \
    modelLoadRequest.cxx \
    modelSaveRequest.cxx \
    modelNode.cxx \
    modelPool.cxx \
    modelRoot.cxx \
    nodePathCollection.cxx \
    nodePathComponent.cxx \
    occluderEffect.cxx \
    occluderNode.cxx \
    pandaNode.cxx \
    pandaNodeChain.cxx \
    paramNodePath.cxx \
    planeNode.cxx \
    polylightEffect.cxx \
    polylightNode.cxx \
    portalNode.cxx \
    portalClipper.cxx \
    renderAttrib.cxx \
    renderAttribRegistry.cxx \
    renderEffect.cxx \
    renderEffects.cxx \
    renderModeAttrib.cxx \
    renderState.cxx \
    rescaleNormalAttrib.cxx \
    sceneGraphReducer.cxx \
    sceneSetup.cxx \
    scissorAttrib.cxx \
    scissorEffect.cxx \
    shadeModelAttrib.cxx \
    shaderAttrib.cxx \
    shaderInput.cxx \
    shaderPool.cxx \
    showBoundsEffect.cxx \
    stateMunger.cxx \
    stencilAttrib.cxx \
    texMatrixAttrib.cxx \
    texProjectorEffect.cxx \
    textureAttrib.cxx \
    texGenAttrib.cxx \
    textureStageCollection.cxx \
    transformState.cxx \
    transparencyAttrib.cxx \
    weakNodePath.cxx \
    workingNodePath.cxx

  #define INSTALL_HEADERS \
    accumulatedAttribs.I accumulatedAttribs.h \
    alphaTestAttrib.I alphaTestAttrib.h \
    antialiasAttrib.I antialiasAttrib.h \
    attribNodeRegistry.I attribNodeRegistry.h \
    auxSceneData.I auxSceneData.h \
    auxBitplaneAttrib.I auxBitplaneAttrib.h \
    bamFile.I bamFile.h \
    billboardEffect.I billboardEffect.h \
    cacheStats.I cacheStats.h \
    camera.I camera.h \
    clipPlaneAttrib.I clipPlaneAttrib.h \
    colorAttrib.I colorAttrib.h \
    colorBlendAttrib.I colorBlendAttrib.h \
    colorScaleAttrib.I colorScaleAttrib.h \
    colorWriteAttrib.I colorWriteAttrib.h \
    compassEffect.I compassEffect.h \
    config_pgraph.h \
    cullBin.I cullBin.h \
    cullBinEnums.h \
    cullBinAttrib.I cullBinAttrib.h \
    cullBinManager.I cullBinManager.h \
    cullFaceAttrib.I cullFaceAttrib.h \
    cullHandler.I cullHandler.h \
    cullLights.I cullLights.h \
    cullPlanes.I cullPlanes.h \
    cullResult.I cullResult.h \
    cullTraverser.I cullTraverser.h \
    cullTraverserData.I cullTraverserData.h \
    cullableObject.I cullableObject.h \
    decalEffect.I decalEffect.h \
    depthBiasAttrib.I depthBiasAttrib.h \
    depthOffsetAttrib.I depthOffsetAttrib.h \
    depthTestAttrib.I depthTestAttrib.h \
    depthWriteAttrib.I depthWriteAttrib.h \
    eventStorePandaNode.I eventStorePandaNode.h \
    fog.I fog.h \
    fogAttrib.I fogAttrib.h \
    geomDrawCallbackData.I geomDrawCallbackData.h \
    geomNode.I geomNode.h \
    geomTransformer.I geomTransformer.h \
    instanceList.I instanceList.h \
    instancedNode.I instancedNode.h \
    internalNameCollection.I internalNameCollection.h \
    lensNode.I lensNode.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightRampAttrib.I lightRampAttrib.h \
    loader.I loader.h \
    loaderFileType.h \
    loaderFileTypeBam.h \
    loaderFileTypeRegistry.h \
    logicOpAttrib.I logicOpAttrib.h \
    materialAttrib.I materialAttrib.h \
    materialCollection.I materialCollection.h \
    modelFlattenRequest.I modelFlattenRequest.h \
    modelLoadRequest.I modelLoadRequest.h \
    modelSaveRequest.I modelSaveRequest.h \
    modelNode.I modelNode.h \
    modelPool.I modelPool.h \
    modelRoot.I modelRoot.h \
    nodePath.I nodePath.h \
    nodePathCollection.I nodePathCollection.h \
    nodePathComponent.I nodePathComponent.h \
    occluderEffect.I occluderEffect.h \
    occluderNode.I occluderNode.h \
    pandaNode.I pandaNode.h \
    pandaNodeChain.I pandaNodeChain.h \
    paramNodePath.I paramNodePath.h \
    planeNode.I planeNode.h \
    polylightEffect.I polylightEffect.h \
    polylightNode.I polylightNode.h \
    portalNode.I portalNode.h \
    portalClipper.I portalClipper.h \
    renderAttrib.I renderAttrib.h \
    renderAttribRegistry.I renderAttribRegistry.h \
    renderEffect.I renderEffect.h \
    renderEffects.I renderEffects.h \
    renderModeAttrib.I renderModeAttrib.h \
    renderState.I renderState.h \
    rescaleNormalAttrib.I rescaleNormalAttrib.h \
    sceneGraphReducer.I sceneGraphReducer.h \
    sceneSetup.I sceneSetup.h \
    scissorAttrib.I scissorAttrib.h \
    scissorEffect.I scissorEffect.h \
    shadeModelAttrib.I shadeModelAttrib.h \
    shaderAttrib.I shaderAttrib.h \
    shaderInput.I shaderInput.h \
    shaderPool.I shaderPool.h \
    showBoundsEffect.I showBoundsEffect.h \
    stateMunger.I stateMunger.h \
    stencilAttrib.I stencilAttrib.h \
    texMatrixAttrib.I texMatrixAttrib.h \
    texProjectorEffect.I texProjectorEffect.h \
    textureAttrib.I textureAttrib.h \
    texGenAttrib.I texGenAttrib.h \
    textureStageCollection.I textureStageCollection.h \
    transformState.I transformState.h \
    transparencyAttrib.I transparencyAttrib.h \
    weakNodePath.I weakNodePath.h \
    workingNodePath.I workingNodePath.h

// No need to install these.
//    findApproxLevelEntry.I findApproxLevelEntry.h \
//    findApproxPath.I findApproxPath.h \

  #define IGATESCAN all

  #define IGATEEXT \
    loaderFileTypeRegistry_ext.cxx \
    loaderFileTypeRegistry_ext.h \
    nodePathCollection_ext.cxx \
    nodePathCollection_ext.h \
    nodePath_ext.cxx \
    nodePath_ext.h \
    nodePath_ext.I \
    pandaNode_ext.cxx \
    pandaNode_ext.h \
    pythonLoaderFileType.cxx \
    pythonLoaderFileType.h \
    renderState_ext.cxx \
    renderState_ext.h \
    shaderAttrib_ext.cxx \
    shaderAttrib_ext.h \
    shaderInput_ext.cxx \
    shaderInput_ext.h \
    transformState_ext.cxx \
    transformState_ext.h

// Uncomment these lines to compile everything individually instead of
// combining into pgraph_composite*.cxx.

//  #define SOURCES $[SOURCES] $[COMPOSITE_SOURCES]
//  #define COMPOSITE_SOURCES

#end lib_target


#begin test_bin_target
  #define TARGET test_pgraph

  #define SOURCES \
    test_pgraph.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] pgraph

#end test_bin_target
