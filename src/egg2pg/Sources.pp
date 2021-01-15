#define BUILD_DIRECTORY $[HAVE_EGG]

#define OTHER_LIBS dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET egg2pg
  #define LOCAL_LIBS \
    parametrics collide egg pgraph chan char

  #define BUILDING_DLL BUILDING_PANDA_EGG2PG

  #define SOURCES \
    animBundleMaker.h \
    characterMaker.h \
    config_egg2pg.h \
    deferredNodeProperty.h \
    eggBinner.h \
    eggLoader.h eggLoader.I \
    eggRenderState.h eggRenderState.I \
    //eggSaver.h eggSaver.I \
    egg_parametrics.h \
    load_egg_file.h \
    //save_egg_file.h \
    loaderFileTypeEgg.h

  #define COMPOSITE_SOURCES \
    animBundleMaker.cxx \
    characterMaker.cxx \
    config_egg2pg.cxx \
    deferredNodeProperty.cxx \
    eggBinner.cxx \
    eggLoader.cxx \
    eggRenderState.cxx \
    //eggSaver.cxx \
    egg_parametrics.cxx \
    load_egg_file.cxx \
    //save_egg_file.cxx \
    loaderFileTypeEgg.cxx

  #if $[DONT_COMPOSITE_PGRAPH]
    #define SOURCES $[SOURCES] $[COMPOSITE_SOURCES]
    #define COMPOSITE_SOURCES
  #endif

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target
