/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weightList.h
 * @author brian
 * @date 2021-05-07
 */

#ifndef WEIGHTLIST_H
#define WEIGHTLIST_H

#include "pandabase.h"
#include "pmap.h"
#include "numeric_types.h"
#include "namable.h"
#include "referenceCount.h"
#include "typedWritableReferenceCount.h"

class Character;
class FactoryParams;

/**
 * A descriptor for a joint weighting table.  Used to create a WeightList
 * object for per-joint weighted blending of additive animations.
 */
class EXPCL_PANDA_ANIM WeightListDesc : public Namable {
PUBLISHED:
  INLINE WeightListDesc(const std::string &name);

  INLINE void set_weight(const std::string &joint, PN_stdfloat weight);
  INLINE PN_stdfloat get_weight(const std::string &joint) const;
  INLINE bool has_weight(const std::string &joint) const;

public:
  INLINE void set_weights(const pmap<std::string, PN_stdfloat> &weights);

private:
  typedef pmap<std::string, PN_stdfloat> Weights;
  Weights _weights;

  friend class WeightList;
};

/**
 *
 */
class EXPCL_PANDA_ANIM WeightList : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  WeightList(Character *character, const WeightListDesc &desc);

  INLINE size_t get_num_weights() const;
  INLINE PN_stdfloat get_weight(size_t n) const;

private:
  WeightList() = default;

  void r_fill_weights(Character *character, const WeightListDesc &desc,
                      int joint, PN_stdfloat weight);

  typedef pvector<PN_stdfloat> Weights;
  Weights _weights;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "WeightList",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "weightList.I"

#endif // WEIGHTLIST_H
