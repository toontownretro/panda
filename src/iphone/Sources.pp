#define BUILD_DIRECTORY $[BUILD_IPHONE]

#define OTHER_LIBS interrogatedb  \
                   dtoolutil:c dtoolbase:c dtool:m prc

#define OSX_SYS_FRAMEWORKS Foundation QuartzCore UIKit OpenGLES

#begin bin_target
  #define TARGET iphone_pview

  #define LOCAL_LIBS \
    iphonedisplay \
    framework putil collide pgraph chan text \
    pnmimage pnmimagetypes event gobj display \
    mathutil putil express dgraph device tform \
    linmath pstatclient panda glstuff

  #define SOURCES \
    pview_delegate.h pview_delegate.mm \
    pview_main.mm

#end bin_target
