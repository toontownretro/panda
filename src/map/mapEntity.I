/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapEntity.I
 * @author brian
 * @date 2021-07-08
 */

/**
 *
 */
INLINE void MapEntity::
set_model_index(int model) {
  _model_index = model;
}

/**
 *
 */
INLINE int MapEntity::
get_model_index() const {
  return _model_index;
}

/**
 *
 */
INLINE void MapEntity::
set_class_name(const std::string &class_name) {
  _class_name = class_name;
}

/**
 *
 */
INLINE const std::string &MapEntity::
get_class_name() const {
  return _class_name;
}

/**
 *
 */
INLINE void MapEntity::
set_properties(PDXElement *props) {
  _properties = props;
}

/**
 *
 */
INLINE PDXElement *MapEntity::
get_properties() const {
  return _properties;
}
