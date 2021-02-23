/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBase.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef ANIMCHANNELBASE_H
#define ANIMCHANNELBASE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "pointerTo.h"

class AnimBundle;

/**
 * Parent class for all animation channels.  An AnimChannel is an arbitrary
 * function that changes over time (actually, over frames), usually defined by
 * a table read from an egg file (but possibly computed or generated in any
 * other way).
 */
class EXPCL_PANDA_CHAN AnimChannelBase : public TypedWritableReferenceCount, public Namable {
protected:
  INLINE AnimChannelBase(const AnimChannelBase &copy);

public:
  INLINE AnimChannelBase(const std::string &name);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);

  virtual TypeHandle get_value_type() const=0;

protected:

  int _last_frame;

  AnimBundle *_root;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

PUBLISHED:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

public:
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "AnimChannelBase",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class AnimBundle;
};

#include "animChannelBase.I"

#endif
