/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glVAOs_src.h
 * @author brian
 * @date 2022-03-08
 */

#include "pandabase.h"
#include "geomVertexFormat.h"
#include "shader.h"

/**
 * Data for a VAO corresponding to each unique GeomVertexFormat+
 * vertex shader input signature combination.
 *
 * Similar in principle to D3D10+ input layouts, but with VAOs,
 * the bound vertex arrays and index buffer is part of the VAO state,
 * along with actual input layout.
 *
 * Relies on OpenGL 4.3+ or the GL_ARB_vertex_attrib_binding extension.
 */
class VAOState {
public:
  INLINE VAOState() {
    _vao_id = 0;
    memset(_arrays, 0, sizeof(_arrays));
    _vertex_array_colors = false;
    _has_vertex_colors = false;
    _has_vertex_8joints = false;
    _vertex_array_8joints = false;
    _index_buffer = 0;
  }
  GLuint _vao_id;
  GLuint _index_buffer;
  // True if vertex colors are being used from a vertex array.
  bool _vertex_array_colors;
  // True if the vertex format has a color column.
  bool _has_vertex_colors;
  // True if the vertex format has transform_weight2 and transform_index2
  // columns.  Indicates that the vertex format has data for GPU animation
  // with up to 8 joint assignments per vertex.
  bool _has_vertex_8joints;
  bool _vertex_array_8joints;
  // BitMask of vertex array indices into the vertex format that are
  // actually needed by the shader.
  BitMask32 _used_arrays;
  struct ArrayBindState {
    GLuint _divisor;
    GLuint _stride;
    GLuint _array;
  };
  ArrayBindState _arrays[32];
};

/**
 * Defines a vertex input signature of a shader.
 * Shaders with identical vertex input signatures will share the same
 * ShaderVertexInputSignature pointer.
 */
class ShaderVertexInputSignature {
public:
  INLINE ShaderVertexInputSignature() = default;
  INLINE ShaderVertexInputSignature(const ShaderVertexInputSignature &copy) :
    _inputs(copy._inputs)
  {
  }
  INLINE ShaderVertexInputSignature(ShaderVertexInputSignature &&other) :
    _inputs(std::move(other._inputs))
  {
  }

  typedef pvector<Shader::ShaderVarSpec> VertexInputs;
  VertexInputs _inputs;

  INLINE int compare_to(const ShaderVertexInputSignature &other) const {
    if (_inputs.size() != other._inputs.size()) {
      return _inputs.size() < other._inputs.size() ? -1 : 1;
    }

    for (size_t i = 0; i < _inputs.size(); ++i) {
      const Shader::ShaderVarSpec &a = _inputs[i];
      const Shader::ShaderVarSpec &b = other._inputs[i];

      if (a._name != b._name) {
        return a._name < b._name ? -1 : 1;
      }
      if (a._elements != b._elements) {
        return a._elements < b._elements ? -1 : 1;
      }
      if (a._scalar_type != b._scalar_type) {
        return a._scalar_type < b._scalar_type ? -1 : 1;
      }
      if (a._append_uv != b._append_uv) {
        return a._append_uv < b._append_uv ? -1 : 1;
      }
      if (a._id._location != b._id._location) {
        return a._id._location < b._id._location ? -1 : 1;
      }
      if (a._id._type != b._id._type) {
        return a._id._type < b._id._type ? -1 : 1;
      }
      if (a._id._name != b._id._name) {
        return a._id._name < b._id._name ? -1 : 1;
      }
    }

    return 0;
  }
};

/**
 * Defines a lookup key for a unique GL VAO.
 *
 * A VAO is created for each unique
 * GeomVertexFormat+ShaderVertexInputSignature
 * combination.
 */
class VAOKey {
public:
  INLINE VAOKey() :
    _format(nullptr),
    _input_signature(nullptr)
  {
  }

  const GeomVertexFormat *_format;
  const ShaderVertexInputSignature *_input_signature;

  INLINE int compare_to(const VAOKey &other) const {
    if (_format != other._format) {
      return _format < other._format ? -1 : 1;
    }
    if (_input_signature != other._input_signature) {
      return _input_signature < other._input_signature ? -1 : 1;
    }
    return 0;
  }

  INLINE bool operator == (const VAOKey &other) const {
    return (_format == other._format) && (_input_signature == other._input_signature);
  }
};
