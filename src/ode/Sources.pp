#define BUILD_DIRECTORY $[HAVE_ODE]
#define BUILDING_DLL BUILDING_PANDAODE

#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET pandaode
  #define LOCAL_LIBS \
    pgraph physics

  #define USE_PACKAGES ode

  #define SOURCES \
    ode_includes.h config_ode.h \
    odeWorld.I odeWorld.h \
    odeMass.I odeMass.h \
    odeBody.I odeBody.h \
    odeJointGroup.I odeJointGroup.h \
    odeJoint.I odeJoint.h \
    odeUtil.h \
    odeSpace.I odeSpace.h \
    odeGeom.I odeGeom.h \
    odeSurfaceParameters.I odeSurfaceParameters.h \
    odeContactGeom.I odeContactGeom.h \
    odeContact.I odeContact.h \
    odeAMotorJoint.I odeAMotorJoint.h \
    odeBallJoint.I odeBallJoint.h \
    odeContactJoint.I odeContactJoint.h \
    odeFixedJoint.I odeFixedJoint.h \
    odeHingeJoint.I odeHingeJoint.h \
    odeHinge2Joint.I odeHinge2Joint.h \
    odeLMotorJoint.I odeLMotorJoint.h \
    odeNullJoint.I odeNullJoint.h \
    odePlane2dJoint.I odePlane2dJoint.h \
    odeSliderJoint.I odeSliderJoint.h \
    odeUniversalJoint.I odeUniversalJoint.h \
    odeJointCollection.I odeJointCollection.h \
    odeSimpleSpace.I odeSimpleSpace.h \
    odeHashSpace.I odeHashSpace.h \
    odeQuadTreeSpace.I odeQuadTreeSpace.h \
    odeSphereGeom.I odeSphereGeom.h \
    odeBoxGeom.I odeBoxGeom.h \
    odePlaneGeom.I odePlaneGeom.h \
    odeCappedCylinderGeom.I odeCappedCylinderGeom.h \
    odeCylinderGeom.I odeCylinderGeom.h \
    odeRayGeom.I odeRayGeom.h \
    odeTriMeshData.I odeTriMeshData.h \
    odeTriMeshGeom.I odeTriMeshGeom.h \
    odeCollisionEntry.I odeCollisionEntry.h \
    odeHelperStructs.h

  #define COMPOSITE_SOURCES \
    config_ode.cxx \
    odeWorld.cxx odeMass.cxx \
    odeBody.cxx \
    odeJointGroup.cxx \
    odeJoint.cxx \
    odeUtil.cxx \
    odeSpace.cxx \
    odeGeom.cxx \
    odeSurfaceParameters.cxx \
    odeContactGeom.cxx odeContact.cxx \
    odeAMotorJoint.cxx odeBallJoint.cxx \
    odeContactJoint.cxx odeFixedJoint.cxx \
    odeHingeJoint.cxx odeHinge2Joint.cxx \
    odeLMotorJoint.cxx odeNullJoint.cxx \
    odePlane2dJoint.cxx odeSliderJoint.cxx \
    odeUniversalJoint.cxx odeJointCollection.cxx\
    odeSimpleSpace.cxx \
    odeHashSpace.cxx odeQuadTreeSpace.cxx \
    odeSphereGeom.cxx odeBoxGeom.cxx \
    odePlaneGeom.cxx odeCappedCylinderGeom.cxx \
    odeCylinderGeom.cxx odeRayGeom.cxx \
    odeTriMeshData.cxx  odeTriMeshGeom.cxx \
    odeCollisionEntry.cxx


  #define INSTALL_HEADERS \
    ode_includes.h config_ode.h \
    odeWorld.I odeWorld.h \
    odeMass.I odeMass.h \
    odeBody.I odeBody.h \
    odeJointGroup.I odeJointGroup.h \
    odeJoint.I odeJoint.h \
    odeUtil.h \
    odeSpace.I odeSpace.h \
    odeGeom.I odeGeom.h \
    odeSurfaceParameters.I odeSurfaceParameters.h \
    odeContactGeom.I odeContactGeom.h \
    odeContact.I odeContact.h \
    odeAMotorJoint.I odeAMotorJoint.h \
    odeBallJoint.I odeBallJoint.h \
    odeContactJoint.I odeContactJoint.h \
    odeFixedJoint.I odeFixedJoint.h \
    odeHingeJoint.I odeHingeJoint.h \
    odeHinge2Joint.I odeHinge2Joint.h \
    odeLMotorJoint.I odeLMotorJoint.h \
    odeNullJoint.I odeNullJoint.h \
    odePlane2dJoint.I odePlane2dJoint.h \
    odeSliderJoint.I odeSliderJoint.h \
    odeUniversalJoint.I odeUniversalJoint.h \
    odeJointCollection.I odeJointCollection.h \
    odeSimpleSpace.I odeSimpleSpace.h \
    odeHashSpace.I odeHashSpace.h \
    odeQuadTreeSpace.I odeQuadTreeSpace.h \
    odeSphereGeom.I odeSphereGeom.h \
    odeBoxGeom.I odeBoxGeom.h \
    odePlaneGeom.I odePlaneGeom.h \
    odeCappedCylinderGeom.I odeCappedCylinderGeom.h \
    odeCylinderGeom.I odeCylinderGeom.h \
    odeRayGeom.I odeRayGeom.h \
    odeTriMeshData.I odeTriMeshData.h \
    odeTriMeshGeom.I odeTriMeshGeom.h  \
    odeCollisionEntry.I odeCollisionEntry.h

  #define IGATESCAN all

  #define IGATEEXT \
    odeBody_ext.cxx \
    odeBody_ext.h \
    odeBody_ext.I \
    odeGeom_ext.cxx \
    odeGeom_ext.h \
    odeGeom_ext.I \
    odeJoint_ext.cxx \
    odeJoint_ext.h \
    odeSpace_ext.cxx \
    odeSpace_ext.h \
    odeSpace_ext.I \
    odeUtil_ext.cxx \
    odeUtil_ext.h

#end lib_target
