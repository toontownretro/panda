/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderType.h
 * @author rdb
 * @date 2019-03-12
 */

#ifndef SHADERTYPE_H
#define SHADERTYPE_H

#include "typedObject.h"
#include "geomEnums.h"
#include "pmap.h"
#include "stl_compares.h"
#include "texture.h"

/**
 * This represents a single type as defined in a shader.  There is only ever a
 * single instance in existence for any particular type.
 */
class EXPCL_PANDA_GOBJ ShaderType : public TypedWritable {
public:
  template<class Type>
  static const Type *register_type(Type &&type);

  INLINE int compare_to(const ShaderType &other) const;
  virtual int compare_to_impl(const ShaderType &other) const=0;

  virtual void output(std::ostream &out) const=0;

  virtual int get_num_parameter_locations() const { return 1; }

private:
  typedef pset<const ShaderType *, indirect_compare_to<const ShaderType *> > Registry;
  static Registry *_registered_types;

PUBLISHED:
  class Scalar;
  class Vector;
  class Matrix;
  class Struct;
  class Array;
  class Image;
  class Sampler;
  class SampledImage;

  // Fundamental types.
  static const ShaderType::Scalar *bool_type;
  static const ShaderType::Scalar *int_type;
  static const ShaderType::Scalar *uint_type;
  static const ShaderType::Scalar *float_type;
  static const ShaderType::Scalar *double_type;

public:
  virtual const Scalar *as_scalar() const { return nullptr; }
  virtual const Vector *as_vector() const { return nullptr; }
  virtual const Matrix *as_matrix() const { return nullptr; }
  virtual const Struct *as_struct() const { return nullptr; }
  virtual const Array *as_array() const { return nullptr; }
  virtual const Image *as_image() const { return nullptr; }
  virtual const Sampler *as_sampler() const { return nullptr; }
  virtual const SampledImage *as_sampled_image() const { return nullptr; }

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const ShaderType &stype) {
  stype.output(out);
  return out;
}

/**
 * A numeric scalar type, like int or float.
 */
class EXPCL_PANDA_GOBJ ShaderType::Scalar final : public ShaderType {
private:
  INLINE Scalar(GeomEnums::NumericType numeric_type);

  INLINE GeomEnums::NumericType get_numeric_type() const;

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  const Scalar *as_scalar() const override { return this; }

private:
  GeomEnums::NumericType _numeric_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Multiple scalar types.
 */
class EXPCL_PANDA_GOBJ ShaderType::Vector final : public ShaderType {
public:
  INLINE Vector(const ShaderType *base_type, size_t num_elements);
  Vector(const Vector &copy) = default;

  INLINE const ShaderType *get_base_type() const;
  INLINE size_t get_num_elements() const;

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

public:
  const Vector *as_vector() const override { return this; }

private:
  const ShaderType *_base_type;
  size_t _num_elements;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Matrix consisting of multiple column vectors.
 */
class EXPCL_PANDA_GOBJ ShaderType::Matrix final : public ShaderType {
public:
  INLINE Matrix(const ShaderType *base_type, size_t num_rows, size_t num_columns);

  INLINE const ShaderType *get_base_type() const;
  INLINE size_t get_num_rows() const;
  INLINE size_t get_num_columns() const;

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  const Matrix *as_matrix() const override { return this; }

private:
  const ShaderType *_base_type;
  size_t _num_rows;
  size_t _num_columns;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * A structure type, with named members.
 */
class EXPCL_PANDA_GOBJ ShaderType::Struct final : public ShaderType {
public:
  struct Member;

  INLINE size_t get_num_members() const;
  INLINE const Member &get_member(size_t i) const;
  INLINE void add_member(const ShaderType *type, CPT(InternalName) name);

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual int get_num_parameter_locations() const override;

  const Struct *as_struct() const override { return this; }

PUBLISHED:
  MAKE_SEQ_PROPERTY(members, get_num_members, get_member);

  struct Member {
    const ShaderType *type;
    CPT(InternalName) name;
  };

private:
  pvector<Member> _members;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * An array type.
 */
class EXPCL_PANDA_GOBJ ShaderType::Array final : public ShaderType {
public:
  INLINE Array(const ShaderType *element_type, size_t num_elements);

  INLINE const ShaderType *get_element_type() const;
  INLINE size_t get_num_elements() const;

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  virtual int get_num_parameter_locations() const override;

  const Array *as_array() const override { return this; }

PUBLISHED:
  MAKE_PROPERTY(element_type, get_element_type);
  MAKE_PROPERTY(num_elements, get_num_elements);

private:
  const ShaderType *_element_type;
  size_t _num_elements;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Image type.
 */
class EXPCL_PANDA_GOBJ ShaderType::Image final : public ShaderType {
PUBLISHED:
  enum class Access {
    unknown = 0,
    read_only = 1,
    write_only = 2,
    read_write = 3,
  };

public:
  INLINE Image(Texture::TextureType texture_type, Access access);

  INLINE Texture::TextureType get_texture_type() const;
  INLINE Access get_access() const;
  INLINE bool is_writable() const;

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  const Image *as_image() const override { return this; }

PUBLISHED:
  MAKE_PROPERTY(texture_type, get_texture_type);
  MAKE_PROPERTY(access, get_access);
  MAKE_PROPERTY(writable, is_writable);

private:
  Texture::TextureType _texture_type;
  Access _access;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

/**
 * Sampled image type.
 */
class EXPCL_PANDA_GOBJ ShaderType::SampledImage final : public ShaderType {
public:
  INLINE SampledImage(Texture::TextureType texture_type);

  INLINE Texture::TextureType get_texture_type() const;

  virtual void output(std::ostream &out) const override;
  virtual int compare_to_impl(const ShaderType &other) const override;

  const SampledImage *as_sampled_image() const override { return this; }

private:
  Texture::TextureType _texture_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class ShaderType;
};

#ifndef CPPPARSER
#include "shaderType.I"
#endif

#endif  // SHADERTYPE_H
