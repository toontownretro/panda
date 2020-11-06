/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderEnums.cxx
 * @author lachbr
 * @date 2020-11-02
 */

#include "shaderEnums.h"
#include "string_utils.h"

std::ostream &
operator << (std::ostream &out, ShaderEnums::ShaderQuality sm) {
  switch (sm) {
    case ShaderEnums::SQ_low:
      out << "low";
      return out;
    case ShaderEnums::SQ_medium:
      out << "medium";
      return out;
    case ShaderEnums::SQ_high:
      out << "high";
      return out;
    default:
      out << "unknown";
      return out;
  }
}

std::istream &
operator >> (std::istream &in, ShaderEnums::ShaderQuality &sm) {
  std::string word;
  in >> word;

  if (word.size() == 0) {
    sm = ShaderEnums::SQ_high;
  } else if (cmp_nocase(word, "low") == 0) {
    sm = ShaderEnums::SQ_low;
  } else if (cmp_nocase(word, "medium") == 0) {
    sm = ShaderEnums::SQ_medium;
  } else if (cmp_nocase(word, "high") == 0) {
    sm = ShaderEnums::SQ_high;
  } else {
    sm = ShaderEnums::SQ_high;
  }

  return in;
}
