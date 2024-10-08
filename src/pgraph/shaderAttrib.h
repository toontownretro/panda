/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib.h
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 *   for set_shader_input)
 * @author weifengh, PandaSE
 * @date 2010-04-15
 */

#ifndef SHADERATTRIB_H
#define SHADERATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"
#include "pointerTo.h"
#include "shaderInput.h"
#include "shader.h"
#include "pta_float.h"
#include "pta_double.h"
#include "pta_LMatrix4.h"
#include "pta_LMatrix3.h"
#include "pta_LVecBase4.h"
#include "pta_LVecBase3.h"
#include "pta_LVecBase2.h"
#include "extension.h"
#include "ordered_vector.h"

/**
 *
 */
class EXPCL_PANDA_PGRAPH ShaderAttrib final : public RenderAttrib {
private:
  INLINE ShaderAttrib();
  INLINE ShaderAttrib(const ShaderAttrib &copy);

PUBLISHED:
  static CPT(RenderAttrib) make(const Shader *shader = nullptr, int priority = 0);
  static CPT(RenderAttrib) make(const std::string &shader_name, int priority = 0);
  static CPT(RenderAttrib) make(const Shader *shader, pvector<ShaderInput> &&inputs,
                                int flags, int instance_count);
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  enum {
    F_disable_alpha_write = 1 << 0,  // Suppress writes to color buffer alpha channel.
    F_subsume_alpha_test  = 1 << 1,  // Shader promises to subsume the alpha test using TEXKILL
    F_hardware_skinning   = 1 << 2,  // Shader needs pre-animated vertices
    F_shader_point_size   = 1 << 3,  // Shader provides point size, not RenderModeAttrib
    F_hardware_instancing = 1 << 4,  // Shader needs instance list
  };

  INLINE bool               has_shader() const;
  INLINE bool               auto_shader() const;
  INLINE int                get_shader_priority() const;
  INLINE int                get_instance_count() const;

  CPT(RenderAttrib) set_shader(const Shader *s, int priority = 0) const;
  CPT(RenderAttrib) set_shader(const std::string &shader_name, int priority = 0) const;
  CPT(RenderAttrib) set_shader_off(int priority=0) const;

  CPT(RenderAttrib) clear_shader() const;
  // Shader Inputs
  CPT(RenderAttrib) set_shader_input(const ShaderInput &input) const;
  CPT(RenderAttrib) set_shader_input(ShaderInput &&input) const;

  CPT(RenderAttrib) copy_shader_inputs_from(const ShaderAttrib *other) const;

public:
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, Texture *tex,       int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const NodePath &np, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_float &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_double &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LMatrix4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LMatrix3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LVecBase4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LVecBase3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const PTA_LVecBase2 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LVecBase4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LVecBase3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LVecBase2 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LMatrix4 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, const LMatrix3 &v, int priority=0) const;
  INLINE CPT(RenderAttrib) set_shader_input(CPT_InternalName id, double n1=0, double n2=0, double n3=0, double n4=1,
                                            int priority=0) const;

  CPT(RenderAttrib) set_shader_inputs(const pvector<ShaderInput> &inputs) const;

PUBLISHED:
  PY_EXTENSION(CPT(RenderAttrib) set_shader_input(CPT_InternalName, PyObject *, int priority=0) const);
  PY_EXTENSION(CPT(RenderAttrib) set_shader_inputs(PyObject *args, PyObject *kwargs) const);

  CPT(RenderAttrib) set_instance_count(int instance_count) const;

  CPT(RenderAttrib) set_flag(int flag, bool value) const;
  CPT(RenderAttrib) clear_flag(int flag) const;

  CPT(RenderAttrib) set_hardware_skinning(bool flag, int num_transforms = 4) const;

  CPT(RenderAttrib) clear_shader_input(const InternalName *id) const;
  CPT(RenderAttrib) clear_shader_input(const std::string &id) const;

  CPT(RenderAttrib) clear_all_shader_inputs() const;

  INLINE bool get_flag(int flag) const;
  INLINE bool has_shader_input(const InternalName *id) const;
  INLINE bool has_shader_input(const std::string &id) const;

  INLINE const Shader *get_shader() const;
  INLINE const ShaderInput &get_shader_input(const InternalName *id) const;
  INLINE const ShaderInput &get_shader_input(const std::string &id) const;
  INLINE size_t get_num_shader_inputs() const;

  INLINE const InternalName *get_shader_name() const;

  INLINE bool has_hardware_skinning() const;
  INLINE int get_num_transforms() const;

  const NodePath &get_shader_input_nodepath(const InternalName *id) const;
  LVecBase4 get_shader_input_vector(const InternalName *id) const;
  Texture *get_shader_input_texture(const InternalName *id, const SamplerState *&sampler) const;
  const Shader::ShaderPtrData *get_shader_input_ptr(const InternalName *id) const;
  bool get_shader_input_ptr(const InternalName *id, Shader::ShaderPtrData &data) const;
  const LMatrix4 &get_shader_input_matrix(const InternalName *id, LMatrix4 &matrix) const;
  ShaderBuffer *get_shader_input_buffer(const InternalName *id) const;

PUBLISHED:
  MAKE_PROPERTY(shader, get_shader);
  MAKE_PROPERTY(instance_count, get_instance_count);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

public:
  void build_texture_inputs();

private:

  CPT(Shader) _shader;
  int         _shader_priority;
  CPT(InternalName) _shader_name;
  bool        _auto_shader;
  bool        _has_shader;
  int         _flags;
  int         _num_transforms;
  int         _has_flags;
  int         _instance_count;

  // We don't keep a reference to the InternalName, since this is also already
  // stored on the ShaderInput object.
  typedef pvector<ShaderInput> Inputs;
  Inputs _inputs;

  INLINE Inputs::iterator find_input(const InternalName *name);
  INLINE Inputs::const_iterator find_input(const InternalName *name) const;
  INLINE void insert_input(const ShaderInput &input);
  INLINE void insert_input(ShaderInput &&input);

public:
  bool _has_texture_inputs;
  typedef pflat_map<const InternalName *, Texture *> TextureInputs;
  TextureInputs _texture_inputs;

  friend class Extension<NodePath>;
  friend class Extension<ShaderAttrib>;

  friend class CullBinStateSorted;

PUBLISHED:
  INLINE static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "ShaderAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 10, new ShaderAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};


#include "shaderAttrib.I"

#endif  // !SHADERATTRIB_H
