#define DIR_TYPE module

#define BUILD_DIRECTORY $[HAVE_MAPBUILDER]

#begin python_module_target
  #define TARGET panda3d.mapbuilder
  #define IGATE_LIBS mapbuilder
  #define IMPORT panda3d.core panda3d.pphysics
#end python_module_target
