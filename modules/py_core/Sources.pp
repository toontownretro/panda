#define DIR_TYPE module

#begin python_module_target
  #define TARGET panda3d.core
  #define IGATE_LIBS \
    p3chan p3char p3collide p3cull p3device p3dgraph p3display \
    p3downloader p3dxml p3event p3express p3gobj p3grutil p3gsgbase p3linmath \
    p3mathutil p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui \
    p3pipeline p3pnmimage p3pstatclient p3putil p3recorder p3text p3tform \
    p3prc p3nativenet p3net p3audio p3pnmtext p3steamnet
  #define LOCAL_LIBS panda
#end python_module_target
