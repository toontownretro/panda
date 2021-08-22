// New animation system

#define LOCAL_LIBS \
  pgraph ssemath

#begin lib_target
  #define TARGET anim

  #define BUILDING_DLL BUILDING_PANDA_ANIM

  #define SOURCES \
    animActivity.h \
    animChannel.h animChannel.I \
    animChannelBlend1D.h animChannelBlend1D.I \
    animChannelBlend2D.h animChannelBlend2D.I \
    animChannelBundle.h animChannelBundle.I \
    animChannelLayered.h animChannelLayered.I \
    animChannelTable.h animChannelTable.I \
    animEvalContext.h animEvalContext.I \
    animEvent.h \
    animLayer.h animLayer.I \
    character.h character.I \
    characterAttachment.h characterAttachment.I \
    characterJoint.h characterJoint.I \
    characterJointEffect.h characterJointEffect.I \
    characterNode.h characterNode.I \
    characterPart.h characterPart.I \
    characterSlider.h characterSlider.I \
    characterVertexSlider.h characterVertexSlider.I \
    config_anim.h \
    eyeballNode.h eyeballNode.I \
    ikChain.h ikChain.I \
    ikSolver.h \
    jointVertexTransform.h jointVertexTransform.I \
    poseParameter.h poseParameter.I \
    sharedEnum.h sharedEnum.I \
    weightList.h weightList.I

  #define COMPOSITE_SOURCES \
    animActivity.cxx \
    animChannel.cxx \
    animChannelBlend1D.cxx \
    animChannelBlend2D.cxx \
    animChannelBundle.cxx \
    animChannelLayered.cxx \
    animChannelTable.cxx \
    animEvent.cxx \
    animLayer.cxx \
    character.cxx \
    characterAttachment.cxx \
    characterJoint.cxx \
    characterJointEffect.cxx \
    characterNode.cxx \
    characterPart.cxx \
    characterSlider.cxx \
    characterVertexSlider.cxx \
    config_anim.cxx \
    eyeballNode.cxx \
    ikChain.cxx \
    ikSolver.cxx \
    jointVertexTransform.cxx \
    poseParameter.cxx \
    sharedEnum.cxx \
    weightList.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

  #define IGATEEXT \
    sharedEnum_ext.h sharedEnum_ext.I

#end lib_target
