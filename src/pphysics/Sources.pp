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
    physScene.h physScene.I \
    physSphere.h physSphere.I \
    physSystem.h physSystem.I \
    physXAllocator.h \
    physXErrorCallback.h

  #define COMPOSITE_SOURCES \
    config_pphysics.cxx \
    physBox.cxx \
    physCapsule.cxx \
    physScene.cxx \
    physSphere.cxx \
    physSystem.cxx \
    physXAllocator.cxx \
    physXErrorCallback.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
