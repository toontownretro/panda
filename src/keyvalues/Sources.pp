#begin lib_target
  #define TARGET p3keyvalues
  #define LOCAL_LIBS p3linmath p3express p3putil

  #define SOURCES \
    config_keyvalues.h \
    keyValues.h

  #define COMPOSITE_SOURCES \
    config_keyvalues.cxx \
    keyValues.cxx

  #define INSTALL_HEADERS \
    config_keyvalues.h \
    keyValues.h

  #define IGATESCAN all
  #define IGATEEXT \
    keyValues_ext.h keyValues_ext.cxx

#end lib_target
