#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc
#define LOCAL_LIBS \
    event gsgbase gobj putil linmath \
    downloader express pandabase pstatclient pgraph

#begin lib_target
  #define TARGET pgraphnodes

  #define BUILDING_DLL BUILDING_PANDA_PGRAPHNODES

  #define SOURCES \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    cascadeLight.h cascadeLight.I \
    computeNode.h computeNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
    eyeballNode.h eyeballNode.I \
    fadeLodNode.I fadeLodNode.h fadeLodNodeData.h \
    lightLensNode.h lightLensNode.I \
    lightNode.h lightNode.I \
    lodNode.I lodNode.h lodNodeType.h \
    nodeCullCallbackData.h nodeCullCallbackData.I \
    pointLight.h pointLight.I \
    rectangleLight.h rectangleLight.I \
    sceneGraphAnalyzer.h sceneGraphAnalyzer.I \
    selectiveChildNode.h selectiveChildNode.I \
    sequenceNode.h sequenceNode.I \
    sphereLight.h sphereLight.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I \
    uvScrollNode.I uvScrollNode.h

  #define COMPOSITE_SOURCES \
    ambientLight.cxx \
    callbackNode.cxx \
    cascadeLight.cxx \
    computeNode.cxx \
    config_pgraphnodes.cxx \
    directionalLight.cxx \
    eyeballNode.cxx \
    fadeLodNode.cxx fadeLodNodeData.cxx \
    lightLensNode.cxx \
    lightNode.cxx \
    lodNode.cxx lodNodeType.cxx \
    nodeCullCallbackData.cxx \
    pointLight.cxx \
    rectangleLight.cxx \
    sceneGraphAnalyzer.cxx \
    selectiveChildNode.cxx \
    sequenceNode.cxx \
    sphereLight.cxx \
    spotlight.cxx \
    switchNode.cxx \
    uvScrollNode.cxx

  #define INSTALL_HEADERS \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    cascadeLight.h cascadeLight.I \
    computeNode.h computeNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
    eyeballNode.h eyeballNode.I \
    fadeLodNode.I fadeLodNode.h fadeLodNodeData.h \
    lightLensNode.h lightLensNode.I \
    lightNode.h lightNode.I \
    lodNode.I lodNode.h lodNodeType.h \
    nodeCullCallbackData.h nodeCullCallbackData.I \
    pointLight.h pointLight.I \
    rectangleLight.h rectangleLight.I \
    sceneGraphAnalyzer.h sceneGraphAnalyzer.I \
    selectiveChildNode.h selectiveChildNode.I \
    sequenceNode.h sequenceNode.I \
    sphereLight.h sphereLight.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I \
    uvScrollNode.I uvScrollNode.h

  #define IGATESCAN all

#end lib_target
