#define DIR_TYPE module

#begin python_module_target
  #define TARGET panda3d.physics

  #define IGATE_LIBS physics particlesystem
  #define IMPORT panda3d.core

  #define LOCAL_LIBS pandaphysics

#end python_module_target
