#define DIR_TYPE module

#define BUILD_DIRECTORY $[HAVE_VRPN]

#begin python_module_target
  #define TARGET panda3d.vrpn
  #define IGATE_LIBS p3vrpn
  #define IMPORT panda3d.core
#end python_module_target
