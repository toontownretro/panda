#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin interface_target
  #define TARGET pandabase

  #define SOURCES \
    pandabase.h pandasymbols.h \

  #define INSTALL_HEADERS \
    pandabase.h pandasymbols.h

#end interface_target
