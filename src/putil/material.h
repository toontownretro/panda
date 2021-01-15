/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.h
 * @author lachbr
 * @date 2020-10-13
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "filename.h"
#include "pmap.h"
#include "lightMutex.h"
#include "config_putil.h"
#include "dSearchPath.h"
#include "luse.h"
#include "referenceCount.h"
#include "pointerTo.h"

class KeyValues;

/**
 * Main interface to the material script, which is a text file that describes
 * the material/render state of geometry.
 */
class EXPCL_PANDA_PUTIL Material : public ReferenceCount {
PUBLISHED:

  // These indicate what kind of state is specified in the script.
  enum States {
    S_none          = 0,
    S_texture       = 1 << 0,
    S_shader        = 1 << 1,
    S_fog_off       = 1 << 2,
    S_light_off     = 1 << 3,
    S_z_write       = 1 << 4,
    S_z_test        = 1 << 5,
    S_z_offset      = 1 << 6,
    S_cull_bin      = 1 << 7,
    S_alpha_test    = 1 << 8,
    S_transparency  = 1 << 9,
    S_render_mode   = 1 << 10,
    S_color         = 1 << 11,
    S_color_scale   = 1 << 12,
    S_color_write   = 1 << 13,
    S_cull_face     = 1 << 14,
    S_color_blend   = 1 << 15,
  };

  /**
   * Describes how to save path names for textures.
   */
  enum PathMode {
    // Leave the filenames as-is.
    PM_unchanged,
    // Write filename references relative to the directory the material is
    // being saved to.
    PM_relative,
    // Write an absolute filename from the root of the drive.
    PM_absolute,
  };

  enum CullFaceMode {
    CFM_none,
    CFM_clockwise,
    CFM_counter_clockwise,
  };

  enum TransparencyMode {
    TM_none,
    TM_alpha,
    TM_binary,
    TM_dual,
    TM_multisample,
  };

  enum ColorChannel {
    CC_off    = 0,
    CC_red    = 1,
    CC_green  = 2,
    CC_blue   = 4,
    CC_rgb    = (CC_red | CC_green | CC_blue),
    CC_alpha  = 8,
    CC_all    = (CC_red | CC_green | CC_blue | CC_alpha),
  };

  enum ColorBlendMode {
    CBM_none,
    CBM_additive,
    CBM_modulate,
  };

  enum ColorType {
    CT_none,
    CT_flat,
    CT_vertex,
  };

  /**
   * "texture" block data.
   */
  class ScriptTexture : public ReferenceCount {
  public:
    // Is the texture block referencing a file on disk or an engine-created
    // texture?
    enum Type {
      T_filename,
      T_engine,
    };

    INLINE ScriptTexture();
    INLINE ScriptTexture(const ScriptTexture &other);
    INLINE void clear();

    int compare_to(const ScriptTexture *other) const;

    Type _texture_type;

    Filename _filename;
    Filename _fullpath;
    std::string _name;

    std::string _stage_name;
    std::string _texcoord_name;

    bool _has_transform;
    LPoint3 _pos;
    LVector3 _hpr;
    LVector3 _scale;
  };

  typedef pvector<PT(ScriptTexture)> Textures;

  /**
   * "bin" block data.
   */
  class Bin {
  public:
    INLINE Bin();
    INLINE void clear();

    int compare_to(const Bin &other) const;

    std::string _name;
    int _sort;
  };

  /**
   * "alpha_test" block data.
   */
  class AlphaTest {
  public:
    enum Compare {
      C_none = 0,         // alpha-test disabled (always-draw)
      C_never,            // Never draw.
      C_less,             // incoming < reference_alpha
      C_equal,            // incoming == reference_alpha
      C_less_equal,       // incoming <= reference_alpha
      C_greater,          // incoming > reference_alpha
      C_not_equal,        // incoming != reference_alpha
      C_greater_equal,    // incoming >= reference_alpha
      C_always            // Always draw.
    };

    INLINE AlphaTest();
    INLINE void clear();

    int compare_to(const AlphaTest &other) const;

    float _reference;
    Compare _compare;
  };

  /**
   * "render_mode" block data.
   */
  class RenderMode {
  public:
    enum Mode {
      M_unchanged,
      M_filled,
      M_wireframe,
      M_point,
      M_filled_flat,
      M_filled_wireframe,
    };

    Mode _mode;
    float _thickness;
    LColor _wireframe_color;
    bool _perspective;
  };

  Material();

  void clear();

  bool resolve_filenames();

  int compare_to(const Material *other) const;

  void compose(const Material *other);

  static PT(Material) load(const Filename &filename,
                                 const DSearchPath &search_path = get_model_path());
  static PT(Material) parse(const std::string &data,
                                  const DSearchPath &search_path = get_model_path());
  void write(const Filename &filename, PathMode path_mode = PM_unchanged);

