/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlSwitch.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLSWITCH_H
#define PMDLSWITCH_H

#include "pandabase.h"
#include "referenceCount.h"
#include "vector_string.h"
#include "luse.h"

class TokenFile;

/**
 * Defines an LOD switch in a .pmdl file.
 */
class EXPCL_PANDA_EGG PMDLSwitch : public ReferenceCount {
PUBLISHED:
  INLINE PMDLSwitch();

  INLINE void set_in_distance(PN_stdfloat distance);
  INLINE PN_stdfloat get_in_distance() const;

  INLINE void set_out_distance(PN_stdfloat distance);
  INLINE PN_stdfloat get_out_distance() const;

  INLINE void add_group(const std::string &name);
  INLINE size_t get_num_groups() const;
  INLINE std::string get_group(size_t n) const;

  INLINE void set_center(const LPoint3 &center);
  INLINE const LPoint3 &get_center() const;

  INLINE void set_fade(PN_stdfloat fade);
  INLINE PN_stdfloat get_fade() const;

public:
  bool parse(TokenFile *tokens);
  void write(std::ostream &out, int indent_level);

private:
  // List of meshes/nodes that should be part of the switch.
  vector_string _groups;

  PN_stdfloat _fade;

  LPoint3 _center;

  PN_stdfloat _in_distance;
  PN_stdfloat _out_distance;
};

#include "pmdlSwitch.I"

#endif // PMDLSWITCH_H
