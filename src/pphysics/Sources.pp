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
    physGeometry.h \
    physMaterial.h physMaterial.I \
    physPlane.h physPlane.I \
    physRigidActorNode.h physRigidActorNode.I \
    physRigidBodyNode.h physRigidBodyNode.I \
    physRigidDynamicNode.h physRigidDynamicNode.I \
    physRigidStaticNode.h physRigidStaticNode.I \
    physScene.h physScene.I \
    physShape.h physShape.I \
    physSphere.h physSphere.I \
    physSystem.h physSystem.I \
    physx_includes.h \
    physXAllocator.h \
    physXErrorCallback.h

  #define COMPOSITE_SOURCES \
    config_pphysics.cxx \
    physBox.cxx \
    physCapsule.cxx \
    physMaterial.cxx \
    physPlane.cxx \
    physRigidActorNode.cxx \
    physRigidBodyNode.cxx \
    physRigidDynamicNode.cxx \
    physRigidStaticNode.cxx \
    physScene.cxx \
    physShape.cxx \
    physSphere.cxx \
    physSystem.cxx \
    physXAllocator.cxx \
    physXErrorCallback.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
