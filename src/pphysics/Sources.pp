#define BUILD_DIRECTORY $[HAVE_PHYSX]

#define LOCAL_LIBS pgraph linmath mathutil anim audio pipeline
#define USE_PACKAGES physx

#begin lib_target
  #define TARGET pphysics
  #define BUILDING_DLL BUILDING_PANDA_PPHYSICS

  #define SOURCES \
    config_pphysics.h \
    physBox.h physBox.I \
    physBoxController.h physBoxController.I \
    physCapsule.h physCapsule.I \
    physContactCallbackData.h physContactCallbackData.I \
    physController.h physController.I \
    physConvexMesh.h physConvexMesh.I \
    physConvexMeshData.h physConvexMeshData.I \
    physD6Joint.h physD6Joint.I \
    physDistanceJoint.h physDistanceJoint.I \
    physEnums.h \
    physFixedJoint.h physFixedJoint.I \
    physGeometry.h \
    physJoint.h physJoint.I \
    physJointLimit.h physJointLimit.I \
    physJointLimitAngularPair.h physJointLimitAngularPair.I \
    physJointLimitCone.h physJointLimitCone.I \
    physJointLimitLinearPair.h physJointLimitLinearPair.I \
    physJointLimitPyramid.h physJointLimitPyramid.I \
    physMaterial.h physMaterial.I \
    physPlane.h physPlane.I \
    physPrismaticJoint.h physPrismaticJoint.I \
    physQueryFilter.h physQueryFilter.I \
    physRagdoll.h physRagdoll.I \
    physRayCastResult.h physRayCastResult.I \
    physRevoluteJoint.h physRevoluteJoint.I \
    physRigidActorNode.h physRigidActorNode.I \
    physRigidBodyNode.h physRigidBodyNode.I \
    physRigidDynamicNode.h physRigidDynamicNode.I \
    physRigidStaticNode.h physRigidStaticNode.I \
    physScene.h physScene.I \
    physShape.h physShape.I \
    physSleepStateCallbackData.h physSleepStateCallbackData.I \
    physSphere.h physSphere.I \
    physSphericalJoint.h physSphericalJoint.I \
    physSweepResult.h physSweepResult.I \
    physSystem.h physSystem.I \
    physTriggerCallbackData.h physTriggerCallbackData.I \
    physx_includes.h \
    physx_shaders.h physx_shaders.I \
    physx_utils.h physx_utils.I \
    physXAllocator.h \
    physXErrorCallback.h \
    physXSimulationEventCallback.h physXSimulationEventCallback.I \
    refCallbackData.h refCallbackData.I \

  #define COMPOSITE_SOURCES \
    config_pphysics.cxx \
    physBox.cxx \
    physBoxController.cxx \
    physCapsule.cxx \
    physContactCallbackData.cxx \
    physController.cxx \
    physConvexMesh.cxx \
    physConvexMeshData.cxx \
    physD6Joint.cxx \
    physDistanceJoint.cxx \
    physFixedJoint.cxx \
    physJoint.cxx \
    physJointLimit.cxx \
    physJointLimitAngularPair.cxx \
    physJointLimitCone.cxx \
    physJointLimitLinearPair.cxx \
    physJointLimitPyramid.cxx \
    physMaterial.cxx \
    physPlane.cxx \
    physPrismaticJoint.cxx \
    physQueryFilter.cxx \
    physRagdoll.cxx \
    physRayCastResult.cxx \
    physRevoluteJoint.cxx \
    physRigidActorNode.cxx \
    physRigidBodyNode.cxx \
    physRigidDynamicNode.cxx \
    physRigidStaticNode.cxx \
    physScene.cxx \
    physShape.cxx \
    physSleepStateCallbackData.cxx \
    physSphere.cxx \
    physSphericalJoint.cxx \
    physSweepResult.cxx \
    physSystem.cxx \
    physTriggerCallbackData.cxx \
    physx_shaders.cxx \
    physXAllocator.cxx \
    physXErrorCallback.cxx \
    physXSimulationEventCallback.cxx \
    refCallbackData.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all
  #define IGATEEXT \
    pythonPhysQueryFilter.h \
    pythonPhysQueryFilter.cxx

#end lib_target
