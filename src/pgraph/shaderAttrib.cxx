/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib.cxx
 * @author sshodhan
 * @date 2004-07-10
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 *   for set_shader_input)
 * @author weifengh, PandaSE
 * @date 2010-04-15
 *   set_shader_auto)
 */

#include "pandabase.h"
#include "shaderAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "nodePath.h"
#include "paramNodePath.h"
#include "paramTexture.h"
#include "shaderBuffer.h"
#include "string_utils.h"

using std::ostream;
using std::ostringstream;

TypeHandle ShaderAttrib::_type_handle;
int ShaderAttrib::_attrib_slot;

/**
 * Constructs a new ShaderAttrib object that disables the use of shaders (it
 * does not clear out all shader data, however.)
 */
CPT(RenderAttrib) ShaderAttrib::
make_off() {
  static CPT(RenderAttrib) _off_attrib;
  if (_off_attrib == nullptr) {
    ShaderAttrib *attrib = new ShaderAttrib;
    attrib->_has_shader = true;
    _off_attrib = return_new(attrib);
  }
  return _off_attrib;
}

/**
 * Constructs a new ShaderAttrib object with nothing set.
 */
CPT(RenderAttrib) ShaderAttrib::
make(const Shader *shader, int priority) {
  static CPT(RenderAttrib) _null_attrib;
  if (_null_attrib == nullptr) {
    ShaderAttrib *attrib = new ShaderAttrib;
    _null_attrib = return_new(attrib);
  }

  if (shader == nullptr) {
    return _null_attrib;
  } else {
    return DCAST(ShaderAttrib, _null_attrib)->set_shader(shader, priority);
  }
}

/**
 * Constructs a new ShaderAttrib that indicates the name of the shader
 * generator that should be used to generate a shader for the state.
 */
