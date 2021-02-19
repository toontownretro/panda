#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET chan
  #define LOCAL_LIBS \
    pgraph

  #define BUILDING_DLL BUILDING_PANDA_CHAN

  #define SOURCES \
    animAddNode.I animAddNode.h \
    animBlendNode2D.I animBlendNode2D.h \
    animBundle.I animBundle.h \
    animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I  \
    animChannelBase.h \
    animChannelFixed.I animChannelFixed.h \
    animChannelMatrixDynamic.I animChannelMatrixDynamic.h \
    animChannelMatrixFixed.I animChannelMatrixFixed.h \
    animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h \
    animChannelScalarDynamic.I animChannelScalarDynamic.h \
    animChannelScalarTable.I animChannelScalarTable.h \
    animControl.I \
    animControl.h animControlCollection.I  \
    animGraphNode.I animGraphNode.h  \
    animControlCollection.h animGroup.I animGroup.h \
    animMixNode.I animMixNode.h \
    animPreloadTable.I animPreloadTable.h \
    animSampleNode.I animSampleNode.h \
    auto_bind.h  \
    bindAnimRequest.I bindAnimRequest.h \
    config_chan.h \
    movingPart.I movingPart.h \
    movingPartBase.I movingPartBase.h  \
    movingPartMatrix.I movingPartMatrix.h movingPartScalar.I  \
    movingPartScalar.h partBundle.I partBundle.N partBundle.h  \
    partBundleHandle.I partBundleHandle.h \
    partBundleNode.I partBundleNode.h \
    partGroup.I partGroup.h  \
    partSubset.I partSubset.h \
    vector_PartGroupStar.h

  #define COMPOSITE_SOURCES  \
    animAddNode.cxx \
    animBlendNode2D.cxx \
    animBundle.cxx \
    animBundleNode.cxx \
    animChannel.cxx  \
    animChannelBase.cxx \
    animChannelFixed.cxx \
    animChannelMatrixDynamic.cxx  \
    animChannelMatrixFixed.cxx  \
    animChannelMatrixXfmTable.cxx  \
    animChannelScalarDynamic.cxx \
    animChannelScalarTable.cxx \
    animControl.cxx  \
    animControlCollection.cxx animGroup.cxx \
    animGraphNode.cxx \
    animMixNode.cxx \
    animPreloadTable.cxx \
    animSampleNode.cxx \
    auto_bind.cxx  \
    bindAnimRequest.cxx \
    config_chan.cxx \
    movingPartBase.cxx movingPartMatrix.cxx  \
    movingPartScalar.cxx partBundle.cxx \
    partBundleHandle.cxx \
    partBundleNode.cxx \
    partGroup.cxx \
    partSubset.cxx \
    vector_PartGroupStar.cxx

  #define INSTALL_HEADERS \
    animAddNode.I animAddNode.h \
    animBlendNode2D.I animBlendNode2D.h \
    animBundle.I animBundle.h \
    animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I  \
    animChannelBase.h \
    animChannelFixed.I animChannelFixed.h \
    animChannelMatrixDynamic.I animChannelMatrixDynamic.h \
    animChannelMatrixFixed.I animChannelMatrixFixed.h \
    animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h \
    animChannelScalarDynamic.I animChannelScalarDynamic.h \
    animChannelScalarTable.I animChannelScalarTable.h \
    animControl.I \
    animControl.h animControlCollection.I  \
    animGraphNode.I animGraphNode.h  \
    animControlCollection.h animGroup.I animGroup.h \
    animMixNode.I animMixNode.h \
    animPreloadTable.I animPreloadTable.h \
    animSampleNode.I animSampleNode.h \
    auto_bind.h  \
    bindAnimRequest.I bindAnimRequest.h \
    config_chan.h \
    movingPart.I movingPart.h \
    movingPartBase.I movingPartBase.h  \
    movingPartMatrix.I movingPartMatrix.h movingPartScalar.I  \
    movingPartScalar.h partBundle.I partBundle.h  \
    partBundleHandle.I partBundleHandle.h \
    partBundleNode.I partBundleNode.h \
    partGroup.I partGroup.h  \
    partSubset.I partSubset.h \
    vector_PartGroupStar.h

  #define IGATESCAN all

#end lib_target
