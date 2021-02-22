#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET char
  #define LOCAL_LIBS \
    chan

  #define BUILDING_DLL BUILDING_PANDA_CHAR

  #define SOURCES \
    character.I character.h \
    characterJoint.I characterJoint.h \
    characterJointBundle.I characterJointBundle.h \
    characterJointEffect.h characterJointEffect.I \
    characterSlider.h \
    characterVertexSlider.I characterVertexSlider.h \
    config_char.h \
    jointVertexTransform.I jointVertexTransform.h

  #define COMPOSITE_SOURCES \
    character.cxx \
    characterJoint.cxx characterJointBundle.cxx  \
    characterJointEffect.cxx \
    characterSlider.cxx \
    characterVertexSlider.cxx \
    config_char.cxx  \
    ikChain.cxx \
    ikSolver.cxx \
    jointVertexTransform.cxx

  #define INSTALL_HEADERS \
    character.I character.h \
    characterJoint.I characterJoint.h \
    characterJointBundle.I characterJointBundle.h \
    characterJointEffect.h characterJointEffect.I \
    characterSlider.h \
    characterVertexSlider.I characterVertexSlider.h \
    config_char.h \
    jointVertexTransform.I jointVertexTransform.h

  #define IGATESCAN all

#end lib_target
