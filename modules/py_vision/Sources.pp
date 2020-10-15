#define DIR_TYPE module

#define BUILD_DIRECTORY $[or $[HAVE_OPENCV],$[HAVE_ARTOOLKIT]]

#begin python_module_target
  #define TARGET panda3d.vision
  #define IGATE_LIBS vision
  #define IMPORT panda3d.core
#end python_module_target
