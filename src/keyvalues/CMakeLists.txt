set(P3KEYVALUES_HEADERS
  config_keyvalues.h
  keyValues.h
)

set(P3KEYVALUES_SOURCES
  config_keyvalues.cxx
  keyValues.cxx
)

composite_sources(p3keyvalues P3KEYVALUES_SOURCES)
add_component_library(p3keyvalues SYMBOL BUILDING_VIFPARSER
  ${P3KEYVALUES_HEADERS} ${P3KEYVALUES_SOURCES})
target_link_libraries(p3keyvalues p3putil p3linmath pandaexpress)
target_interrogate(p3keyvalues ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3keyvalues
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3KEYVALUES_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
