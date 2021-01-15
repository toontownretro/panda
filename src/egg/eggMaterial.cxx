/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterial.cxx
 * @author lachbr
 * @date 2021-01-11
 */

#include "eggMaterial.h"
#include "eggMiscFuncs.h"

TypeHandle EggMaterial::_type_handle;

/**
 *
 */
EggMaterial::
EggMaterial(const std::string &mref_name, const Filename &filename) :
  EggFilenameNode(mref_name, filename)
{
}

/**
 *
 */
EggMaterial::
EggMaterial(const EggMaterial &copy) :
  EggFilenameNode(copy),
  _material(copy._material)
{
}

/**
 * Writes the material definition to the indicated output stream in Egg
 * format.
 */
void EggMaterial::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Material>");
  enquote_string(out, get_filename(), indent_level + 2) << "\n";
  indent(out, indent_level) << "}\n";
}

/**
 * Returns true if the two materials are equivalent in all relevant properties
 * (according to eq), false otherwise.
 *
 * The Equivalence parameter, eq, should be set to the bitwise OR of the
 * following properties, according to what you consider relevant:
 *
 * EggMaterial::E_filename: The filename referenced by the material, regardless
 * of the MRef name.
 *
 * EggMaterial::E_mref_name: The MRef name.
 */
bool EggMaterial::
is_equivalent_to(const EggMaterial &other, int eq) const {
  if (eq & E_filename) {
    if (get_filename() != other.get_filename()) {
      return false;
    }
  }

  if (eq & E_mref_name) {
    if (get_name() != other.get_name()) {
      return false;
    }
  }

  return true;
}

/**
 * An ordering operator to compare two materials for sorting order.  This
 * imposes an arbitrary ordering useful to identify unique materials,
 * according to the indicated Equivalence factor.  See is_equivalent_to().
 */
bool EggMaterial::
sorts_less_than(const EggMaterial &other, int eq) const {
  if (eq & E_filename) {
    if (get_filename() != other.get_filename()) {
      return get_filename() < other.get_filename();
    }
  }

  if (eq & E_mref_name) {
    if (get_name() != other.get_name()) {
      return get_name() < other.get_name();
    }
  }

  return false;
}
