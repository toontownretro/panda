// Don't build it, this is just an example.
#define BUILD_DIRECTORY

#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#define USE_PACKAGES
#define BUILDING_DLL BUILDING_PANDASKEL

#begin lib_target
  #define TARGET skel
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil

  #define SOURCES \
    config_skel.h \
    basicSkel.I basicSkel.h \
    typedSkel.I typedSkel.h

  #define COMPOSITE_SOURCES \
    config_skel.cxx \
    basicSkel.cxx \
    typedSkel.cxx

  #define INSTALL_HEADERS \
    basicSkel.h basicSkel.I \
    typedSkel.I typedSkel.h

  #define IGATESCAN all

#end lib_target
