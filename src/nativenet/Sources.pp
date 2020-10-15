#define LOCAL_LIBS express pandabase

#define OTHER_LIBS \
    interrogatedb \
    dtoolutil:c dtoolbase:c prc dtool:m

#define BUILD_DIRECTORY $[WANT_NATIVE_NET]
#define USE_PACKAGES native_net openssl

#begin lib_target
  #define TARGET nativenet

  #define BUILDING_DLL BUILDING_PANDA_NATIVENET

  #define SOURCES \
    config_nativenet.h \
    buffered_datagramconnection.h \
    buffered_datagramreader.h buffered_datagramreader.I \
    buffered_datagramwriter.h buffered_datagramwriter.I \
    ringbuffer.h ringbuffer.I socket_ip.h \
    socket_tcp.h socket_tcp_listen.h \
    time_accumulator.h time_out.h \
    socket_address.I socket_address.h \
    socket_portable.h time_base.h time_span.h \
    socket_base.h socket_selector.h \
    socket_udp.h \
    socket_udp_incoming.h time_clock.h \
    membuffer.h membuffer.I socket_fdset.h \
    socket_udp_outgoing.h time_general.h

  #define COMPOSITE_SOURCES \
    config_nativenet.cxx \
    buffered_datagramconnection.cxx \
    socket_address.cxx \
    socket_ip.cxx \
    socket_tcp.cxx \
    socket_tcp_listen.cxx \
    socket_udp.cxx \
    socket_udp_incoming.cxx \
    socket_udp_outgoing.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
