#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET tform
  #define LOCAL_LIBS \
    grutil dgraph pgraph linmath display event putil gobj gsgbase \
    mathutil device

  #define BUILDING_DLL BUILDING_PANDA_TFORM

  #define SOURCES  \
    buttonThrower.I buttonThrower.h \
    config_tform.h \
    driveInterface.I driveInterface.h \
    mouseInterfaceNode.I mouseInterfaceNode.h \
    mouseSubregion.I mouseSubregion.h \
    mouseWatcher.I mouseWatcher.h \
    mouseWatcherBase.h mouseWatcherBase.I \
    mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    trackball.h \
    transform2sg.h

  #define COMPOSITE_SOURCES  \
    buttonThrower.cxx \
    config_tform.cxx \
    driveInterface.cxx \
    mouseInterfaceNode.cxx \
    mouseSubregion.cxx \
    mouseWatcher.cxx \
    mouseWatcherBase.cxx \
    mouseWatcherGroup.cxx \
    mouseWatcherParameter.cxx \
    mouseWatcherRegion.cxx \
    trackball.cxx \
    transform2sg.cxx

  #define INSTALL_HEADERS \
    buttonThrower.I buttonThrower.h \
    driveInterface.I driveInterface.h \
    mouseInterfaceNode.I mouseInterfaceNode.h \
    mouseSubregion.I mouseSubregion.h \
    mouseWatcher.I mouseWatcher.h \
    mouseWatcherBase.h mouseWatcherBase.I \
    mouseWatcherGroup.h \
    mouseWatcherParameter.I mouseWatcherParameter.h \
    mouseWatcherRegion.I mouseWatcherRegion.h \
    trackball.h \
    transform2sg.h

  #define IGATESCAN all

#end lib_target
