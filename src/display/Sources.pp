#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] ws2_32.lib

#define USE_PACKAGES x11

#begin lib_target
  #define TARGET display
  #define LOCAL_LIBS \
    cull device pgraphnodes gsgbase

  #define BUILDING_DLL BUILDING_PANDA_DISPLAY

  #define SOURCES  \
    config_display.h \
    callbackGraphicsWindow.I callbackGraphicsWindow.h \
    drawableRegion.I drawableRegion.h \
    displayRegion.I displayRegion.h  \
    displayRegionCullCallbackData.I displayRegionCullCallbackData.h \
    displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h \
    frameBufferProperties.I frameBufferProperties.h \
    get_x11.h pre_x11_include.h post_x11_include.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsOutput.I graphicsOutput.h \
    graphicsBuffer.I graphicsBuffer.h \
    graphicsDevice.h graphicsDevice.I \
    graphicsPipe.I graphicsPipe.h  \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.cxx \
    graphicsStateGuardian.I graphicsStateGuardian.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindow.I graphicsWindow.h \
    graphicsWindowInputDevice.I  \
    graphicsWindowInputDevice.h \
    graphicsWindowProc.h \
    graphicsWindowProcCallbackData.I graphicsWindowProcCallbackData.h \
    mouseAndKeyboard.h \
    nativeWindowHandle.I nativeWindowHandle.h \
    parasiteBuffer.I parasiteBuffer.h \
    pStatGPUTimer.I pStatGPUTimer.h \
    windowHandle.I windowHandle.h \
    windowProperties.I windowProperties.h \
    renderBuffer.h \
    stereoDisplayRegion.I stereoDisplayRegion.h \
    displaySearchParameters.h \
    displayInformation.h \
    subprocessWindow.h subprocessWindow.I \
    $[if $[OSX_PLATFORM], subprocessWindowBuffer.h subprocessWindowBuffer.I] \
    touchInfo.h


 #define COMPOSITE_SOURCES  \
    config_display.cxx \
    callbackGraphicsWindow.cxx \
    drawableRegion.cxx \
    displayRegion.cxx \
    displayRegionCullCallbackData.cxx \
    displayRegionDrawCallbackData.cxx \
    displaySearchParameters.cxx \
    displayInformation.cxx \
    frameBufferProperties.cxx \
    graphicsEngine.cxx \
    graphicsOutput.cxx \
    graphicsBuffer.cxx \
    graphicsPipe.cxx \
    graphicsPipeSelection.cxx \
    graphicsThreadingModel.cxx \
    graphicsWindow.cxx graphicsWindowInputDevice.cxx  \
    graphicsWindowProc.cxx \
    graphicsWindowProcCallbackData.cxx \
    graphicsDevice.cxx \
    mouseAndKeyboard.cxx \
    nativeWindowHandle.cxx \
    parasiteBuffer.cxx \
    windowHandle.cxx \
    windowProperties.cxx \
    stereoDisplayRegion.cxx \
    subprocessWindow.cxx \
    $[if $[OSX_PLATFORM], subprocessWindowBuffer.cxx] \
    touchInfo.cxx

  #define INSTALL_HEADERS \
    config_display.h \
    callbackGraphicsWindow.I callbackGraphicsWindow.h \
    drawableRegion.I drawableRegion.h \
    displayInformation.h \
    displayRegion.I displayRegion.h \
    displayRegionCullCallbackData.I displayRegionCullCallbackData.h \
    displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h \
    displaySearchParameters.h \
    frameBufferProperties.I frameBufferProperties.h \
    get_x11.h pre_x11_include.h post_x11_include.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsOutput.I graphicsOutput.h \
    graphicsBuffer.I graphicsBuffer.h \
    graphicsPipe.I graphicsPipe.h \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I \
    graphicsStateGuardian.h \
    graphicsWindow.I graphicsWindow.h \
    graphicsWindowProc.h \
    graphicsWindowProcCallbackData.I graphicsWindowProcCallbackData.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindowInputDevice.I graphicsWindowInputDevice.h \
    graphicsDevice.I graphicsDevice.h \
    mouseAndKeyboard.h \
    nativeWindowHandle.I nativeWindowHandle.h \
    parasiteBuffer.I parasiteBuffer.h \
    pStatGPUTimer.I pStatGPUTimer.h \
    windowHandle.I windowHandle.h \
    windowProperties.I windowProperties.h \
    renderBuffer.h \
    stereoDisplayRegion.I stereoDisplayRegion.h \
    subprocessWindow.h subprocessWindow.I \
    $[if $[OSX_PLATFORM], subprocessWindowBuffer.h subprocessWindowBuffer.I] \
    touchInfo.h

  #define IGATESCAN all

  #define IGATEEXT \
    graphicsStateGuardian_ext.cxx \
    graphicsStateGuardian_ext.h \
    graphicsWindow_ext.cxx \
    graphicsWindow_ext.h \
    pythonGraphicsWindowProc.cxx \
    pythonGraphicsWindowProc.h \
    windowProperties_ext.cxx \
    windowProperties_ext.h

#end lib_target
