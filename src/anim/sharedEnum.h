/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sharedEnum.h
 * @author brian
 * @date 2021-06-14
 */

#ifndef SHAREDENUM_H
#define SHAREDENUM_H

#include "pandabase.h"
#include "pmap.h"
#include "configVariableList.h"

/**
 * This is a base class for pseudo-enumerated types whose values are defined
 * in external PDX files.  The reason for this class is to give C++ code
 * access to game-specific enumerated types/values, such as animation events
 * and activity types.  Animation events and activity types are heavily
 * game-specific, but the C++ code needs access to them when loading .pmdl
 * files.  Each child of this class should define a global pointer and config
 * variable listing the PDX files to read the values from.
 */
class EXPCL_PANDA_ANIM SharedEnum {
PUBLISHED:
  int add_value(const std::string &name);
  void add_value(const std::string &name, int id);

  INLINE std::string get_value_name(int type) const;
  INLINE int get_value_id(const std::string &name) const;

  void load_values();

  EXTENSION(INLINE PyObject *__getattr__(PyObject *self, const std::string &attr_name) const);

protected:
  SharedEnum();

  // Override this in derived classes to return the ConfigVariableList that
  // contains the PDX files to read the enum values from.
  virtual const ConfigVariableList &get_config_var() const=0;

private:
  phash_map<std::string, int, string_hash> _value_ids;
  phash_map<int, std::string, int_hash> _value_names;
  bool _loaded_values;
  int _last_id;
};

#include "sharedEnum.I"

#endif // SHAREDENUM_H