CPT(RenderAttrib) ShaderAttrib::
make(const std::string &shader_name, int priority) {
  ShaderAttrib *attr = new ShaderAttrib;
  attr->_shader_name = InternalName::make(downcase(shader_name));
  attr->_shader_priority = priority;
  attr->_auto_shader = true;
  attr->_has_shader = true;
  return return_new(attr);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
make(const Shader *shader, pvector<ShaderInput> &&inputs, int flags, int instance_count) {
  ShaderAttrib *attr = new ShaderAttrib;
  attr->_shader = shader;
  for (size_t i = 0; i < inputs.size(); ++i) {
    attr->insert_input(std::move(inputs[i]));
  }
  attr->_has_shader = true;
  attr->_flags = flags;
  attr->_has_flags = true;
  attr->_instance_count = instance_count;
  attr->build_texture_inputs();
  return return_new(attr);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) ShaderAttrib::
make_default() {
  return return_new(new ShaderAttrib);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader(const Shader *s, int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = s;
  result->_shader_priority = priority;
  result->_auto_shader = false;
  result->_has_shader = true;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader(const std::string &shader_name, int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader_name = InternalName::make(downcase(shader_name));
  result->_shader = nullptr;
  result->_shader_priority = priority;
  result->_auto_shader = true;
  result->_has_shader = true;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_off(int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = nullptr;
  result->_shader_priority = priority;
  result->_auto_shader = false;
  result->_has_shader = true;
  result->_shader_name = nullptr;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_shader() const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = nullptr;
  result->_shader_priority = 0;
  result->_auto_shader = false;
  result->_has_shader = false;
  result->_shader_name = nullptr;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_flag(int flag, bool value) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  if (value) {
    result->_flags |= flag;
  } else {
    result->_flags &= ~flag;
  }
  result->_has_flags |= flag;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_flag(int flag) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_flags &= ~flag;
  result->_has_flags &= ~flag;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_hardware_skinning(bool flag, int num_transforms) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  if (flag) {
    result->_flags |= F_hardware_skinning;
  } else {
    result->_flags &= ~F_hardware_skinning;
  }
  result->_num_transforms = num_transforms;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_input(const ShaderInput &input) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->insert_input(input);
  result->build_texture_inputs();
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_input(ShaderInput &&input) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->insert_input(std::move(input));
  result->build_texture_inputs();
  return return_new(result);
}

/**
 * Returns a new ShaderAttrib with the ShaderInputs copied in from the other
 * ShaderAttrib.
 */
CPT(RenderAttrib) ShaderAttrib::
copy_shader_inputs_from(const ShaderAttrib *other) const {
  ShaderAttrib *result = new ShaderAttrib(*this);

  Inputs::const_iterator i = other->_inputs.begin();
  for (; i != other->_inputs.end(); i++) {
    result->insert_input((*i));
  }

  result->build_texture_inputs();

  return return_new(result);
}

/**
 * Returns a new ShaderAttrib with the given shader inputs set.  This is a
 * more efficient way to set multiple shader inputs than calling
 * set_shader_input multiple times.
 */
CPT(RenderAttrib) ShaderAttrib::
set_shader_inputs(const pvector<ShaderInput> &inputs) const {
  ShaderAttrib *result = new ShaderAttrib(*this);

  size_t num_inputs = inputs.size();
  for (size_t i = 0; i < num_inputs; i++) {
    result->insert_input(inputs[i]);
  }

  result->build_texture_inputs();

  return return_new(result);
}

/**
 * Sets the geometry instance count.  Do not confuse this with instanceTo,
 * which is used for animation instancing, and has nothing to do with this.  A
 * value of 0 means not to use instancing at all.
 *
 * This value should not be set if F_hardware_instancing is also set.
 */
CPT(RenderAttrib) ShaderAttrib::
set_instance_count(int instance_count) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_instance_count = instance_count;
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_shader_input(const InternalName *id) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  Inputs::iterator it = result->find_input(id);
  if (it != result->_inputs.end()) {
    result->_inputs.erase(it);
  }
  result->build_texture_inputs();
  return return_new(result);
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
clear_shader_input(const std::string &id) const {
  return clear_shader_input(InternalName::make(id));
}

/**
 * Clears all the shader inputs on the attrib.
 */
CPT(RenderAttrib) ShaderAttrib::
clear_all_shader_inputs() const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_inputs.clear();
  result->_texture_inputs.clear();
  result->_has_texture_inputs = true;
  return return_new(result);
}

/**
 * Returns the ShaderInput as a nodepath.  Assertion fails if there is none,
 * or if it is not a nodepath.
 */
const NodePath &ShaderAttrib::
get_shader_input_nodepath(const InternalName *id) const {
  static NodePath resfail;
  Inputs::const_iterator i = find_input(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i);
    if (p.get_value_type() == ShaderInput::M_nodepath) {
      return ((const ParamNodePath *)p.get_value())->get_value();
    } else {
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a nodepath.\n";
      nassert_raise(strm.str());
      return resfail;
    }
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return resfail;
  }

  // Satisfy compiler.
  return resfail;
}

/**
 * Returns the ShaderInput as a vector.  Assertion fails if there is none, or
 * if it is not a vector.
 */
LVecBase4 ShaderAttrib::
get_shader_input_vector(const InternalName *id) const {
  static LVecBase4 resfail(0,0,0,0);
  Inputs::const_iterator i = find_input(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i);

    if (p.get_value_type() == ShaderInput::M_vector) {
      return p.get_vector();

    } else if (p.get_value_type() == ShaderInput::M_numeric && p.get_ptr()._size <= 4) {
      const Shader::ShaderPtrData &ptr = p.get_ptr();

      switch (ptr._type) {
      case ShaderType::ST_float:
        {
          LVector4f vectorf;
          memcpy(&vectorf[0], ptr._ptr, sizeof(float) * ptr._size);
          return LCAST(PN_stdfloat, vectorf);
        }
      case ShaderType::ST_double:
        {
          LVector4d vectord;
          memcpy(&vectord[0], ptr._ptr, sizeof(double) * ptr._size);
          return LCAST(PN_stdfloat, vectord);
        }
      case ShaderType::ST_int:
        {
          LVector4i vectori;
          memcpy(&vectori[0], ptr._ptr, sizeof(int) * ptr._size);
          return LCAST(PN_stdfloat, vectori);
        }
      default:
       {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain numeric data.\n";
          nassert_raise(strm.str());
          return resfail;
        }
      }

    } else if (p.get_value_type() == ShaderInput::M_param) {
      // Temporary solution until the new param system
      TypedWritableReferenceCount *param = p.get_value();
      if (param != nullptr && param->is_of_type(ParamVecBase4::get_class_type())) {
        return ((const ParamVecBase4 *)param)->get_value();
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a vector.\n";
    nassert_raise(strm.str());
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
  }
  return resfail;
}

/**
 * Returns the ShaderInput as a ShaderPtrData struct.  Assertion fails if
 * there is none.  or if it is not a PTA(double/float)
 */
const Shader::ShaderPtrData *ShaderAttrib::
get_shader_input_ptr(const InternalName *id) const {
  Inputs::const_iterator i = find_input(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i);
    if (p.get_value_type() != ShaderInput::M_numeric &&
        p.get_value_type() != ShaderInput::M_vector) {
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a PTA(float/double) type.\n";
      nassert_raise(strm.str());
      return nullptr;
    }
    return &(p.get_ptr());
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return nullptr;
  }
}

/**
 * Returns the ShaderInput as a ShaderPtrData struct.  Assertion fails if
 * there is none.  or if it is not a PTA(double/float)
 */
bool ShaderAttrib::
get_shader_input_ptr(const InternalName *id, Shader::ShaderPtrData &data) const {
  Inputs::const_iterator i = find_input(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i);
    if (p.get_value_type() == ShaderInput::M_numeric ||
        p.get_value_type() == ShaderInput::M_vector) {

      data = p.get_ptr();
      return (data._ptr != nullptr);
    }
    if (p.get_value_type() == ShaderInput::M_param) {
      // Temporary solution until the new param system
      TypedWritableReferenceCount *param = p.get_value();
      if (param != nullptr) {
        if (param->is_of_type(ParamVecBase4f::get_class_type())) {
          data._ptr = (void *)((const ParamVecBase4f *)param)->get_value().get_data();
          data._size = 4;
          data._type = ShaderType::ST_float;
          return true;
        }
        else if (param->is_of_type(ParamVecBase4i::get_class_type())) {
          data._ptr = (void *)((const ParamVecBase4i *)param)->get_value().get_data();
          data._size = 4;
          data._type = ShaderType::ST_int;
          return true;
        }
        else if (param->is_of_type(ParamVecBase4d::get_class_type())) {
          data._ptr = (void *)((const ParamVecBase4d *)param)->get_value().get_data();
          data._size = 4;
          data._type = ShaderType::ST_float;
          return true;
        }
      }
    }
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " was given an incompatible parameter type.\n";
    nassert_raise(strm.str());
    return false;
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return false;
  }
}

/**
 * Returns the ShaderInput as a texture.  Assertion fails if there is none, or
 * if it is not a texture.
 *
 * If sampler is not NULL, the sampler state to use for this texture is
 * assigned to it.
 */
Texture *ShaderAttrib::
get_shader_input_texture(const InternalName *id, const SamplerState *&sampler) const {
  Inputs::const_iterator i = find_input(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i);
    switch (p.get_value_type()) {
    case ShaderInput::M_texture:
    case ShaderInput::M_texture_sampler:
      {
        Texture *tex = p.get_texture();
        //if (sampler) {
          sampler = &p.get_sampler();
        //}
        return tex;
      }

    default:
      ostringstream strm;
      strm <<  "Shader input " << id->get_name() << " is not a texture.\n";
      nassert_raise(strm.str());
      return nullptr;
    }

  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return nullptr;
  }
}

