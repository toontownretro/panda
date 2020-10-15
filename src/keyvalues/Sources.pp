#begin lib_target
  #define TARGET keyvalues
  #define LOCAL_LIBS linmath express putil

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
