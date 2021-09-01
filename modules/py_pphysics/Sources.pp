#define DIR_TYPE module

#define BUILD_DIRECTORY $[HAVE_PHYSX]

#begin python_module_target
  #define TARGET panda3d.pphysics
  #define IGATE_LIBS pphysics
  #define IMPORT panda3d.core
#end python_module_target
