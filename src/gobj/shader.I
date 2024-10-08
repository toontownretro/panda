/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shader.I
 * @author jyelon
 * @date 2005-09
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 */

/**
 * Return the Shader's filename for the given shader type.
 *
 * @deprecated Access the filename of the individual modules instead.
 */
INLINE Filename Shader::
get_filename(DeprecatedShaderType type) const {
  if (_filename._separate && type != ST_none) {
    switch (type) {
      case ST_vertex:
        return _filename._vertex;
        break;
      case ST_fragment:
        return _filename._fragment;
        break;
      case ST_geometry:
        return _filename._geometry;
        break;
      case ST_tess_control:
        return _filename._tess_control;
        break;
      case ST_tess_evaluation:
        return _filename._tess_evaluation;
        break;
      case ST_compute:
        return _filename._compute;
        break;
      default:
        return _filename._shared;
    }
  } else if (!_filename._shared.empty()) {
    return _filename._shared;

  } else {
    // Um, better than nothing?
    return _filename._fragment;
  }
}

/**
 * Sets the Shader's filename for the given shader type.  Useful for
 * associating a shader created with Shader.make with a name for diagnostics.
 *
 * @deprecated Access the filename of the individual modules instead.
 */
INLINE void Shader::
set_filename(DeprecatedShaderType type, const Filename &filename) {
  _filename._separate = true;
  switch (type) {
  case ST_vertex:
    _filename._vertex = filename;
    break;
  case ST_fragment:
    _filename._fragment = filename;
    break;
  case ST_geometry:
    _filename._geometry = filename;
    break;
  case ST_tess_control:
    _filename._tess_control = filename;
    break;
  case ST_tess_evaluation:
    _filename._tess_evaluation = filename;
    break;
  case ST_compute:
    _filename._compute = filename;
    break;
  default:
    _filename._shared = filename;
    _filename._separate = false;
  }
}

/**
 * Return the Shader's text for the given shader type.
 *
 * @deprecated
 */
INLINE const std::string &Shader::
get_text(DeprecatedShaderType type) const {
  if (_text._separate) {
    nassertr(type != ST_none || !_text._shared.empty(), _text._shared);
    switch (type) {
      case ST_vertex:
        return _text._vertex;
        break;
      case ST_fragment:
        return _text._fragment;
        break;
      case ST_geometry:
        return _text._geometry;
        break;
      case ST_tess_control:
        return _text._tess_control;
        break;
      case ST_tess_evaluation:
        return _text._tess_evaluation;
        break;
      case ST_compute:
        return _text._compute;
        break;
      default:
        return _text._shared;
    }
  } else {
    return _text._shared;
  }
}

/**
 * Returns true if the shader contains a compile-time error.  This doesn't
 * tell you whether or not the shader is supported on the current video card.
 */
INLINE bool Shader::
get_error_flag() const {
  return _error_flag;
}

/**
 * Returns the shader language in which this shader was written.
 */
INLINE Shader::ShaderLanguage Shader::
get_language() const {
  return _language;
}

/**
 * Returns a mask indicating which capabilities this shader needs to function
 * optimally.
 */
INLINE int Shader::
get_used_capabilities() const {
  return _used_caps;
}

/**
 * Returns true if the fullpath has been set and is available.  See
 * set_fullpath().
 */
INLINE bool Shader::
has_fullpath() const {
  return !_fullpath.empty();
}

/**
 * Returns the fullpath that has been set.  This is the full path to the file
 * as it was found along the model-path.
 */
INLINE const Filename &Shader::
get_fullpath() const {
  return _fullpath;
}

/**
 * Returns true if the shader has the given stage.
 */
INLINE bool Shader::
has_stage(Stage stage) const {
  return (_module_mask & (1u << (uint32_t)stage)) != 0;
}

/**
 * Returns the module for the given stage, or null if this stage isn't here.
 */
INLINE CPT(ShaderModule) Shader::
get_module(Stage stage) const {
  if (has_stage(stage)) {
    size_t index = ::count_bits_in_word(((1u << (uint32_t)stage) - 1) & _module_mask);
    CPT(ShaderModule) module = _modules[index]._module.get_read_pointer();
    nassertr(module->get_stage() == stage, nullptr);
    return module;
  }
  return nullptr;
}

/**
 * Returns the module for the given stage, or null if this stage isn't here.
 */
INLINE PT(ShaderModule) Shader::
modify_module(Stage stage) {
  if (has_stage(stage)) {
    size_t index = ::count_bits_in_word(((1u << (uint32_t)stage) - 1) & _module_mask);
    PT(ShaderModule) module = _modules[index]._module.get_write_pointer();
    nassertr(module->get_stage() == stage, nullptr);
    return module;
  }
  return nullptr;
}

/**
 * Returns the setting of the cache_compiled_shader flag.  See
 * set_cache_compiled_shader().
 */
