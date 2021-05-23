/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physQueryFilter.I
 * @author brian
 * @date 2021-05-22
 */

/**
 * Sets the parent node for the filter.  If the filter type is FT_exclude, any
 * actor nodes that are descendants of this node will be filtered out.  If the
 * filter type is FT_exclusive_include, actor nodes that are not descendants of
 * this node will be filtered out.
 */
INLINE void PhysQueryNodeFilter::
set_parent(const NodePath &parent) {
  _parent_node = parent;
}

/**
 * Returns the parent node for the filter.  See set_parent().
 */
INLINE NodePath PhysQueryNodeFilter::
get_parent() const {
  return _parent_node;
}

/**
 * Sets how to filter actor nodes that are descendants of the parent.
 */
INLINE void PhysQueryNodeFilter::
set_filter_type(FilterType filter_type) {
  _filter_type = filter_type;
}

/**
 * Returns how to filter actor nodes that are descendants of the parent.
 */
INLINE PhysQueryNodeFilter::FilterType PhysQueryNodeFilter::
get_filter_type() const {
  return _filter_type;
}
