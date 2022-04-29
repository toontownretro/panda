// New animation system

#define LOCAL_LIBS \
  pgraph audio

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
    animEventQueue.h animEventQueue.I \
    animLayer.h animLayer.I \
    animTimer.h animTimer.I \
    character.h character.I \
    characterAttachment.h characterAttachment.I \
    characterJoint.h characterJoint.I \
    characterJointEffect.h characterJointEffect.I \
    characterNode.h characterNode.I \
    characterPart.h characterPart.I \
    characterSlider.h characterSlider.I \
    characterTalker.h characterTalker.I \
    characterVertexSlider.h characterVertexSlider.I \
    config_anim.h \
    eyeballNode.h eyeballNode.I \
    ikChain.h ikChain.I \
    ikHelper.h ikHelper.I \
    ikSolver.h \
    ikTarget.h ikTarget.I \
    jointVertexTransform.h jointVertexTransform.I \
    phonemes.h phonemes.I \
    poseParameter.h poseParameter.I \
    pt_AnimChannelTable.h \
    pt_Character.h \
    sharedEnum.h sharedEnum.I \
    spokenSentence.h spokenSentence.I \
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
    characterTalker.cxx \
    characterVertexSlider.cxx \
    config_anim.cxx \
    eyeballNode.cxx \
    ikChain.cxx \
    ikHelper.cxx \
    jointVertexTransform.cxx \
    phonemes.cxx \
    poseParameter.cxx \
    pt_AnimChannelTable.cxx \
    pt_Character.cxx \
    sharedEnum.cxx \
    spokenSentence.cxx \
    weightList.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

  #define IGATEEXT \
    sharedEnum_ext.h sharedEnum_ext.I

#end lib_target
