if(NOT HAVE_BULLET)
  return()
endif()

set(PANDABSP_HEADERS
  config_bsp.h
  #actor.h
  ambient_boost_effect.h
  ambient_probes.h
  audio_3d_manager.h
  aux_data_attrib.h
  bloom_attrib.h
  bounding_kdop.h
  bsp_render.h
  bsp_trace.h
  bsploader.h
  ciolib.h
  cubemaps.h
  decals.h
  entity.h
  glow_node.h
  interpolated.h
  interpolatedvar.h
  lerp_functions.h
  lighting_origin_effect.h
  lightmap_palettes.h
  physics_character_controller.h
  planar_reflections.h
  pssmCameraRig.h
  py_bsploader.h
  rangecheckedvar.h
  raytrace.h
  shader_csmrender.h
  shader_decalmodulate.h
  shader_features.h
  shader_generator.h
  shader_lightmappedgeneric.h
  shader_skybox.h
  shader_spec.h
  shader_unlitgeneric.h
  shader_unlitnomat.h
  shader_vertexlitgeneric.h
  static_props.h
  subprocess.h
  texture_filter.h
  TexturePacker.h

  kdtree/KDTree.h
  kdtree/utils.h

  postprocess/bloom.h
  postprocess/blur_passes.h
  postprocess/fxaa.h
  postprocess/hdr.h
  postprocess/postprocess.h
  postprocess/postprocess_defines.h
  postprocess/postprocess_effect.h
  postprocess/postprocess_pass.h
  postprocess/postprocess_scene_pass.h
  postprocess/ssao.h
)

set(PANDABSP_SOURCES
  config_bsp.cpp
  ambient_boost_effect.cpp
  ambient_probes.cpp
  audio_3d_manager.cpp
  aux_data_attrib.cpp
  bloom_attrib.cpp
  bounding_kdop.cpp
  bsp_render.cpp
  bsp_trace.cpp
  bsploader.cpp
  ciolib.cpp
  decals.cpp
  entity.cpp
  glow_node.cpp
  interpolated.cpp
  interpolatedvar.cpp
  lighting_origin_effect.cpp
  lightmap_palettes.cpp
  physics_character_controller.cpp
  planar_reflections.cpp
  pssmCameraRig.cpp
  py_bsploader.cpp
  rangecheckedvar.cpp
  raytrace.cpp
  shader_csmrender.cpp
  shader_decalmodulate.cpp
  shader_features.cpp
  shader_generator.cpp
  shader_lightmappedgeneric.cpp
  shader_skybox.cpp
  shader_spec.cpp
  shader_unlitgeneric.cpp
  shader_unlitnomat.cpp
  shader_vertexlitgeneric.cpp
  static_props.cpp
  texture_filter.cpp
  TexturePacker.cpp

  kdtree/KDTree.cpp

  postprocess/bloom.cpp
  postprocess/fxaa.cpp
  postprocess/hdr.cpp
  postprocess/postprocess.cpp
  postprocess/postprocess_effect.cpp
  postprocess/postprocess_pass.cpp
  postprocess/postprocess_scene_pass.cpp
  postprocess/ssao.cpp
)

composite_sources(pandabsp PANDABSP_SOURCES)
add_library(pandabsp ${PANDABSP_SOURCES} ${PANDABSP_HEADERS})
set_target_properties(pandabsp PROPERTIES DEFINE_SYMBOL BUILDING_LIBPANDABSP)
target_link_libraries(pandabsp panda pandaegg pandabullet pandagl PKG::BULLET)
target_interrogate(pandabsp ALL)

install(TARGETS pandabsp
  EXPORT PandaBSP COMPONENT PandaBSP
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT PandaBSPDevel)
install(FILES ${PANDABSP_HEADERS} COMPONENT PandaBSPDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(PandaBSP COMPONENT PandaBSPDevel)
