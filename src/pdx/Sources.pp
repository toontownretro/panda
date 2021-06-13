#define LOCAL_LIBS express putil

#begin lib_target
  #define TARGET pdx
  #define BUILDING_DLL BUILDING_PANDA_PDX

  #define SOURCES \
    config_pdx.h \
    pdxElement.h pdxElement.I \
    pdxList.h pdxList.I \
    pdxValue.h pdxValue.I

  #define COMPOSITE_SOURCES \
    config_pdx.cxx \
    pdxElement.cxx \
    pdxList.cxx \
    pdxValue.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
