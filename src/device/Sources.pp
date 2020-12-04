#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET device
  #define LOCAL_LIBS \
    dgraph

  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] cfgmgr32.lib user32.lib
  #define OSX_SYS_FRAMEWORKS IOKit

  #define BUILDING_DLL BUILDING_PANDA_DEVICE

  #define SOURCES \
    analogNode.I analogNode.h \
    buttonNode.I buttonNode.h  \
    clientAnalogDevice.I clientAnalogDevice.h \
    clientBase.I clientBase.h \
    clientButtonDevice.I clientButtonDevice.h  \
    clientDevice.I clientDevice.h \
    clientDialDevice.I clientDialDevice.h  \
    clientTrackerDevice.I clientTrackerDevice.h  \
    config_device.h \
    dialNode.I dialNode.h  \
    evdevInputDevice.I evdevInputDevice.h \
    inputDevice.I inputDevice.h \
    inputDeviceManager.I inputDeviceManager.h \
    inputDeviceNode.h \
    inputDeviceSet.h inputDeviceSet.I \
    ioKitInputDevice.h \
    ioKitInputDeviceManager.h \
    linuxInputDeviceManager.h \
    linuxJoystickDevice.h linuxJoystickDevice.I \
    trackerData.I trackerData.h \
    trackerNode.I trackerNode.h \
    virtualMouse.h \
    winInputDeviceManager.h \
    winRawInputDevice.h \
    xInputDevice.h

  #define COMPOSITE_SOURCES \
    analogNode.cxx \
    buttonNode.cxx \
    clientAnalogDevice.cxx  \
    clientBase.cxx \
    clientButtonDevice.cxx \
    clientDevice.cxx  \
    clientDialDevice.cxx \
    clientTrackerDevice.cxx  \
    config_device.cxx \
    dialNode.cxx \
    evdevInputDevice.cxx \
    inputDevice.cxx \
    inputDeviceManager.cxx \
    inputDeviceNode.cxx \
    inputDeviceSet.cxx \
    ioKitInputDevice.cxx \
    ioKitInputDeviceManager.cxx \
    linuxInputDeviceManager.cxx \
    linuxJoystickDevice.cxx \
    trackerData.cxx \
    trackerNode.cxx \
    virtualMouse.cxx \
    winInputDeviceManager.cxx \
    winRawInputDevice.cxx \
    xInputDevice.cxx

  #define INSTALL_HEADERS \
    analogNode.I analogNode.h \
    buttonNode.I buttonNode.h  \
    clientAnalogDevice.I clientAnalogDevice.h \
    clientBase.I clientBase.h \
    clientButtonDevice.I clientButtonDevice.h  \
    clientDevice.I clientDevice.h \
    clientDialDevice.I clientDialDevice.h  \
    clientTrackerDevice.I clientTrackerDevice.h  \
    config_device.h \
    dialNode.I dialNode.h  \
    evdevInputDevice.I evdevInputDevice.h \
    inputDevice.I inputDevice.h \
    inputDeviceManager.I inputDeviceManager.h \
    inputDeviceNode.h \
    inputDeviceSet.h inputDeviceSet.I \
    ioKitInputDevice.h \
    ioKitInputDeviceManager.h \
    linuxInputDeviceManager.h \
    linuxJoystickDevice.h linuxJoystickDevice.I \
    trackerData.I trackerData.h \
    trackerNode.I trackerNode.h \
    virtualMouse.h \
    winInputDeviceManager.h \
    winRawInputDevice.h \
    xInputDevice.h

  #define IGATESCAN all

#end lib_target
