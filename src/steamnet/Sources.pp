#define BUILD_DIRECTORY $[and $[HAVE_NET],$[WANT_NATIVE_NET],$[HAVE_VALVE_STEAMNET]]

#begin lib_target
  #define TARGET p3steamnet

  #define USE_PACKAGES valve_steamnet
  #define LOCAL_LIBS p3express p3net p3putil

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
