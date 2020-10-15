#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET dgraph
  #define LOCAL_LIBS \
    pgraph

  #define BUILDING_DLL BUILDING_PANDA_DGRAPH

  #define SOURCES \
    config_dgraph.h \
    dataGraphTraverser.I dataGraphTraverser.h \
    dataNode.I dataNode.h \
    dataNodeTransmit.I dataNodeTransmit.h

 #define COMPOSITE_SOURCES \
    config_dgraph.cxx \
    dataGraphTraverser.cxx \
    dataNode.cxx \
    dataNodeTransmit.cxx

  #define INSTALL_HEADERS \
    config_dgraph.h \
    dataGraphTraverser.I dataGraphTraverser.h \
    dataNode.I dataNode.h \
    dataNodeTransmit.I dataNodeTransmit.h

  #define IGATESCAN \
    all

#end lib_target
