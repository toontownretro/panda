/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggUtilities.cxx
 * @author drose
 * @date 1999-01-28
 */

#include "eggUtilities.h"
#include "eggPrimitive.h"
#include "eggGroupNode.h"
#include "dcast.h"


/**
 * Extracts from the egg subgraph beginning at the indicated node a set of all
 * the material objects referenced, grouped together by filename.  Material
 * objects that share a common filename (but possibly differ in other
 * properties) are returned together in the same element of the map.
 */
void
get_materials_by_filename(const EggNode *node, EggMaterialFilenames &result) {
  if (node->is_of_type(EggPrimitive::get_class_type())) {
    const EggPrimitive *prim = DCAST(EggPrimitive, node);

    if (prim->has_material()) {
      EggMaterial *mat = prim->get_material();
      result[mat->get_filename()].insert(mat);
    }

  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    const EggGroupNode *group = DCAST(EggGroupNode, node);

    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      get_materials_by_filename(*ci, result);
    }
  }
}
