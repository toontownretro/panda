#begin static_lib_target
  #define TARGET p3tinyxml

  #define SOURCES \
     tinyxml.h

  #define COMPOSITE_SOURCES  \
     tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  #define INSTALL_HEADERS \
    tinyxml.h

  #define EXTRA_CDEFS TIXML_USE_STL
  #define C++FLAGS $[CFLAGS_SHARED]

#end static_lib_target

#begin lib_target
  #define TARGET p3dxml

  #define LOCAL_LIBS p3pandabase
  #define OTHER_LIBS \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3prc:c p3dtool:m

  #define SOURCES \
    config_dxml.h tinyxml.h

  #define COMPOSITE_SOURCES \
    config_dxml.cxx \
    tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  // It's important not to include tinyxml.h on the IGATESCAN list,
  // because that file has to be bracketed by BEGIN_PUBLISH
  // .. END_PUBLISH (which is handled by config_dxml.cxx).
  #define IGATESCAN config_dxml.h $[COMPOSITE_SOURCES]
#end lib_target
