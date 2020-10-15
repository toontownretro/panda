#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil movies audio

  #define BUILDING_DLL BUILDING_PANDA_GRUTIL

  #define SOURCES \
    pipeOcclusionCullTraverser.I pipeOcclusionCullTraverser.h \
    cardMaker.I cardMaker.h \
    config_grutil.h \
    movieTexture.I movieTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    meshDrawer.I meshDrawer.h \
    meshDrawer2D.I meshDrawer2D.h \
    geoMipTerrain.I geoMipTerrain.h \
    sceneGraphAnalyzerMeter.I sceneGraphAnalyzerMeter.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    shaderTerrainMesh.I shaderTerrainMesh.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h multitexReducer.cxx \
    nodeVertexTransform.I nodeVertexTransform.h \
    pfmVizzer.I pfmVizzer.h \
    rigidBodyCombiner.I rigidBodyCombiner.h

  #define COMPOSITE_SOURCES \
    cardMaker.cxx \
    movieTexture.cxx \
    fisheyeMaker.cxx \
    config_grutil.cxx \
    frameRateMeter.cxx \
    meshDrawer.cxx \
    meshDrawer2D.cxx \
    geoMipTerrain.cxx \
    sceneGraphAnalyzerMeter.cxx \
    heightfieldTesselator.cxx \
    shaderTerrainMesh.cxx \
    nodeVertexTransform.cxx \
    pfmVizzer.cxx \
    pipeOcclusionCullTraverser.cxx \
    lineSegs.cxx \
    rigidBodyCombiner.cxx

  #define INSTALL_HEADERS \
    cardMaker.I cardMaker.h \
    movieTexture.I movieTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    meshDrawer.I meshDrawer.h \
    meshDrawer2D.I meshDrawer2D.h \
    geoMipTerrain.I geoMipTerrain.h \
    sceneGraphAnalyzerMeter.I sceneGraphAnalyzerMeter.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    shaderTerrainMesh.I shaderTerrainMesh.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h \
    nodeVertexTransform.I nodeVertexTransform.h \
    pfmVizzer.I pfmVizzer.h \
    rigidBodyCombiner.I rigidBodyCombiner.h

  #define IGATESCAN all

#end lib_target
