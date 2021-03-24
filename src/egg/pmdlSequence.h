/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlSequence.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLSEQUENCE_H
#define PMDLSEQUENCE_H

#include "pandabase.h"
#include "pmdlObject.h"

/**
 * This class represents a single animation sequence of a model.
 */
class PMDLSequence : public PMDLObject {
PUBLISHED:
  INLINE PMDLSequence(const std::string &name);

  INLINE void set_anim_filename(const Filename &filename);
  INLINE Filename get_anim_filename() const;

  INLINE void set_fps(int fps);
  INLINE int get_fps() const;

  INLINE void set_fade_in(PN_stdfloat fade_in);
  INLINE PN_stdfloat get_fade_in() const;

  INLINE void set_fade_out(PN_stdfloat fade_out);
  INLINE PN_stdfloat get_fade_out() const;

  INLINE void set_snap(bool snap);
  INLINE bool get_snap() const;

private:
  Filename _anim_filename;

  unsigned int _flags;

  int _fps;
  PN_stdfloat _fade_in;
  PN_stdfloat _fade_out;
  bool _snap;
};

#include "pmdlSequence.I"

#endif // PMDLSEQUENCE_H
