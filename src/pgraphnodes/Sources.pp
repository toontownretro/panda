#define OTHER_LIBS p3interrogatedb:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m
#define LOCAL_LIBS \
    p3event p3gsgbase p3gobj p3putil p3linmath \
    p3downloader p3express p3pandabase p3pstatclient p3pgraph

#begin lib_target
  #define TARGET p3pgraphnodes

  #define BUILDING_DLL BUILDING_PANDA_PGRAPHNODES

  #define SOURCES \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    computeNode.h computeNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
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
    shaderGenerator.h shaderGenerator.I \
    sphereLight.h sphereLight.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I \
    uvScrollNode.I uvScrollNode.h

  #define COMPOSITE_SOURCES \
    ambientLight.cxx \
    callbackNode.cxx \
    computeNode.cxx \
    config_pgraphnodes.cxx \
    directionalLight.cxx \
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
    shaderGenerator.cxx \
    sphereLight.cxx \
    spotlight.cxx \
    switchNode.cxx \
    uvScrollNode.cxx

  #define INSTALL_HEADERS \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    computeNode.h computeNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
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
    shaderGenerator.h shaderGenerator.I \
    sphereLight.h sphereLight.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I \
    uvScrollNode.I uvScrollNode.h

  #define IGATESCAN all

#end lib_target
