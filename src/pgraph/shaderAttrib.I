/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib.I
 * @author sshodhan
 * @date 2004-07-10
 * @author weifengh, PandaSE
 * @date 2010-04-15
 */

/**
 *
 */
INLINE ShaderAttrib::
ShaderAttrib() :
  _shader(nullptr),
  _shader_priority(0),
  // As of 10/30/2020, auto-shader is on by default.
  _auto_shader(true),
  _has_shader(true),
  // A NULL shader name means to use whatever shader is best for the material
  // type that is applied.
  _shader_name(nullptr),
  _flags(0),
  _has_flags(0),
  _num_transforms(0),
  _instance_count(0),
  _has_texture_inputs(false)
{
}

/**
 *
 */
INLINE ShaderAttrib::
ShaderAttrib(const ShaderAttrib &copy) :
  _shader(copy._shader),
  _shader_priority(copy._shader_priority),
  _auto_shader(copy._auto_shader),
  _has_shader(copy._has_shader),
  _flags(copy._flags),
  _has_flags(copy._has_flags),
  _num_transforms(copy._num_transforms),
  _instance_count(copy._instance_count),
  _inputs(copy._inputs),
  _shader_name(copy._shader_name),
  _texture_inputs(copy._texture_inputs),
  _has_texture_inputs(copy._has_texture_inputs)
{
}

/**
 * If true, the shader field of this attribute overrides the shader field of
 * the parent attribute.
 */
INLINE bool ShaderAttrib::
has_shader() const {
  return _has_shader;
}

/**
 * If true, then this ShaderAttrib does not contain an explicit shader -
 * instead, it requests the automatic generation of a shader.
 */
INLINE bool ShaderAttrib::
auto_shader() const {
  return _auto_shader;
}

/**
 *
 */
INLINE int ShaderAttrib::
get_shader_priority() const {
  return _shader_priority;
}

/**
 * Returns the number of geometry instances.  A value of 0 means not to use
 * instancing at all.  This value is ignored if F_hardware_instancing is set.
 */
INLINE int ShaderAttrib::
get_instance_count() const {
  return _instance_count;
}

/**
 *
 */
INLINE bool ShaderAttrib::
get_flag(int flag) const {
  return (_flags & flag) != 0;
}

/**
 * Returns true if there is a ShaderInput of the given name.
 */
INLINE bool ShaderAttrib::
has_shader_input(const InternalName *id) const {
  return (_inputs.find(id) != _inputs.end());
}

/**
 * Returns true if there is a ShaderInput of the given name.
 */
INLINE bool ShaderAttrib::
has_shader_input(const std::string &id) const {
  return has_shader_input(InternalName::make(id));
}

/**
 * Returns the number of ShaderInputs.
 */
INLINE size_t ShaderAttrib::
get_num_shader_inputs() const {
  return _inputs.size();
}

/**
 * Returns the name of the shader generator that should be used to generate a
 * shader for this state.
 */
INLINE const InternalName *ShaderAttrib::
get_shader_name() const {
  return _shader_name;
}

/**
 *
 */
INLINE bool ShaderAttrib::
has_hardware_skinning() const {
  return (_flags & F_hardware_skinning) != 0;
}

/**
 *
 */
INLINE int ShaderAttrib::
get_num_transforms() const {
  return _num_transforms;
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_float &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_double &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LVecBase4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LVecBase3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}


/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LVecBase2 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LVecBase4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LVecBase3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LVecBase2 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LMatrix4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LMatrix3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LMatrix4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LMatrix3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, Texture *tex, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), tex, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const NodePath &np, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), np, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, double n1, double n2, double n3, double n4, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), LVecBase4((PN_stdfloat)n1, (PN_stdfloat)n2, (PN_stdfloat)n3, (PN_stdfloat)n4), priority));
}
