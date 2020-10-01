#define DIR_TYPE module

#define BUILD_DIRECTORY $[HAVE_ODE]

#begin python_module_target
  #define TARGET panda3d.ode
  #define IGATE_LIBS p3ode
  #define IMPORT panda3d.core
#end python_module_target
