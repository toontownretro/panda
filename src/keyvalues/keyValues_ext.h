#pragma once

#include "config_keyvalues.h"
#include "keyValues.h"
#include "extension.h"
#include "py_panda.h"

template<>
class Extension<CKeyValues> : public ExtensionBase<CKeyValues> {
public:
  PyObject *as_int_list(const std::string &str);
  PyObject *as_float_list(const std::string &str);
  PyObject *as_float_tuple_list(const std::string &str);
};
