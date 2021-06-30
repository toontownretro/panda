/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sharedEnum.cxx
 * @author brian
 * @date 2021-06-14
 */

#include "sharedEnum.h"
#include "filename.h"
#include "executionEnvironment.h"
#include "config_anim.h"
#include "pdxList.h"
#include "pdxValue.h"

/**
 *
 */
SharedEnum::
SharedEnum() {
  _loaded_values = false;
  _last_id = INT_MIN;
}

/**
 * Adds a new enum value with the indicated name and returns the ID assigned to
 * the value.
 */
int SharedEnum::
add_value(const std::string &name) {
  int id;
  if (_value_ids.size() == 0) {
    id = 0;
  } else {
    id = ++_last_id;
  }
  add_value(name, id);
  return id;
}

/**
 * Adds a new enum value with an explicit ID.
 */
void SharedEnum::
add_value(const std::string &name, int id) {
  _value_ids[name] = id;
  _value_names[id] = name;
  if (_value_ids.size() == 1) {
    _last_id = id;
  } else {
    _last_id = std::max(id, _last_id);
  }
}

/**
 * Reads in the enum values from the configured list of PDX files.
 */
void SharedEnum::
load_values() {
  if (_loaded_values) {
    return;
  }

  _last_id = INT_MIN;
  _value_ids.clear();
  _value_names.clear();

  // Start with the invalid value.
  add_value("Invalid", -1);

  const ConfigVariableList &list = get_config_var();

  for (size_t i = 0; i < list.get_num_unique_values(); i++) {
    Filename filename = Filename::from_os_specific(
      ExecutionEnvironment::expand_string(
        list.get_unique_value(i)));

    anim_cat.info()
      << "Loading enum file " << filename << "\n";

    PDXValue val;
    if (!val.read(filename)) {
      anim_cat.error()
        << "Could not load anim event file " << filename << "\n";
      continue;
    }

    PDXList *event_list = val.get_list();
    if (event_list == nullptr) {
      anim_cat.error()
        << "Root value of anim event file must be a PDXList\n";
      continue;
    }

    for (size_t j = 0; j < event_list->size(); j++) {
      add_value(event_list->get(j).get_string());
    }
  }

  _loaded_values = true;
}
