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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_vector(LCAST(PN_stdfloat, vec)),
  _stored_ptr(vec),
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
  _stored_vector(vec.get_x(), vec.get_y(), vec.get_z(), 0.0),
  _stored_ptr(vec),
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
  _stored_vector(vec.get_x(), vec.get_y(), 0.0, 0.0),
  _stored_ptr(vec),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_matrix(mat),
  _stored_ptr(mat),
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
  _stored_matrix(mat),
  _stored_ptr(mat),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_vector(LCAST(PN_stdfloat, vec)),
  _stored_ptr(vec),
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
  _stored_vector(vec.get_x(), vec.get_y(), vec.get_z(), 0.0),
  _stored_ptr(vec),
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
  _stored_vector(vec.get_x(), vec.get_y(), 0.0, 0.0),
  _stored_ptr(vec),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_matrix(LCAST(PN_stdfloat, mat)),
  _stored_ptr(mat),
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
  _stored_matrix(LCAST(PN_stdfloat, mat)),
  _stored_ptr(mat),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_ptr(ptr),
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
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), (int)vec.get_z(), (int)vec.get_w()),
  _stored_ptr(vec),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3i &vec, int priority) :
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), (int)vec.get_z(), 0.0),
  _stored_ptr(vec),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2i &vec, int priority) :
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), 0.0, 0.0),
  _stored_ptr(vec),
  _name(std::move(name)),
  _priority(priority),
  _type(M_vector)
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
    return _stored_vector == other._stored_vector;

  case M_matrix:
    return _stored_matrix == other._stored_matrix;

  case M_numeric:
    return _stored_ptr._ptr == other._stored_ptr._ptr;

  case M_texture_sampler:
    return DCAST(ParamTextureSampler, _value)->get_texture() == DCAST(ParamTextureSampler, other._value)->get_texture() &&
           DCAST(ParamTextureSampler, _value)->get_sampler() == DCAST(ParamTextureSampler, other._value)->get_sampler();

  default:
    return _value == other._value;
  }
}

/**
 *
 */
INLINE bool ShaderInput::
operator != (const ShaderInput &other) const {
  if (_type != other._type || _name != other._name || _priority != other._priority) {
    return true;
  }
  switch (_type) {
  case M_invalid:
    return false;

  case M_vector:
    return _stored_vector != other._stored_vector;

  case M_matrix:
    return _stored_matrix != other._stored_matrix;

  case M_numeric:
    return _stored_ptr._ptr != other._stored_ptr._ptr;

  case M_texture_sampler:
    return DCAST(ParamTextureSampler, _value)->get_texture() != DCAST(ParamTextureSampler, other._value)->get_texture() ||
           DCAST(ParamTextureSampler, _value)->get_sampler() != DCAST(ParamTextureSampler, other._value)->get_sampler();

  default:
    return _value != other._value;
  }
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
    if (_stored_vector[0] != other._stored_vector[0]) {
      return _stored_vector[0] < other._stored_vector[0];
    }
    if (_stored_vector[1] != other._stored_vector[1]) {
      return _stored_vector[1] < other._stored_vector[1];
    }
    if (_stored_vector[2] != other._stored_vector[2]) {
      return _stored_vector[2] < other._stored_vector[2];
    }
    if (_stored_vector[3] != other._stored_vector[3]) {
      return _stored_vector[3] < other._stored_vector[3];
    }
    return false;

  case M_matrix:
    {
      int cmp = _stored_matrix.compare_to(other._stored_matrix);
      return cmp < 0;
    }

  case M_numeric:
    return _stored_ptr._ptr < other._stored_ptr._ptr;

  case M_texture_sampler:
    if (DCAST(ParamTextureSampler, _value)->get_texture() != DCAST(ParamTextureSampler, other._value)->get_texture()) {
      return DCAST(ParamTextureSampler, _value)->get_texture() < DCAST(ParamTextureSampler, other._value)->get_texture();
    }
    if (DCAST(ParamTextureSampler, _value)->get_sampler() != DCAST(ParamTextureSampler, other._value)->get_sampler()) {
      return DCAST(ParamTextureSampler, _value)->get_sampler() < DCAST(ParamTextureSampler, other._value)->get_sampler();
    }
    return false;

  default:
    return _value < other._value;
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
  return _stored_vector;
}

/**
 *
 */
INLINE const LMatrix4 &ShaderInput::
get_matrix() const {
  return _stored_matrix;
}

/**
 *
 */
INLINE const Shader::ShaderPtrData &ShaderInput::
get_ptr() const {
  return _stored_ptr;
}

/**
 *
 */
INLINE ParamValueBase *ShaderInput::
get_param() const {
  return DCAST(ParamValueBase, _value);
}

/**
 *
 */
INLINE TypedWritableReferenceCount *ShaderInput::
get_value() const {
  return _value.p();
}
