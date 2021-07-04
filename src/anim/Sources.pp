// New animation system

#define LOCAL_LIBS \
  pgraph ssemath

#begin lib_target
  #define TARGET anim

  #define BUILDING_DLL BUILDING_PANDA_ANIM

  #define SOURCES \
    animActivity.h \
    animAddNode.h animAddNode.I \
    animBlendNode2D.h animBlendNode2D.I \
    animBundle.h animBundle.I \
    animBundleNode.h animBundleNode.I \
    animEvent.h \
    animGraphNode.h animGraphNode.I \
    animLayer.h animLayer.I \
    animMixNode.h animMixNode.I \
    animOverlayNode.h animOverlayNode.I \
    animPreloadTable.h animPreloadTable.I \
    animSequence.h animSequence.I \
    animSequencePlayer.h animSequencePlayer.I \
    animStateMachine.h animStateMachine.I \
    auto_bind.h \
    bindAnimRequest.h bindAnimRequest.I \
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
    partSubset.h partSubset.I \
    poseParameter.h poseParameter.I \
    sharedEnum.h sharedEnum.I \
    weightList.h weightList.I

  #define COMPOSITE_SOURCES \
    animActivity.cxx \
    animAddNode.cxx \
    animBlendNode2D.cxx \
    animBundle.cxx \
    animBundleNode.cxx \
    animEvent.cxx \
    animGraphNode.cxx \
    animLayer.cxx \
    animMixNode.cxx \
    animOverlayNode.cxx \
    animPreloadTable.cxx \
    animSequence.cxx \
    animSequencePlayer.cxx \
    animStateMachine.cxx \
    auto_bind.cxx \
    bindAnimRequest.cxx \
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
    partSubset.cxx \
    poseParameter.cxx \
    sharedEnum.cxx \
    weightList.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

  #define IGATEEXT \
    sharedEnum_ext.h sharedEnum_ext.I

#end lib_target
