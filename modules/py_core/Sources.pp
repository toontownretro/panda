#define DIR_TYPE module

#begin python_module_target
  #define TARGET panda3d.core
  #define IGATE_LIBS \
    chan char collide cull device dgraph display \
    downloader dxml event express gobj grutil gsgbase linmath \
    mathutil movies parametrics pgraph pgraphnodes pgui \
    pipeline pnmimage pstatclient putil recorder text tform \
    prc nativenet net audio pnmtext steamnet keyvalues shader \
    postprocess
  #define LOCAL_LIBS panda
#end python_module_target
