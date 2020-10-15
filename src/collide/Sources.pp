#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET collide
  #define LOCAL_LIBS \
    tform

  #define BUILDING_DLL BUILDING_PANDA_COLLIDE

  #define SOURCES \
    collisionBox.I collisionBox.h \
    collisionCapsule.I collisionCapsule.h \
    collisionEntry.I collisionEntry.h \
    collisionGeom.I collisionGeom.h \
    collisionHandler.I collisionHandler.h  \
    collisionHandlerEvent.I collisionHandlerEvent.h  \
    collisionHandlerHighestEvent.h  \
    collisionHandlerFloor.I collisionHandlerFloor.h  \
    collisionHandlerGravity.I collisionHandlerGravity.h  \
    collisionHandlerPhysical.I collisionHandlerPhysical.h  \
    collisionHandlerPusher.I collisionHandlerPusher.h  \
    collisionHandlerFluidPusher.I collisionHandlerFluidPusher.h  \
    collisionHandlerQueue.h \
    collisionInvSphere.I collisionInvSphere.h \
    collisionLine.I collisionLine.h \
    collisionLevelStateBase.I collisionLevelStateBase.h \
    collisionLevelState.I collisionLevelState.h \
    collisionNode.I collisionNode.h \
    collisionParabola.I collisionParabola.h  \
    collisionPlane.I collisionPlane.h  \
    collisionPolygon.I collisionPolygon.h \
    collisionFloorMesh.I collisionFloorMesh.h \
    collisionRay.I collisionRay.h \
    collisionRecorder.I collisionRecorder.h \
    collisionSegment.I collisionSegment.h  \
    collisionSolid.I collisionSolid.h \
    collisionSphere.I collisionSphere.h \
    collisionTraverser.I collisionTraverser.h  \
    collisionTube.h \
    collisionVisualizer.I collisionVisualizer.h \
    config_collide.h

 #define COMPOSITE_SOURCES \
    collisionBox.cxx \
    collisionCapsule.cxx \
    collisionEntry.cxx \
    collisionGeom.cxx \
    collisionHandler.cxx \
    collisionHandlerEvent.cxx  \
    collisionHandlerHighestEvent.cxx  \
    collisionHandlerFloor.cxx \
    collisionHandlerGravity.cxx \
    collisionHandlerPhysical.cxx  \
    collisionHandlerPusher.cxx \
    collisionHandlerFluidPusher.cxx \
    collisionHandlerQueue.cxx  \
    collisionLevelStateBase.cxx \
    collisionLevelState.cxx \
    collisionInvSphere.cxx  \
    collisionLine.cxx \
    collisionNode.cxx \
    collisionParabola.cxx  \
    collisionPlane.cxx  \
    collisionPolygon.cxx \
    collisionFloorMesh.cxx \
    collisionRay.cxx \
    collisionRecorder.cxx \
    collisionSegment.cxx  \
    collisionSolid.cxx \
    collisionSphere.cxx  \
    collisionTraverser.cxx \
    collisionVisualizer.cxx \
    config_collide.cxx

  #define INSTALL_HEADERS \
    collisionBox.I collisionBox.h \
    collisionCapsule.I collisionCapsule.h \
    collisionEntry.I collisionEntry.h \
    collisionGeom.I collisionGeom.h \
    collisionHandler.I collisionHandler.h \
    collisionHandlerEvent.I collisionHandlerEvent.h \
    collisionHandlerHighestEvent.h \
    collisionHandlerFloor.I collisionHandlerFloor.h \
    collisionHandlerGravity.I collisionHandlerGravity.h \
    collisionHandlerPhysical.I collisionHandlerPhysical.h \
    collisionHandlerPusher.I collisionHandlerPusher.h \
    collisionHandlerFluidPusher.I collisionHandlerFluidPusher.h \
    collisionHandlerQueue.h \
    collisionInvSphere.I collisionInvSphere.h \
    collisionLevelStateBase.I collisionLevelStateBase.h \
    collisionLevelState.I collisionLevelState.h \
    collisionLine.I collisionLine.h \
    collisionNode.I collisionNode.h \
    collisionParabola.I collisionParabola.h \
    collisionPlane.I collisionPlane.h \
    collisionPolygon.I collisionPolygon.h \
    collisionFloorMesh.I collisionFloorMesh.h \
    collisionRay.I collisionRay.h \
    collisionRecorder.I collisionRecorder.h \
    collisionSegment.I collisionSegment.h \
    collisionSolid.I collisionSolid.h \
    collisionSphere.I collisionSphere.h \
    collisionTraverser.I collisionTraverser.h \
    collisionVisualizer.I collisionVisualizer.h \
    config_collide.h


  #define IGATESCAN all

#end lib_target
