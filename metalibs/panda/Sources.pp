// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDA
#define USE_PACKAGES net

#define COMPONENT_LIBS \
  p3chan p3char p3collide p3cull p3device p3dgraph p3display p3dxml \
  p3event p3gobj p3grutil p3gsgbase p3linmath p3mathutil \
  p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui p3pipeline \
  p3pnmimage p3pnmimagetypes p3pstatclient p3putil p3recorder p3text p3tform \

#define LOCAL_LIBS \
  p3downloader p3express p3pandabase
#define OTHER_LIBS \
  pandaexpress:m \
  p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
  p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin metalib_target
  #define TARGET panda

  #define SOURCES panda.cxx panda.h
  #define INSTALL_HEADERS panda.h
#end metalib_target