  INLINE static std::string get_extension();
  INLINE static std::string get_binary_extension();

  INLINE void set_filename(const Filename &filename);
  INLINE const Filename &get_filename() const;
  INLINE bool has_filename() const;
  INLINE void clear_filename();

  INLINE void set_fullpath(const Filename &fullpath);
  INLINE const Filename &get_fullpath() const;
  INLINE bool has_fullpath() const;
  INLINE void clear_fullpath();

  INLINE bool has_state(States states) const;

  INLINE void set_fog_off(bool flag);
  INLINE bool get_fog_off() const;
  INLINE bool has_fog_off() const;
  INLINE void clear_fog_off();

  INLINE void set_light_off(bool flag);
  INLINE bool get_light_off() const;
  INLINE bool has_light_off() const;
  INLINE void clear_light_off();

  INLINE void set_z_write(bool flag);
  INLINE bool get_z_write() const;
  INLINE bool has_z_write() const;
  INLINE void clear_z_write();

  INLINE void set_z_test(bool flag);
  INLINE bool get_z_test() const;
  INLINE bool has_z_test() const;
  INLINE void clear_z_test();

  INLINE void set_z_offset(int offset);
  INLINE int get_z_offset() const;
  INLINE bool has_z_offset() const;
  INLINE void clear_z_offset();

  INLINE void set_color(const LColor &color);
  INLINE void set_vertex_color();
  INLINE const LColor &get_color() const;
  INLINE bool has_color() const;
  INLINE void clear_color();

  INLINE ColorType get_color_type() const;

  INLINE void set_color_scale(const LColor &scale);
  INLINE const LColor &get_color_scale() const;
  INLINE bool has_color_scale() const;
  INLINE void clear_color_scale();

  INLINE void set_color_write(ColorChannel channels);
  INLINE ColorChannel get_color_write() const;
  INLINE bool has_color_write() const;
  INLINE void clear_color_write();

  INLINE void set_cull_face(CullFaceMode mode);
  INLINE CullFaceMode get_cull_face() const;
  INLINE bool has_cull_face() const;
  INLINE void clear_cull_face();

  INLINE void set_shader(const std::string &name);
  INLINE const std::string &get_shader() const;
  INLINE void set_param(const std::string &key, const std::string &value);
  INLINE bool has_param_key(const std::string &key) const;
  INLINE std::string get_param_value(const std::string &key) const;
  INLINE bool has_shader() const;
  INLINE void clear_shader();

  INLINE void set_bin(const std::string &name, int sort = 0);
  INLINE const std::string &get_bin_name() const;
  INLINE int get_bin_sort() const;
  INLINE bool has_bin() const;
  INLINE void clear_bin();

  INLINE void set_alpha_test(float reference, AlphaTest::Compare compare);
  INLINE float get_alpha_test_reference() const;
  INLINE AlphaTest::Compare get_alpha_test_compare() const;
  INLINE bool has_alpha_test() const;
  INLINE void clear_alpha_test();

  INLINE void set_transparency(TransparencyMode mode);
  INLINE TransparencyMode get_transparency() const;
  INLINE bool has_transparency() const;
  INLINE void clear_transparency();

  INLINE void add_texture(ScriptTexture *tex);
  INLINE size_t get_num_textures() const;
  INLINE bool has_texture(const std::string &name) const;
  INLINE ScriptTexture *get_texture(size_t n) const;
  INLINE ScriptTexture *get_texture(const std::string &name) const;
  INLINE void remove_texture(size_t n);
  INLINE void remove_texture(const std::string &name);
  INLINE bool has_textures() const;
  INLINE void clear_textures();

private:
  static bool parse_bool_string(const std::string &value);
  static bool is_true_string(const std::string &value);
  static void parse_texture_block(KeyValues *block, Material *script);
  static void parse_bin_block(KeyValues *block, Material *script);
  static void parse_alpha_test_block(KeyValues *block, Material *script);
  static void parse_render_mode_block(KeyValues *block, Material *script);
  static void parse_color_blend_block(KeyValues *block, Material *script);
  static void parse_color_write(const std::string &value, Material *script);

private:
  int _flags;

  bool _fog_off;
  bool _light_off;
  bool _enable_z_write;
  bool _enable_z_test;
  int _z_offset;
  ColorType _color_type;
  LColorf _color;
  LColorf _color_scale;
  ColorChannel _color_write;
  CullFaceMode _cull_face;
  std::string _shader;
  Bin _bin;
  AlphaTest _alpha_test;
  TransparencyMode _transparency;
  Textures _textures;
  ColorBlendMode _color_blend;

  pmap<std::string, std::string> _parameters;

  Filename _filename;
  Filename _fullpath;

  typedef pmap<Filename, PT(Material)> ScriptCache;
  static ScriptCache _cache;

  static LightMutex _mutex;

  friend class RenderState;
};

#include "material.I"

#endif // MATERIAL
