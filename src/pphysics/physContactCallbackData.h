/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physContactCallbackData.h
 * @author brian
 * @date 2021-04-24
 */

#ifndef PHYSCONTACTCALLBACKDATA_H
#define PHYSCONTACTCALLBACKDATA_H

#include "pandabase.h"
#include "refCallbackData.h"
#include "physRigidActorNode.h"
#include "physx_utils.h"

class EXPCL_PANDA_PPHYSICS PhysContactPoint {
PUBLISHED:
  INLINE PhysContactPoint(const physx::PxContactPairPoint &point);

  INLINE LPoint3 get_position() const;
  INLINE PN_stdfloat get_separation() const;
  INLINE LVector3 get_normal() const;
  INLINE LVector3 get_impulse() const;
  INLINE size_t get_face_index_a() const;
  INLINE size_t get_face_index_b() const;

private:
  const physx::PxContactPairPoint &_point;
};

class EXPCL_PANDA_PPHYSICS PhysContactPair {
PUBLISHED:
  PhysContactPair() = default;

  INLINE PhysShape *get_shape_a() const;
  INLINE PhysShape *get_shape_b() const;

  INLINE int get_contact_type() const;
  INLINE bool is_contact_type(int flags) const;

  INLINE PhysContactPoint get_contact_point(size_t n) const;
  INLINE size_t get_num_contact_points() const;

private:
  PhysShape *_shape_a;
  PhysShape *_shape_b;

  int _contact_type;

  typedef pvector<physx::PxContactPairPoint> ContactPoints;
  ContactPoints _contact_points;

  friend class PhysContactCallbackData;
};

/**
 * Callback data for physics contact events.
 */
class EXPCL_PANDA_PPHYSICS PhysContactCallbackData : public RefCallbackData {
PUBLISHED:
  PhysContactCallbackData(const physx::PxContactPairHeader &header);
  ~PhysContactCallbackData() = default;

  INLINE PhysRigidActorNode *get_actor_a() const;
  INLINE PhysRigidActorNode *get_actor_b() const;

  INLINE const PhysContactPair *get_contact_pair(size_t n) const;
  INLINE size_t get_num_contact_pairs() const;

private:
  // The two actors in contact.
  PhysRigidActorNode *_a;
  PhysRigidActorNode *_b;

  typedef pvector<PhysContactPair> Pairs;
  Pairs _contact_pairs;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RefCallbackData::init_type();
    register_type(_type_handle, "PhysContactCallbackData",
                  RefCallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physContactCallbackData.I"

#endif // PHYSCONTACTCALLBACKDATA_H
