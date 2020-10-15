#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define LOCAL_LIBS \
    net putil express

  #define TARGET pstatclient

  #define BUILDING_DLL BUILDING_PANDA_PSTATCLIENT

  #define SOURCES \
     config_pstatclient.h pStatClient.I pStatClient.h \
     pStatClientImpl.I pStatClientImpl.h \
     pStatClientVersion.I  \
     pStatClientVersion.h pStatClientControlMessage.h  \
     pStatCollector.I pStatCollector.h pStatCollectorDef.h  \
     pStatCollectorForward.I pStatCollectorForward.h \
     pStatFrameData.I pStatFrameData.h pStatProperties.h  \
     pStatServerControlMessage.h pStatThread.I pStatThread.h  \
     pStatTimer.I pStatTimer.h

  #define COMPOSITE_SOURCES  \
     config_pstatclient.cxx pStatClient.cxx pStatClientImpl.cxx \
     pStatClientVersion.cxx  \
     pStatClientControlMessage.cxx \
     pStatCollector.cxx \
     pStatCollectorDef.cxx  \
     pStatCollectorForward.cxx \
     pStatFrameData.cxx pStatProperties.cxx  \
     pStatServerControlMessage.cxx \
     pStatThread.cxx

  #define INSTALL_HEADERS \
    config_pstatclient.h pStatClient.I pStatClient.h \
    pStatClientImpl.I pStatClientImpl.h \
    pStatClientVersion.I pStatClientVersion.h \
    pStatClientControlMessage.h pStatCollector.I pStatCollector.h \
    pStatCollectorDef.h \
    pStatCollectorForward.I pStatCollectorForward.h \
    pStatFrameData.I pStatFrameData.h \
    pStatProperties.h \
    pStatServerControlMessage.h pStatThread.I pStatThread.h \
    pStatTimer.I pStatTimer.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define LOCAL_LIBS \
    pstatclient

  #define TARGET test_client

  #define SOURCES \
    test_client.cxx

#end test_bin_target
