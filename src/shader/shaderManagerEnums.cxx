/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManagerEnums.cxx
 * @author brian
 * @date 2020-11-02
 */

#include "shaderManagerEnums.h"
#include "string_utils.h"

std::ostream &
operator << (std::ostream &out, ShaderManagerEnums::ShaderQuality sm) {
  switch (sm) {
    case ShaderManagerEnums::SQ_low:
      out << "low";
      return out;
    case ShaderManagerEnums::SQ_medium:
      out << "medium";
      return out;
    case ShaderManagerEnums::SQ_high:
      out << "high";
      return out;
    default:
      out << "unknown";
      return out;
  }
}

std::istream &
operator >> (std::istream &in, ShaderManagerEnums::ShaderQuality &sm) {
  std::string word;
  in >> word;

  if (word.size() == 0) {
    sm = ShaderManagerEnums::SQ_high;
  } else if (cmp_nocase(word, "low") == 0) {
    sm = ShaderManagerEnums::SQ_low;
  } else if (cmp_nocase(word, "medium") == 0) {
    sm = ShaderManagerEnums::SQ_medium;
  } else if (cmp_nocase(word, "high") == 0) {
    sm = ShaderManagerEnums::SQ_high;
  } else {
    sm = ShaderManagerEnums::SQ_high;
  }

  return in;
}