INLINE bool Shader::
get_cache_compiled_shader() const {
  return _cache_compiled_shader;
}

/**
 * Sets the cache_compiled_shader flag.  When this is set, the next time the
 * Shader is loaded on a GSG, it will automatically extract the compiled
 * shader from the GSG and save it to the global BamCache.
 *
 * This is used to store compiled shaders in the BamCache.  This flag should
 * not be set explicitly; it is set automatically by the ShaderPool when
 * model-cache-compiled-shaders is set true.
 */
INLINE void Shader::
set_cache_compiled_shader(bool flag) {
  _cache_compiled_shader = flag;
}

/**
 * Sets a boolean value for the specialization constant with the indicated
 * name.  All modules containing a specialization constant with this name
 * will be given this value.
 *
 * Returns true if there was a specialization constant with this name on any of
 * the modules, false otherwise.
 */
INLINE bool Shader::
set_constant(CPT_InternalName name, bool value) {
  return set_constant(name, (uint32_t)value);
}

/**
 * Sets an integer value for the specialization constant with the indicated
 * name.  All modules containing a specialization constant with this name
 * will be given this value.
 *
 * Returns true if there was a specialization constant with this name on any of
 * the modules, false otherwise.
 */
INLINE bool Shader::
set_constant(CPT_InternalName name, int value) {
  uint32_t val;
  *((int *)&val) = value;
  return set_constant(name, val);
}

/**
 * Sets a float value for the specialization constant with the indicated
 * name.  All modules containing a specialization constant with this name
 * will be given this value.
 *
 * Returns true if there was a specialization constant with this name on any of
 * the modules, false otherwise.
 */
