/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxList.cxx
 * @author brian
 * @date 2021-06-10
 */

#include "pdxList.h"
#include "indent.h"
#include "tokenFile.h"
#include "config_pdx.h"

/**
 *
 */
void PDXList::
to_datagram(Datagram &dg) const {
  dg.add_uint32(size());
  for (size_t i = 0; i < size(); i++) {
    get(i).to_datagram(dg);
  }
}

/**
 *
 */
void PDXList::
from_datagram(DatagramIterator &scan) {
  size_t size = scan.get_uint32();
  _values.resize(size);
  for (size_t i = 0; i < size; i++) {
    get(i).from_datagram(scan);
  }
}

/**
 *
 */
void PDXList::
write(std::ostream &out, int indent_level) const {
  out << "\n";
  indent(out, indent_level) << "[\n";
  for (size_t i = 0; i < size(); i++) {
    const PDXValue &value = get(i);
    value.write(out, indent_level + 2);
    out << "\n";
  }
  indent(out, indent_level) << "]";
}

/**
 *
 */
bool PDXList::
parse(TokenFile *tokens) {
  while (true) {
    if (!tokens->token_available(true)) {
      pdx_cat.error()
        << "EOF while parsing PDX list\n";
      return false;
    }

    tokens->next_token(true);

    if (tokens->get_token_type() == TokenFile::TT_symbol &&
        tokens->get_token() == "]") {
      break;
    }

    PDXValue value;
    if (!value.parse(tokens, false)) {
      pdx_cat.error()
        << "Failed to parse list value\n";
      return false;
    }
    _values.push_back(value);
  }

  return true;
}
