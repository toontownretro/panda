/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterial.h
 * @author lachbr
 * @date 2021-01-11
 */

#ifndef EGGMATERIAL_H
#define EGGMATERIAL_H

#include "pandabase.h"
#include "eggFilenameNode.h"
#include "material.h"

/**
 * A reference to a material file on disk.  This describes the render state
 * of geometry.  It is applied to primitives via <MRef>.
 */
class EggMaterial : public EggFilenameNode {
PUBLISHED:
  explicit EggMaterial(const std::string &mref_name, const Filename &filename);
  EggMaterial(const EggMaterial &copy);

  virtual void write(std::ostream &out, int indent_level) const;

  enum Equivalence {
    E_filename             = 0x001,
    E_mref_name            = 0x002,
  };

  bool is_equivalent_to(const EggMaterial &other, int eq) const;
  bool sorts_less_than(const EggMaterial &other, int eq) const;

  INLINE Material *get_material() const;

private:
  PT(Material) _material;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggFilenameNode::init_type();
    register_type(_type_handle, "EggMaterial",
                  EggFilenameNode::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

/**
 * An STL function object for sorting materials into order by properties.
 * Returns true if the two referenced EggMaterial pointers are in sorted
 * order, false otherwise.
 */
class EXPCL_PANDA_EGG UniqueEggMaterials {
public:
  INLINE UniqueEggMaterials(int eq = ~0);
  INLINE bool operator ()(const EggMaterial *t1, const EggMaterial *t2) const;

  int _eq;
};

#include "eggMaterial.I"

#endif // EGGMATERIAL_H
