// Don't build it, this is just an example.
#define BUILD_DIRECTORY

#define OTHER_LIBS p3interrogatedb:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:m

#define USE_PACKAGES
#define BUILDING_DLL BUILDING_PANDASKEL

#begin lib_target
  #define TARGET p3skel
  #define LOCAL_LIBS \
    p3display p3text p3pgraph p3gobj p3linmath p3putil

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
