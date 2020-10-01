#define OTHER_LIBS p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m

#begin lib_target
  #define TARGET p3char
  #define LOCAL_LIBS \
    p3chan

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
