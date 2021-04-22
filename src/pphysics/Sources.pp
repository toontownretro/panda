#define BUILD_DIRECTORY $[HAVE_PHYSX]

#define LOCAL_LIBS pgraph linmath mathutil
#define USE_PACKAGES physx

#begin lib_target
  #define TARGET pphysics
  #define BUILDING_DLL BUILDING_PANDA_PPHYSICS

  #define SOURCES \
    config_pphysics.h \
    physBox.h physBox.I \
    physCapsule.h physCapsule.I \
    physD6Joint.h physD6Joint.I \
    physDistanceJoint.h physDistanceJoint.I \
    physFixedJoint.h physFixedJoint.I \
    physGeometry.h \
    physJoint.h physJoint.I \
    physJointLimit.h physJointLimit.I \
    physJointLimitAngularPair.h physJointLimitAngularPair.I \
    physJointLimitCone.h physJointLimitCone.I \
    physJointLimitLinearPair.h physJointLimitLinearPair.I \
    physMaterial.h physMaterial.I \
    physPlane.h physPlane.I \
    physPrismaticJoint.h physPrismaticJoint.I \
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
    physSystem.h physSystem.I \
    physTriggerCallbackData.h physTriggerCallbackData.I \
    physx_includes.h \
    physx_shaders.h physx_shaders.I \
    physx_utils.h physx_utils.I \
    physXAllocator.h \
    physXErrorCallback.h \
    physXSimulationEventCallback.h physXSimulationEventCallback.I

  #define COMPOSITE_SOURCES \
    config_pphysics.cxx \
    physBox.cxx \
    physCapsule.cxx \
    physD6Joint.cxx \
    physDistanceJoint.cxx \
    physFixedJoint.cxx \
    physJoint.cxx \
    physJointLimit.cxx \
    physJointLimitAngularPair.cxx \
    physJointLimitCone.cxx \
    physJointLimitLinearPair.cxx \
    physMaterial.cxx \
    physPlane.cxx \
    physPrismaticJoint.cxx \
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
    physSystem.cxx \
    physTriggerCallbackData.cxx \
    physx_shaders.cxx \
    physXAllocator.cxx \
    physXErrorCallback.cxx \
    physXSimulationEventCallback.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
