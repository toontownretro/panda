set(P3BSPINT_HEADERS
  config_bspinternal.h
  bspMaterial.h
  bspMaterialAttrib.h
  textureStages.h
)

set(P3BSPINT_SOURCES
  config_bspinternal.cxx
  bspMaterial.cxx
  bspMaterialAttrib.cxx
  textureStages.cxx
)

composite_sources(p3bspinternal P3BSPINT_SOURCES)
add_component_library(p3bspinternal SYMBOL BUILDING_BSPINTERNAL
  ${P3BSPINT_HEADERS} ${P3BSPINT_SOURCES})
target_link_libraries(p3bspinternal p3putil p3linmath pandaexpress p3gobj p3pgraph p3keyvalues)
# NOTE: While the CXX code is part of libpanda, the interrogated code is compiled
# into panda3d.bsp to remain consistent.
target_interrogate(p3bspinternal ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3bspinternal
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3BSPINT_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
