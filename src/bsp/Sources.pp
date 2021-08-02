#begin lib_target
  #define TARGET bsp
  #define BUILDING_DLL BUILDING_PANDA_BSP

  #define LOCAL_LIBS pandabase express putil linmath mathutil pgraph gobj

  #define SOURCES \
    config_bsp.h \
    bsp.h \
    bspClusterVisibility.h bspClusterVisibility.I \
    bspData.h bspData.I bspData.T \
    bspEnums.h \
    bspFlags.h \
    bspLoader.h bspLoader.I \
    bspLumps.h bspLumps.I \
    bspRoot.h bspRoot.I \
    bspWorld.h bspWorld.I \
    imagePacker.h \
    lightmapPalettizer.h \
    load_bsp_file.h \
    loaderFileTypeBSP.h

  #define COMPOSITE_SOURCES \
    config_bsp.cxx \
    bspData.cxx \
    bspLoader.cxx \
    bspLumps.cxx \
    bspRoot.cxx \
    bspWorld.cxx \
    imagePacker.cxx \
    lightmapPalettizer.cxx \
    load_bsp_file.cxx \
    loaderFileTypeBSP.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define LOCAL_LIBS bsp
  #define TARGET extract-bsp-mat-names
  #define BUILD_TESTS 1
  #define SOURCES extract_bsp_mat_names.cxx
#end test_bin_target
