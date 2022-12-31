/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodeVertexTransform.cxx
 * @author drose
 * @date 2007-02-22
 */

#include "nodeVertexTransform.h"

TypeHandle NodeVertexTransform::_type_handle;

/**
 *
 */
NodeVertexTransform::
NodeVertexTransform(const PandaNode *node,
                    const VertexTransform *prev) :
  _node(node),
  _prev(prev)
{
}

#if 1
/**
 * Returns the transform of the associated node, composed with the previous
 * VertexTransform if any, expressed as a matrix.
 */
LMatrix4 NodeVertexTransform::
get_matrix(Thread *current_thread) const {
  if (_prev != nullptr) {
    LMatrix4 prev_matrix = _prev->get_matrix(current_thread);
    return _node->get_transform()->get_mat() * prev_matrix;

  } else {
    return _node->get_transform()->get_mat();
  }
}
#endif

/**
 *
 */
void NodeVertexTransform::
output(std::ostream &out) const {
  if (_prev != nullptr) {
    _prev->output(out);
    out << " * ";
  }

  out << "NodeVertexTransform(" << _node->get_name() << ")";
}
