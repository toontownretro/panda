#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET pgui
  #define LOCAL_LIBS \
    audio grutil text tform linmath event putil gobj \
    mathutil

  #define BUILDING_DLL BUILDING_PANDA_PGUI

  #define SOURCES  \
    config_pgui.h \
    pgButton.I pgButton.h \
    pgButtonNotify.I pgButtonNotify.h \
    pgCullTraverser.I pgCullTraverser.h \
    pgEntry.I pgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    pgItemNotify.I pgItemNotify.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgScrollFrame.I pgScrollFrame.h \
    pgSliderBar.I pgSliderBar.h \
    pgSliderBarNotify.I pgSliderBarNotify.h \
    pgTop.I pgTop.h \
    pgVirtualFrame.I pgVirtualFrame.h \
    pgWaitBar.I pgWaitBar.h

  #define COMPOSITE_SOURCES  \
    config_pgui.cxx \
    pgButton.cxx \
    pgButtonNotify.cxx \
    pgCullTraverser.cxx \
    pgEntry.cxx \
    pgMouseWatcherGroup.cxx \
    pgMouseWatcherParameter.cxx \
    pgFrameStyle.cxx \
    pgItem.cxx \
    pgItemNotify.cxx \
    pgMouseWatcherBackground.cxx \
    pgMouseWatcherRegion.cxx \
    pgScrollFrame.cxx \
    pgSliderBar.cxx \
    pgSliderBarNotify.cxx \
    pgTop.cxx \
    pgVirtualFrame.cxx \
    pgWaitBar.cxx

  #define INSTALL_HEADERS \
    pgButton.I pgButton.h \
    pgButtonNotify.I pgButtonNotify.h \
    pgCullTraverser.I pgCullTraverser.h \
    pgEntry.I pgEntry.h \
    pgMouseWatcherGroup.I pgMouseWatcherGroup.h \
    pgMouseWatcherParameter.I pgMouseWatcherParameter.h \
    pgFrameStyle.I pgFrameStyle.h \
    pgItem.I pgItem.h \
    pgItemNotify.I pgItemNotify.h \
    pgMouseWatcherBackground.h \
    pgMouseWatcherRegion.I pgMouseWatcherRegion.h \
    pgScrollFrame.I pgScrollFrame.h \
    pgSliderBar.I pgSliderBar.h \
    pgSliderBarNotify.I pgSliderBarNotify.h \
    pgTop.I pgTop.h \
    pgVirtualFrame.I pgVirtualFrame.h \
    pgWaitBar.I pgWaitBar.h


  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_pgentry

  #define LOCAL_LIBS \
    framework putil collide pgraph text \
    pnmimage pnmimagetypes event gobj display \
    mathutil putil express dgraph device tform \
    linmath pstatclient panda anim

  #define UNIX_SYS_LIBS m

  #define SOURCES \
    test_pgentry.cxx

#end test_bin_target
