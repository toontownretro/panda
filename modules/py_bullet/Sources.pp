#define DIR_TYPE module

#define BUILD_DIRECTORY $[HAVE_BULLET]

#begin python_module_target
  #define TARGET panda3d.bullet
  #define IGATE_LIBS pandabullet
  #define IMPORT panda3d.core
#end python_module_target
