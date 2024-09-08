/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInput.I
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 */

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, int priority) :
  _name(std::move(name)),
  _priority(priority),
  _type(M_invalid)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, int priority) :
  _name(std::move(name)),
  _value(tex),
  _priority(priority),
  _type(M_texture)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, ParamValueBase *param, int priority) :
  _name(std::move(name)),
  _value(param),
  _priority(priority),
  _type(M_param)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, ShaderBuffer *buf, int priority) :
  _name(std::move(name)),
  _value(buf),
  _priority(priority),
  _type(M_buffer)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_float &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4f &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3f &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2f &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4f &vec, int priority) :
  _value(LCAST(PN_stdfloat, vec)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3f &vec, int priority) :
  _value(LVecBase4(vec.get_x(), vec.get_y(), vec.get_z(), 0.0)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2f &vec, int priority) :
  _value(LVecBase4(vec.get_x(), vec.get_y(), 0.0, 0.0)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix4f &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix3f &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix4f &mat, int priority) :
  _value(mat),
  _name(std::move(name)),
  _priority(priority),
  _type(M_matrix)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix3f &mat, int priority) :
  _value(LMatrix4(mat)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_matrix)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_double &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4d &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3d &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2d &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4d &vec, int priority) :
  _value(LCAST(PN_stdfloat, vec)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3d &vec, int priority) :
  _value(LVecBase4(vec.get_x(), vec.get_y(), vec.get_z(), 0.0)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2d &vec, int priority) :
  _value(LVecBase4(vec.get_x(), vec.get_y(), 0.0, 0.0)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix4d &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix3d &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix4d &mat, int priority) :
  _value(LCAST(PN_stdfloat, mat)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_matrix)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix3d &mat, int priority) :
  _value(LMatrix4(LCAST(PN_stdfloat, mat))),
  _name(std::move(name)),
  _priority(priority),
  _type(M_matrix)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_int &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4i &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3i &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2i &ptr, int priority) :
  _value(Shader::ShaderPtrData(ptr)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4i &vec, int priority) :
  _value(Shader::ShaderPtrData(vec)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3i &vec, int priority) :
  _value(Shader::ShaderPtrData(vec)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2i &vec, int priority) :
  _value(Shader::ShaderPtrData(vec)),
  _name(std::move(name)),
  _priority(priority),
  _type(M_numeric)
{
}

/**
 *
 */
INLINE ShaderInput::
operator bool () const {
  return _type != M_invalid;
}

/**
 *
 */
INLINE bool ShaderInput::
operator == (const ShaderInput &other) const {
  if (_type != other._type || _name != other._name || _priority != other._priority) {
    return false;
  }
  switch (_type) {
  case M_invalid:
    return true;

  case M_vector:
    return std::get<LVecBase4>(_value) == std::get<LVecBase4>(other._value);

  case M_matrix:
    return std::get<LMatrix4>(_value) == std::get<LMatrix4>(other._value);

  case M_numeric:
    return std::get<Shader::ShaderPtrData>(_value)._ptr == std::get<Shader::ShaderPtrData>(other._value)._ptr;

  case M_texture_sampler:
    {
      const TexSampPair *pts = &std::get<TexSampPair>(_value);
      const TexSampPair *opts = &std::get<TexSampPair>(other._value);
      return pts->_texture == opts->_texture &&
        pts->_samp == opts->_samp;
    }

  default:
    return std::get<PT(TypedWritableReferenceCount)>(_value) == std::get<PT(TypedWritableReferenceCount)>(other._value);
  }
}

/**
 *
 */
INLINE bool ShaderInput::
operator != (const ShaderInput &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool ShaderInput::
operator < (const ShaderInput &other) const {
  if (_type != other._type) {
    return (_type < other._type);
  }
  if (_name != other._name) {
    return (_name < other._name);
  }
  if (_priority != other._priority) {
    return (_priority < other._priority);
  }
  switch (_type) {
  case M_invalid:
    return false;

  case M_vector:
    {
      const LVecBase4 &v = std::get<LVecBase4>(_value);
      const LVecBase4 &ov = std::get<LVecBase4>(other._value);
      if (v[0] != ov[0]) {
        return v[0] < ov[0];
      }
      if (v[1] != ov[1]) {
        return v[1] < ov[1];
      }
      if (v[2] != ov[2]) {
        return v[2] < ov[2];
      }
      if (v[3] != ov[3]) {
        return v[3] < ov[3];
      }
      return false;
    }

  case M_matrix:
    {
      const LMatrix4 &m = std::get<LMatrix4>(_value);
      const LMatrix4 &om = std::get<LMatrix4>(other._value);
      int cmp = m.compare_to(om);
      return cmp < 0;
    }

  case M_numeric:
    return std::get<Shader::ShaderPtrData>(_value)._ptr < std::get<Shader::ShaderPtrData>(other._value)._ptr;

  case M_texture_sampler:
    {
      const TexSampPair *pts = &std::get<TexSampPair>(_value);
      const TexSampPair *opts = &std::get<TexSampPair>(other._value);
      if (pts->get_texture() != opts->get_texture()) {
        return pts->get_texture() < opts->get_texture();
      }
      if (pts->get_sampler() != opts->get_sampler()) {
        return pts->get_sampler() < opts->get_sampler();
      }
      return false;
    }

  default:
    return std::get<PT(TypedWritableReferenceCount)>(_value) < std::get<PT(TypedWritableReferenceCount)>(other._value);
  }
}

/**
 *
 */
INLINE const InternalName *ShaderInput::
get_name() const {
  return _name;
}

/**
 *
 */
INLINE int ShaderInput::
get_value_type() const {
  return _type;
}

/**
 *
 */
INLINE int ShaderInput::
get_priority() const {
  return _priority;
}

/**
 *
 */
INLINE const LVecBase4 &ShaderInput::
get_vector() const {
  return std::get<LVecBase4>(_value);
}

/**
 *
 */
INLINE const LMatrix4 &ShaderInput::
get_matrix() const {
  return std::get<LMatrix4>(_value);
}

/**
 *
 */
INLINE const Shader::ShaderPtrData &ShaderInput::
get_ptr() const {
  return std::get<Shader::ShaderPtrData>(_value);
}

/**
 *
 */
INLINE ParamValueBase *ShaderInput::
get_param() const {
  return DCAST(ParamValueBase, std::get<PT(TypedWritableReferenceCount)>(_value));
}

/**
 *
 */
INLINE TypedWritableReferenceCount *ShaderInput::
get_value() const {
  return std::get<PT(TypedWritableReferenceCount)>(_value).p();
}
