/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxElement.cxx
 * @author brian
 * @date 2021-06-10
 */

#include "pdxElement.h"
#include "indent.h"
#include "config_pdx.h"
#include "tokenFile.h"

/**
 *
 */
void PDXElement::
to_datagram(Datagram &dg) const {
  dg.add_uint32(get_num_attributes());
  for (size_t i = 0; i < get_num_attributes(); i++) {
    dg.add_string(get_attribute_name(i));
    get_attribute_value(i).to_datagram(dg);
  }
}

/**
 *
 */
void PDXElement::
from_datagram(DatagramIterator &scan) {
  size_t size = scan.get_uint32();
  for (size_t i = 0; i < size; i++) {
    std::string name = scan.get_string();
    PDXValue val;
    val.from_datagram(scan);
    set_attribute(name, val);
  }
}

/**
 *
 */
void PDXElement::
write(std::ostream &out, int indent_level) const {
  out << "\n";
  indent(out, indent_level) << "{\n";
  for (int i = 0; i < get_num_attributes(); i++) {
    const std::string &name = get_attribute_name(i);
    const PDXValue &value = get_attribute_value(i);
    indent(out, indent_level + 2) << "\"" << name << "\" ";
    value.write(out, indent_level + 2);
    out << "\n";
  }
  indent(out, indent_level) << "}";
}

/**
 *
 */
bool PDXElement::
parse(TokenFile *tokens) {
  while (true) {
    if (!tokens->token_available(true)) {
      pdx_cat.error()
        << "EOF while parsing PDX element\n";
      return false;
    }

    tokens->next_token(true);

    if (tokens->get_token_type() == TokenFile::TT_symbol &&
        tokens->get_token() == "}") {
      break;
    }

    if (tokens->get_token_type() != TokenFile::TT_word &&
        tokens->get_token_type() != TokenFile::TT_string) {
      pdx_cat.error()
        << "Expected attribute name\n";
      return false;
    }

    std::string name = tokens->get_token();

    PDXValue value;
    if (!value.parse(tokens)) {
      pdx_cat.error()
        << "Failed to parse element value\n";
      return false;
    }

    _attribs[name] = value;
  }

  return true;
}
