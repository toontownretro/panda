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
INLINE const std::string &MapEntity::Connection::
get_output_name() const {
  return _output_name;
}

/**
 *
 */
INLINE const std::string &MapEntity::Connection::
get_target_name() const {
  return _target_name;
}

/**
 *
 */
INLINE const std::string &MapEntity::Connection::
get_input_name() const {
  return _input_name;
}

/**
 *
 */
INLINE int MapEntity::Connection::
get_num_parameters() const {
  return (int)_parameters.size();
}

/**
 *
 */
INLINE const std::string &MapEntity::Connection::
get_parameter(int n) const {
  assert(n >= 0 && n < (int)_parameters.size());
  return _parameters[n];
}

/**
 *
 */
INLINE float MapEntity::Connection::
get_delay() const {
  return _delay;
}

/**
 *
 */
INLINE bool MapEntity::Connection::
get_repeat() const {
  return _repeat;
}

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

/**
 *
 */
INLINE void MapEntity::
add_connection(const Connection &conn) {
  _connections.push_back(conn);
}

/**
 *
 */
INLINE int MapEntity::
get_num_connections() const {
  return (int)_connections.size();
}

/**
 *
 */
INLINE const MapEntity::Connection *MapEntity::
get_connection(int n) const {
  nassertr(n >= 0 && n < (int)_connections.size(), nullptr);
  return &_connections[n];
}
