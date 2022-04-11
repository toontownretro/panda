#begin lib_target
  #define TARGET particlesystem2
  #define BUILDING_DLL BUILDING_PANDA_PARTICLESYSTEM2

  #define LOCAL_LIBS linmath mathutil gobj pgraph putil parametrics anim

  #define HEADERS \
    config_particlesystem2.h \
    p2_utils.h p2_utils.I \
    particle.h \
    particleConstraint2.h particleConstraint2.I \
    particleEmitter2.h particleEmitter2.I \
    particleForce2.h particleForce2.I \
    particleFunction2.h particleFunction2.I \
    particleInitializer2.h particleInitializer2.I \
    particleRenderer2.h particleRenderer2.I \
    particleSystem2.h particleSystem2.I

  #define SOURCES \
    $[HEADERS]

  #define COMPOSITE_SOURCES \
    config_particlesystem2.cxx \
    p2_utils.cxx \
    particle.cxx \
    particleConstraint2.cxx \
    particleEmitter2.cxx \
    particleForce2.cxx \
    particleFunction2.cxx \
    particleInitializer2.cxx \
    particleRenderer2.cxx \
    particleSystem2.cxx

  #define INSTALL_HEADERS $[HEADERS]

  #define IGATESCAN all

#end lib_target
