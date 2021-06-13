/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxValue.h
 * @author brian
 * @date 2021-06-10
 */

#ifndef PDXVALUE_H
#define PDXVALUE_H

#include "pandabase.h"
#include "pointerTo.h"
#include "config_putil.h"
#include "filename.h"
#include "dSearchPath.h"
#include "luse.h"

#include <variant>

class PDXElement;
class PDXList;
class TokenFile;

/**
 * A PDX value.
 */
class EXPCL_PANDA_PDX PDXValue {
PUBLISHED:
  enum ValueType {
    VT_invalid = -1,
    VT_float,
    VT_int,
    VT_string,
    VT_boolean,
    VT_element,
    VT_list,
  };

  PDXValue();
  ~PDXValue();
  PDXValue(const std::string &value);
  PDXValue(bool value);
  PDXValue(float value);
  PDXValue(int value);
  PDXValue(PDXElement *value);
  PDXValue(PDXList *value);

  PDXValue(const PDXValue &copy);
  PDXValue(PDXValue &&other);

  void operator = (const PDXValue &copy);
  void operator = (PDXValue &&other);

  bool read(const Filename &filename, const DSearchPath &search_path = get_model_path());
  bool write(const Filename &filename) const;

  void set_string(const std::string &value);
  INLINE std::string get_string() const;
  INLINE bool is_string() const;

  void set_bool(bool value);
  bool get_bool() const;
  INLINE bool is_bool() const;

  void set_float(float value);
  INLINE float get_float() const;
  INLINE bool is_float() const;

  void set_int(int value);
  INLINE int get_int() const;
  INLINE bool is_int() const;

  void set_element(PDXElement *value);
  INLINE PDXElement *get_element() const;
  INLINE bool is_element() const;

  void set_list(PDXList *value);
  INLINE PDXList *get_list() const;
  INLINE bool is_list() const;

  void clear();

  INLINE ValueType get_value_type() const;

  // Helpers to convert list values to linmath objects and back.
  bool to_vec2(LVecBase2 &vec) const;
  void from_vec2(const LVecBase2 &vec);

  bool to_vec3(LVecBase3 &vec) const;
  void from_vec3(const LVecBase3 &vec);

  bool to_vec4(LVecBase4 &vec) const;
  void from_vec4(const LVecBase4 &vec);

  bool to_mat3(LMatrix3 &mat) const;
  void from_mat3(const LMatrix3 &mat);

  bool to_mat4(LMatrix4 &mat) const;
  void from_mat4(const LMatrix4 &mat);

  void to_datagram(Datagram &dg) const;
  void from_datagram(DatagramIterator &scan);

public:
  void write(std::ostream &out, int indent_level) const;
  bool parse(TokenFile *tokens, bool get_next = true);

private:
  ValueType _value_type;
  std::variant<
    std::string,
    bool,
    int,
    float,
    PT(PDXElement),
    PT(PDXList)> _value;
};

#include "pdxValue.I"

#endif // PDXVALUE_H
