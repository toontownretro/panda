#define BUILD_DIRECTORY $[and $[HAVE_NET],$[WANT_NATIVE_NET],$[HAVE_VALVE_STEAMNET]]

#begin lib_target
  #define TARGET steamnet

  #define BUILDING_DLL BUILDING_PANDA_STEAMNET

  #define USE_PACKAGES valve_steamnet
  #define LOCAL_LIBS express net putil pstatclient

  #define SOURCES \
    config_steamnet.h \
    steamnet_includes.h \
    steamNetworkConnectionInfo.h steamNetworkConnectionInfo.I \
    steamNetworkEnums.h \
    steamNetworkEvent.h steamNetworkEvent.I \
    steamNetworkMessage.h steamNetworkMessage.I \
    steamNetworkSystem.h steamNetworkSystem.I

  #define COMPOSITE_SOURCES \
    config_steamnet.cxx \
    steamNetworkMessage.cxx \
    steamNetworkSystem.cxx

  #define INSTALL_HEADERS \
    config_steamnet.h \
    steamnet_includes.h \
    steamNetworkConnectionInfo.h steamNetworkConnectionInfo.I \
    steamNetworkEnums.h \
    steamNetworkEvent.h steamNetworkEvent.I \
    steamNetworkMessage.h steamNetworkMessage.I \
    steamNetworkSystem.h steamNetworkSystem.I

  #define IGATESCAN all

#end lib_target