/**
 * Returns the ShaderInput as a matrix.  Assertion fails if there is none, or
 * if it is not a matrix or NodePath.
 */
const LMatrix4 &ShaderAttrib::
get_shader_input_matrix(const InternalName *id, LMatrix4 &matrix) const {
  Inputs::const_iterator i = find_input(id);
  if (i != _inputs.end()) {
    const ShaderInput &p = (*i);

    if (p.get_value_type() == ShaderInput::M_matrix) {
      matrix = p.get_matrix();
      return matrix;

    } else if (p.get_value_type() == ShaderInput::M_nodepath) {
      const NodePath &np = p.get_nodepath();
      nassertr(!np.is_empty(), LMatrix4::ident_mat());
      matrix = np.get_transform()->get_mat();
      return matrix;

    } else if (p.get_value_type() == ShaderInput::M_numeric &&
               p.get_ptr()._size >= 16 && (p.get_ptr()._size & 15) == 0) {
      const Shader::ShaderPtrData &ptr = p.get_ptr();

      switch (ptr._type) {
        case ShaderType::ST_float: {
          LMatrix4f matrixf;
          memcpy(&matrixf(0, 0), ptr._ptr, sizeof(float) * 16);
          matrix = LCAST(PN_stdfloat, matrixf);
          return matrix;
        }
        case ShaderType::ST_double: {
          LMatrix4d matrixd;
          memcpy(&matrixd(0, 0), ptr._ptr, sizeof(double) * 16);
          matrix = LCAST(PN_stdfloat, matrixd);
          return matrix;
        }
        default: {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain floating-point data.\n";
          nassert_raise(strm.str());
          return LMatrix4::ident_mat();
        }
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a NodePath, LMatrix4 or PTA_LMatrix4.\n";
    nassert_raise(strm.str());
    return LMatrix4::ident_mat();
  } else {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return LMatrix4::ident_mat();
  }
}

/**
 * Returns the ShaderInput as a ShaderBuffer.  Assertion fails if there is
 * none, or if it is not a ShaderBuffer.
 */
ShaderBuffer *ShaderAttrib::
get_shader_input_buffer(const InternalName *id) const {
  Inputs::const_iterator i = find_input(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return nullptr;
  } else {
    const ShaderInput &p = (*i);

    if (p.get_value_type() == ShaderInput::M_buffer) {
      ShaderBuffer *value;
      DCAST_INTO_R(value, p.get_value(), nullptr);
      return value;
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a ShaderBuffer.\n";
    nassert_raise(strm.str());
    return nullptr;
  }
}

/**
 *
 */
void ShaderAttrib::
output(ostream &out) const {
  out << "ShaderAttrib:";

  if (_auto_shader) {
    out << "auto";
    if (_shader_name != nullptr) {
      out << " (" << _shader_name->get_name() << ")";
    }
    return;
  } else if (_has_shader) {
    if (_shader == nullptr) {
      out << "off";
    } else {
      out << _shader->get_filename().get_basename();
    }
  }

  out << "," << _inputs.size() << " inputs";
}

/**
 * Intended to be overridden by derived ShaderAttrib types to return a unique
 * number indicating whether this ShaderAttrib is equivalent to the other one.
 *
 * This should return 0 if the two ShaderAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ShaderAttrib objects whose get_type()
 * functions return the same.
 */
int ShaderAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ShaderAttrib *that = (const ShaderAttrib *)other;

  if (this->_shader != that->_shader) {
    return (this->_shader < that->_shader) ? -1 : 1;
  }
  if (this->_shader_priority != that->_shader_priority) {
    return (this->_shader_priority < that->_shader_priority) ? -1 : 1;
  }
  if (this->_auto_shader != that->_auto_shader) {
    return (this->_auto_shader < that->_auto_shader) ? -1 : 1;
  }
  if (this->_has_shader != that->_has_shader) {
    return (this->_has_shader < that->_has_shader) ? -1 : 1;
  }
  if (this->_flags != that->_flags) {
    return (this->_flags < that->_flags) ? -1 : 1;
  }
  if (this->_has_flags != that->_has_flags) {
    return (this->_has_flags < that->_has_flags) ? -1 : 1;
  }
  if (this->_num_transforms != that->_num_transforms) {
    return (this->_num_transforms < that->_num_transforms) ? -1 : 1;
  }
  if (this->_instance_count != that->_instance_count) {
    return (this->_instance_count < that->_instance_count) ? -1 : 1;
  }
  if (this->_shader_name != that->_shader_name) {
    return (this->_shader_name < that->_shader_name) ? -1 : 1;
  }

  Inputs::const_iterator i1 = this->_inputs.begin();
  Inputs::const_iterator i2 = that->_inputs.begin();
  while ((i1 != this->_inputs.end()) && (i2 != that->_inputs.end())) {
    if (*i1 != *i2) {
      return (*i1 < *i2) ? -1 : 1;
    }
    ++i1;
    ++i2;
  }
  if (i1 != this->_inputs.end()) {
    return 1;
  }
  if (i2 != that->_inputs.end()) {
    return -1;
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ShaderAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = pointer_hash::add_hash(hash, _shader);
  hash = int_hash::add_hash(hash, _shader_priority);
  hash = int_hash::add_hash(hash, (int)_auto_shader);
  hash = int_hash::add_hash(hash, (int)_has_shader);
  hash = int_hash::add_hash(hash, _flags);
  hash = int_hash::add_hash(hash, _has_flags);
  hash = int_hash::add_hash(hash, _num_transforms);
  hash = int_hash::add_hash(hash, _instance_count);
  hash = pointer_hash::add_hash(hash, _shader_name);

  Inputs::const_iterator ii;
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) {
    hash = (*ii).add_hash(hash);
  }

  return hash;
}

/**
 *
 */
CPT(RenderAttrib) ShaderAttrib::
compose_impl(const RenderAttrib *other) const {
  ShaderAttrib *attr = new ShaderAttrib(*this);
  const ShaderAttrib *over = (const ShaderAttrib *)other;

  // Update the shader portion.
  if (over->_has_shader) {
    if ((attr->_has_shader == false) ||
        (over->_shader_priority >= attr->_shader_priority)) {
      attr->_shader = over->_shader;
      attr->_shader_priority = over->_shader_priority;
      attr->_auto_shader = over->_auto_shader;
      attr->_has_shader = over->_has_shader;
      attr->_shader_name = over->_shader_name;
    }
  }
  // Update the shader-data portion.
  Inputs::const_iterator iover;
  for (iover=over->_inputs.begin(); iover!=over->_inputs.end(); ++iover) {
    const ShaderInput &dover = (*iover);
    const InternalName *id = (*iover).get_name();
    Inputs::iterator iattr = attr->find_input(id);
    if (iattr == attr->_inputs.end()) {
      attr->_inputs.push_back(dover);
    } else {
      const ShaderInput &dattr = (*iattr);
      if (dattr.get_priority() <= dover.get_priority()) {
        *iattr = *iover;
      }
    }
  }
  attr->build_texture_inputs();

  // In case no instance count is set, just copy it.
  if (attr->_instance_count == 0) {
    attr->_instance_count = over->_instance_count;
  } else {
    // If an instance count is set, check if the other attrib has an instance
    // count set, if so, override it, otherwise just keep the current instance
    // count
    if (over->_instance_count > 0) {
      attr->_instance_count = over->_instance_count;
    }
  }

  // Update the flags.
  attr->_flags &= ~(over->_has_flags);
  attr->_flags |= over->_flags;
  attr->_has_flags |= (over->_has_flags);
  attr->_num_transforms = std::max(_num_transforms, over->_num_transforms);
  return return_new(attr);
}

/**
 * Tells the BamReader how to create objects of type ShaderAttrib.
 */
void ShaderAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _shader_name);
  dg.add_bool(_auto_shader);
  dg.add_bool(_has_shader);
  dg.add_int32(_shader_priority);
  dg.add_int32(_flags);
  dg.add_int32(_has_flags);
  //dg.add_int32(_num_transforms);
  dg.add_int32(_instance_count);
}