INLINE bool Shader::
set_constant(CPT_InternalName name, float value) {
  uint32_t val;
  *((float *)&val) = value;
  return set_constant(name, val);
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData() :
  _ptr(nullptr),
  _type(ScalarType::ST_unknown),
  _updated(true),
  _size(0)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_float &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_float),
  _updated(true),
  _size(ptr.size())
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix4f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_float),
  _updated(true),
  _size(ptr.size() * 16)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix3f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_float),
  _updated(true),
  _size(ptr.size() * 9)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase4f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_float),
  _updated(true),
  _size(ptr.size() * 4)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase3f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_float),
  _updated(true),
  _size(ptr.size() * 3)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase2f &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_float),
  _updated(true),
  _size(ptr.size() * 2)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_double &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_double),
  _updated(true),
  _size(ptr.size())
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix4d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_double),
  _updated(true),
  _size(ptr.size() * 16)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LMatrix3d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_double),
  _updated(true),
  _size(ptr.size() * 9)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase4d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_double),
  _updated(true),
  _size(ptr.size() * 4)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase3d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_double),
  _updated(true),
  _size(ptr.size() * 3)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase2d &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_double),
  _updated(true),
  _size(ptr.size() * 2)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_int &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_int),
  _updated(true),
  _size(ptr.size())
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase4i &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_int),
  _updated(true),
  _size(ptr.size() * 4)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase3i &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_int),
  _updated(true),
  _size(ptr.size() * 3)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const PTA_LVecBase2i &ptr):
  _pta(ptr.v0()),
  _ptr(ptr.p()),
  _type(ScalarType::ST_int),
  _updated(true),
  _size(ptr.size() * 2)
{
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase4i &vec) :
  _type(ScalarType::ST_int),
  _updated(true),
  _size(4)
{
  PTA_int pta = PTA_int::empty_array(4);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase3i &vec) :
  _type(ScalarType::ST_int),
  _updated(true),
  _size(3)
{
  PTA_int pta = PTA_int::empty_array(3);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

/**
 *
 */
INLINE Shader::ShaderPtrData::
ShaderPtrData(const LVecBase2i &vec) :
  _type(ScalarType::ST_int),
  _updated(true),
  _size(2)
{
  PTA_int pta = PTA_int::empty_array(2);
  _pta = pta.v0();
  _ptr = pta.p();
  nassertv(sizeof(vec[0]) * vec.get_num_components() == pta.size() * sizeof(pta[0]));
  memcpy(_ptr, vec.get_data(), sizeof(vec[0]) * vec.get_num_components());
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
INLINE void Shader::ShaderPtrData::
write_datagram(Datagram &dg) const {
  dg.add_uint8(_type);
  dg.add_uint32((uint32_t)_size);

  if (_type == ScalarType::ST_double) {
    const double *data = (const double *) _ptr;
    for (size_t i = 0; i < _size; ++i) {
      dg.add_float64(data[i]);
    }
  }
  else if (_type == ScalarType::ST_float) {
    const float *data = (const float *) _ptr;
    for (size_t i = 0; i < _size; ++i) {
      dg.add_float32(data[i]);
    }
  }
  else if (_type == ScalarType::ST_int) {
    const int *data = (const int *) _ptr;
    for (size_t i = 0; i < _size; ++i) {
      dg.add_int32(data[i]);
    }
  }
}

/**
 * Reads the object from a Datagram.
 */
INLINE void Shader::ShaderPtrData::
read_datagram(DatagramIterator &scan) {
  _type = (::ShaderType::ScalarType) scan.get_uint8();
  _size = scan.get_uint32();

  if (_type == ScalarType::ST_double) {
    PTA_double pta = PTA_double::empty_array(_size);
    for (size_t i = 0; i < _size; ++i) {
      pta[i] = scan.get_float64();
    }
    _pta = pta.v0();
    _ptr = pta.p();

  } else if (_type == ScalarType::ST_float) {
    PTA_float pta = PTA_float::empty_array(_size);
    for (size_t i = 0; i < _size; ++i) {
      pta[i] = scan.get_float32();
    }
    _pta = pta.v0();
    _ptr = pta.p();

  } else if (_type == ScalarType::ST_int) {
    PTA_int pta = PTA_int::empty_array(_size);
    for (size_t i = 0; i < _size; ++i) {
      pta[i] = scan.get_int32();
    }
    _pta = pta.v0();
    _ptr = pta.p();
  }
}

/**
 *
 */
INLINE Shader::ShaderFile::
ShaderFile(std::string shared) :
  _separate(false),
  _shared(std::move(shared))
{
}

/**
 *
 */
INLINE Shader::ShaderFile::
ShaderFile(std::string vertex, std::string fragment, std::string geometry,
           std::string tess_control, std::string tess_evaluation) :
  _separate(true),
  _vertex(std::move(vertex)),
  _fragment(std::move(fragment)),
  _geometry(std::move(geometry)),
  _tess_control(std::move(tess_control)),
  _tess_evaluation(std::move(tess_evaluation))
{
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
INLINE void Shader::ShaderFile::
write_datagram(Datagram &dg) const {
  if (_separate) {
    dg.add_uint8(6);
    dg.add_string(_vertex);
    dg.add_string(_fragment);
    dg.add_string(_geometry);
    dg.add_string(_tess_control);
    dg.add_string(_tess_evaluation);
    dg.add_string(_compute);
  } else {
    dg.add_uint8(0);
    dg.add_string(_shared);
  }
}

/**
 * Reads the object from a Datagram.
 */
INLINE void Shader::ShaderFile::
read_datagram(DatagramIterator &scan) {
  short count = scan.get_uint8();
  if (count > 0) {
    _separate = true;
    if (count-- > 0) _vertex = scan.get_string();
    if (count-- > 0) _fragment = scan.get_string();
    if (count-- > 0) _geometry = scan.get_string();
    if (count-- > 0) _tess_control = scan.get_string();
    if (count-- > 0) _tess_evaluation = scan.get_string();
    if (count-- > 0) _compute = scan.get_string();
    while (count-- > 0) {
      scan.get_string();
    }
  } else {
    _separate = false;
    _shared = scan.get_string();
  }
}

/**
 * Ordering operator
 */
INLINE bool Shader::ShaderFile::
operator < (const Shader::ShaderFile &other) const {
  if (_separate != other._separate) {
    return (!_separate && other._separate);
  }
  if (_shared != other._shared) {
    return (_shared < other._shared);
  }
  if (_vertex != other._vertex) {
    return (_vertex < other._vertex);
  }
  if (_fragment != other._fragment) {
    return (_fragment < other._fragment);
  }
  if (_geometry != other._geometry) {
    return (_geometry < other._geometry);
  }
  if (_tess_control != other._tess_control) {
    return (_tess_control < other._tess_control);
  }
  if (_tess_evaluation != other._tess_evaluation) {
    return (_tess_evaluation < other._tess_evaluation);
  }
  if (_compute != other._compute) {
    return (_compute < other._compute);
  }
  return false;
}

/**
 * Sets an external value for the specialization constant with the given ID.
 *
 * Returns true if the value is different from what was already in there, false
 * otherwise.
 */
INLINE bool Shader::ModuleSpecConstants::
set_constant(uint32_t id, uint32_t value) {
  auto it = std::find(_indices.begin(), _indices.end(), id);
  if (it == _indices.end()) {
    _indices.push_back(id);
    _values.push_back(value);
    return true;

  } else {
    size_t loc = it - _indices.begin();
    if (_values[loc] != value) {
      _values[loc] = value;
      return true;
    }
    return false;
  }
}

/**
 * Returns a PStatCollector for timing the preparation of just this shader.
 */
INLINE PStatCollector &Shader::
get_prepare_shader_pcollector() {
  return _prepare_shader_pcollector;
}

/**
 * Returns a name for the shader that is used for debugging.
 */
INLINE const std::string &Shader::
get_debug_name() const {
  return _debug_name;
}
