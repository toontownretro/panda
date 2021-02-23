// New animation system

#define LOCAL_LIBS \
  pgraph

#begin lib_target
  #define TARGET anim

  #define BUILDING_DLL BUILDING_PANDA_ANIM

  #define SOURCES \
    animTable.h animTable.I \
    character.h character.I \
    characterJoint.h characterJoint.I \
    characterSlider.h characterSlider.I \
    config_anim.h

  #define COMPOSITE_SOURCES \
    animTable.cxx \
    character.cxx \
    characterJoint.cxx \
    characterSlider.cxx \
    config_anim.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all
#end lib_target
