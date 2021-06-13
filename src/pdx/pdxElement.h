/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxElement.h
 * @author brian
 * @date 2021-06-10
 */

#ifndef PDXELEMENT_H
#define PDXELEMENT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pdxValue.h"
#include "simpleHashMap.h"

class TokenFile;

/**
 * A list of name->PDXValue attributes.
 */
class EXPCL_PANDA_PDX PDXElement : public ReferenceCount {
PUBLISHED:
  PDXElement() = default;
  ~PDXElement() = default;

  INLINE void set_attribute(const std::string &name, const PDXValue &value);
  INLINE int get_num_attributes() const;
  INLINE const std::string &get_attribute_name(int n) const;
  INLINE const PDXValue &get_attribute_value(int n) const;
  INLINE PDXValue &get_attribute_value(int n);
  INLINE PDXValue &get_attribute_value(const std::string &name);
  INLINE int find_attribute(const std::string &name) const;
  INLINE bool has_attribute(const std::string &name) const;
  INLINE void remove_attribute(const std::string &name);
  INLINE void remove_attribute(int n);

  INLINE PDXValue &operator [] (const std::string &name);

  void to_datagram(Datagram &dg) const;
  void from_datagram(DatagramIterator &scan);

public:
  void write(std::ostream &out, int indent_level) const;
  bool parse(TokenFile *tokens);

private:
  typedef SimpleHashMap<std::string, PDXValue, string_hash> Attributes;
  Attributes _attribs;
};

#include "pdxElement.I"

#endif // PDXELEMENT_H
