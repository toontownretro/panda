#include "textureStages.h"

//====================================================================//

TextureStages::tspool_t TextureStages::_stage_pool;

/**
 * Returns the texture stage with the given name,
 * or a new one if it doesn't already exists.
 */
TextureStage *TextureStages::
get(const std::string &name) {
  auto itr = _stage_pool.find(name);
  if (itr != _stage_pool.end()) {
    return itr->second;
  }

  // Texture stage with this name doesn't exist,
  // create it.

  PT(TextureStage) stage = new TextureStage(name);
  _stage_pool[name] = stage;

  return stage;
}

TextureStage *TextureStages::
get(const std::string &name, const std::string &uv_name) {
  auto itr = _stage_pool.find(name);
  if (itr != _stage_pool.end()) {
    return itr->second;
  }

  // Texture stage with this name doesn't exist,
  // create it. Also, assign the provided UV name.

  PT(TextureStage) stage = new TextureStage(name);
  stage->set_texcoord_name(uv_name);
  _stage_pool[name] = stage;

  return stage;
}

TextureStage *TextureStages::
get_basetexture() {
  return get("basetexture", "basetexture");
}

TextureStage *TextureStages::
get_lightmap() {
  return get("lightmap", "lightmap");
}

TextureStage *TextureStages::
get_bumped_lightmap() {
  return get("lightmap_bumped", "lightmap");
}

TextureStage *TextureStages::
get_spheremap() {
  return get("spheremap");
}

TextureStage *TextureStages::
get_cubemap() {
  return get("cubemap_tex");
}

TextureStage *TextureStages::
get_heightmap() {
  return get("heightmap");
}

TextureStage *TextureStages::
get_normalmap() {
  return get("normalmap");
}

TextureStage *TextureStages::
get_glossmap() {
  return get("glossmap");
}

TextureStage *TextureStages::
get_glowmap() {
  return get("glowmap");
}

//====================================================================//
