/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidActorNode.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSRIGIDACTORNODE_H
#define PHYSRIGIDACTORNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "physShape.h"
#include "collideMask.h"
#include "callbackObject.h"
#include "pvector.h"
#include "interpolatedVariable.h"

class PhysScene;

#include "physx_includes.h"
#include "physx_utils.h"

/**
 * Base class for rigid (non-deformable) objects in a scene.
 */
class EXPCL_PANDA_PPHYSICS PhysRigidActorNode : public PandaNode {
PUBLISHED:
  INLINE void add_shape(PhysShape *shape);
  INLINE void remove_shape(PhysShape *shape);

  PT(BoundingVolume) get_phys_bounds() const;

  INLINE size_t get_num_shapes() const;
  INLINE PhysShape *get_shape(size_t n) const;

  void add_to_scene(PhysScene *scene);
  void remove_from_scene(PhysScene *scene);

  INLINE PhysScene *get_scene() const;

  INLINE void set_wake_callback(CallbackObject *callback);
  INLINE CallbackObject *get_wake_callback() const;

  INLINE void set_sleep_callback(CallbackObject *callback);
  INLINE CallbackObject *get_sleep_callback() const;

  INLINE void set_trigger_callback(CallbackObject *callback);
  INLINE CallbackObject *get_trigger_callback() const;

  INLINE void set_contact_callback(CallbackObject *callback);
  INLINE CallbackObject *get_contact_callback() const;

  INLINE void set_advance_callback(CallbackObject *callback);
  INLINE CallbackObject *get_advance_callback() const;

  INLINE void set_contact_filter(CallbackObject *filter);
  INLINE void clear_contact_filter();
  INLINE CallbackObject *get_contact_filter() const;

  void set_collide_with(PhysRigidActorNode *other, bool flag);
  INLINE bool has_no_collide_with(PhysRigidActorNode *other) const;

  void set_from_collide_mask(BitMask32 contents_mask);
  INLINE BitMask32 get_from_collide_mask() const;

  void set_into_collide_mask(BitMask32 solid_mask);
  INLINE BitMask32 get_into_collide_mask() const;

  INLINE void set_simulation_disabled(bool flag);
  INLINE bool get_simulation_disabled() const;

  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);
  MAKE_SEQ_PROPERTY(shapes, get_num_shapes, get_shape);

  void update_shape_filter_data();
  void update_shape_filter_data(size_t n);

  virtual bool is_self_created() const { return true; }

  void sync_transform();

public:
  INLINE void set_sync_enabled(bool flag);
  INLINE bool get_sync_enabled() const;

  virtual bool safe_to_flatten() const override;
  virtual bool safe_to_combine() const override;
  virtual void xform(const LMatrix4 &mat) override;

protected:
  PhysRigidActorNode(const std::string &name);

  virtual void parents_changed() override;
  virtual void transform_changed() override;

  virtual void do_transform_changed();

  virtual void on_new_scene();

protected:
  void do_set_collide_with(PhysRigidActorNode *other, bool flag);

  // Set by the PhysScene when applying the simulation result onto the node.
  // Stops transform_changed() from being called while doing it.
  bool _sync_enabled;

  PT(CallbackObject) _wake_callback;
  PT(CallbackObject) _sleep_callback;
  PT(CallbackObject) _trigger_callback;
  PT(CallbackObject) _contact_callback;
  PT(CallbackObject) _advance_callback;

  PT(CallbackObject) _contact_filter;

  typedef pvector<PhysRigidActorNode *> Actors;
  Actors _no_collisions;

  BitMask32 _from_collide_mask;
  BitMask32 _into_collide_mask;

  pvector<PT(PhysShape)> _shapes;

  // Used to interpolate simulation results for rendering
  // with a fixed simulation timestep.
  InterpolatedVec3 _iv_pos;
  InterpolatedQuat _iv_rot;
  bool _needs_interpolation;

  friend class PhysScene;

public:
  virtual physx::PxRigidActor *get_rigid_actor() const = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PhysRigidActorNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#include "physRigidActorNode.I"

#endif // PHYSRIGIDACTORNODE_H