/**
 *
 */
int ShaderAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  _shader_name = DCAST(InternalName, p_list[pi++]);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderAttrib is encountered in the Bam file.  It should create the
 * ShaderAttrib and extract its information from the file.
 */
TypedWritable *ShaderAttrib::
make_from_bam(const FactoryParams &params) {
  ShaderAttrib *attrib = new ShaderAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderAttrib.
 */
void ShaderAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  manager->read_pointer(scan);
  _auto_shader = scan.get_bool();
  _has_shader = scan.get_bool();
  _shader_priority = scan.get_int32();
  _flags = scan.get_int32();
  _has_flags = scan.get_int32();
  //_num_transforms = scan.get_int32();
  _instance_count = scan.get_int32();
}

/**
 *
 */
void ShaderAttrib::
build_texture_inputs() {
  // Sort the inputs by value type.
  std::sort(_inputs.begin(), _inputs.end(), [](const ShaderInput &a, const ShaderInput &b) -> bool {
    return a.get_value_type() < b.get_value_type();
  });

  _texture_inputs.clear();

  for (auto it = _inputs.begin(); it != _inputs.end(); ++it) {
    Texture *tex = (*it).get_texture();
    if (tex != nullptr) {
      _texture_inputs.insert({ (*it).get_name(), tex });
    }
  }

  _has_texture_inputs = true;
}
