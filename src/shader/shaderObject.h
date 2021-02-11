/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderObject.h
 * @author lachbr
 * @date 2020-12-22
 */

#ifndef SHADEROBJECT_H
#define SHADEROBJECT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "internalName.h"
#include "shaderModuleSpirV.h"

class FactoryParams;

/**
 * This class represents what we call a shader object: a single shader module
 * containing SPIR-V byte code for each possible combination of preprocessor
 * defines that can be set, called combos.  This concept is based on Valve's
 * Source Engine.  The shader object simply contains an array of SPIR-V
 * shaders, originating from a single GLSL source file.  Each index into the
 * array corresponds to a unique combination of preprocessor definition values.
 */
class EXPCL_PANDA_SHADER ShaderObject : public TypedWritableReferenceCount {
PUBLISHED:
  class Combo {
  PUBLISHED:
    INLINE Combo();
    INLINE bool operator == (const Combo &other) const {
      return name == other.name;
    }

    CPT(InternalName) name;
    int min_val;
    int max_val;
    int scale;
  };

  INLINE ShaderObject();

  INLINE void add_combo(Combo &&combo);
  INLINE void add_combo(const Combo &combo);
  INLINE bool has_combo(CPT_InternalName name) const;
  INLINE const Combo &get_combo(size_t n) const;
  INLINE const Combo &get_combo(CPT_InternalName name) const;
  INLINE size_t get_num_combos() const;

  INLINE void add_permutation(ShaderModuleSpirV *module);
  INLINE void set_permutation(size_t n, ShaderModuleSpirV *module);
  INLINE void resize_permutations(size_t count);
  INLINE const ShaderModuleSpirV *get_permutation(size_t n) const;
  INLINE size_t get_num_permutations() const;

  INLINE size_t get_total_combos() const;

private:
  void calc_total_combos();

private:
  typedef pvector<Combo> Combos;
  Combos _combos;

  typedef pvector<COWPT(ShaderModuleSpirV)> Permutations;
  Permutations _permutations;

  size_t _total_combos;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
	virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ShaderObject",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderObject.I"

#endif // SHADEROBJECT_H
