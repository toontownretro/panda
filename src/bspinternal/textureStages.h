#pragma once

#include "config_bspinternal.h"
#include "pmap.h"
#include "pointerTo.h"
#include "textureStage.h"

/**
 * This simple interface maintains a single TextureStage object for each unique name.
 * It avoids the creation of duplicate TextureStages with the same name, which
 * reduces texture swapping and draw call overhead.
 *
 * If using our shader system, you should always use this interface to get TextureStages.
 * You are not required to change any properties on the returned TextureStage, as the shader
 * specification will know what to do with the TextureStage from the name.
 *
 * For example, you do not need to call TextureStage::set_mode() or NodePath::set_tex_gen().
 * If you apply a texture to a node with the get_normalmap() stage, the shader specification
 * will know that the texture you supplied is to be treated as a normal map.
 */
class EXPCL_BSPINTERNAL TextureStages {
PUBLISHED:
  static TextureStage *get(const std::string &name);
  static TextureStage *get(const std::string &name, const std::string &uv_name);

  static TextureStage *get_basetexture();
  static TextureStage *get_lightmap();
  static TextureStage *get_bumped_lightmap();
  static TextureStage *get_spheremap();
  static TextureStage *get_cubemap();
  static TextureStage *get_normalmap();
  static TextureStage *get_heightmap();
  static TextureStage *get_glossmap();
  static TextureStage *get_glowmap();

private:
  typedef pmap<std::string, PT(TextureStage)> tspool_t;
  static tspool_t _stage_pool;
};
