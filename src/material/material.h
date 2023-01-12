/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.h
 * @author brian
 * @date 2021-03-06
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "pointerTo.h"
#include "texture.h"
#include "luse.h"
#include "pmap.h"
#include "internalName.h"
#include "materialParamBase.h"
#include "config_putil.h"
#include "simpleHashMap.h"
#include "vector_string.h"
#include "pvector.h"

class PDXElement;

/**
 * This is an abstract base class for any kind of material that can be applied
 * to a surface.  At its core, a material can have a name and a number of
 * named parameters.  It is up to derived material types to expose the
 * parameters that can be set for that particular material.
 */
class EXPCL_PANDA_MATERIAL Material : public TypedWritableReferenceCount, public Namable {
public:
  Material(const std::string &name);

  virtual void read_pdx(PDXElement *data, const DSearchPath &search_path);
  virtual void write_pdx(PDXElement *data, const Filename &filename);

PUBLISHED:
  enum Flags {
    F_none = 0,
    // Flags for what render state modifying attributes are specified in the
    // material.  The render state is created in MaterialAttrib in libpgraph
    // because libpgraph depends on libmaterial, and we don't want to create
    // circular dependencies.
    F_transparency = 1 << 0,
    F_color = 1 << 1,
    F_alpha_test = 1 << 2,
    F_cull_face = 1 << 3,
    F_depth_write = 1 << 4,
    F_depth_test = 1 << 5,
    F_light = 1 << 6,
    F_fog = 1 << 7,
    F_bin = 1 << 8,
    F_render_mode = 1 << 9,
    F_color_blend = 1 << 10,
    F_color_scale = 1 << 11,
  };

  INLINE size_t get_num_params() const;
  INLINE MaterialParamBase *get_param(size_t n) const;
  INLINE MaterialParamBase *get_param(CPT_InternalName name) const;

  INLINE void set_filename(const Filename &filename);
  INLINE const Filename &get_filename() const;
  MAKE_PROPERTY(filename, get_filename, set_filename);

  INLINE void set_fullpath(const Filename &fullpath);
  INLINE const Filename &get_fullpath() const;
  MAKE_PROPERTY(fullpath, get_fullpath, set_fullpath);

  INLINE void set_tag(const std::string &tag, std::string &value);
  INLINE void clear_tag(const std::string &tag);
  INLINE void clear_tag(int n);
  INLINE bool has_tag(const std::string &tag) const;
  INLINE int get_num_tags() const;
  INLINE std::string get_tag_value(const std::string &key) const;
  INLINE std::string get_tag_key(int n) const;
  INLINE std::string get_tag_value(int n) const;

  void write_pmat(const Filename &filename);
  bool write_mto(const Filename &filename);

  INLINE void set_param(MaterialParamBase *param);
  INLINE void clear_param(MaterialParamBase *param);

public:
  Filename _filename;
  Filename _fullpath;

  typedef SimpleHashMap<CPT_InternalName, PT(MaterialParamBase)> Params;
  Params _params;

  typedef SimpleHashMap<std::string, std::string, string_hash> Tags;
  Tags _tags;

  unsigned int _attrib_flags;

  std::string _bin_name;
  int _bin_sort;

  LColor _color;
  LColor _color_scale;

  int _transparency_mode;

  int _alpha_test_mode;
  PN_stdfloat _alpha_test_ref;

  int _cull_face_mode;

  bool _depth_write;
  int _depth_test_mode;

  bool _light_off;
  bool _fog_off;

  // Filled, wireframe, etc.
  int _render_mode;

  int _color_blend_mode;

private:
  // Only used during Bam reading.
  int _num_params;
  bool _read_rawdata;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "Material",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


#include "material.I"

#endif // MATERIAL_H
