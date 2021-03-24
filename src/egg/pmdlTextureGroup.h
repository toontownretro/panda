/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlTextureGroup.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLTEXTUREGROUP_H
#define PMDLTEXTUREGROUP_H

#include "pandabase.h"
#include "referenceCount.h"
#include "namable.h"
#include "filename.h"
#include "pvector.h"

/**
 * This class represents a set of materials that may be replaced by another set
 * of materials, defining the different "skins" of a model.
 */
class EXPCL_PANDA_EGG PMDLTextureGroup : public ReferenceCount {
PUBLISHED:
  INLINE PMDLTextureGroup();

  INLINE void add_material(const Filename &filename);
  INLINE const Filename &get_material(size_t n) const;
  INLINE size_t get_num_materials() const;

private:
  pvector<Filename> _materials;
};

#include "pmdlTextureGroup.I"

#endif // PMDLTEXTUREGROUP_H
