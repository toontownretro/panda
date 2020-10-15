#define DIR_TYPE module

#define BUILD_DIRECTORY $[HAVE_EGG]

#begin python_module_target
  #define TARGET panda3d.egg
  #define IGATE_LIBS \
    egg egg2pg
  #define IMPORT panda3d.core

  #define LOCAL_LIBS pandaegg
#end python_module_target
