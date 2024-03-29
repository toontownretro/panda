#ifndef KEYVALUES_EXT_H
#define KEYVALUES_EXT_H

#include "config_putil.h"
#include "keyValues.h"
#include "extension.h"
#include "py_panda.h"

template<>
class Extension<KeyValues> : public ExtensionBase<KeyValues> {
public:
  PyObject *as_int_list(const std::string &str);
  PyObject *as_float_list(const std::string &str);
  PyObject *as_float_tuple_list(const std::string &str);
};

#endif // KEYVALUES_EXT_H
