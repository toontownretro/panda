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

class PhysScene;

#include "physx_includes.h"

/**
 * Base class for rigid (non-deformable) objects in a scene.
 */
class EXPCL_PANDA_PPHYSICS PhysRigidActorNode : public PandaNode {
PUBLISHED:
  INLINE void add_shape(PhysShape *shape);
  INLINE void remove_shape(PhysShape *shape);

  INLINE size_t get_num_shapes() const;
  INLINE PhysShape *get_shape(size_t n) const;

  void add_to_scene(PhysScene *scene);
  void remove_from_scene(PhysScene *scene);

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

  void set_collide_with(PhysRigidActorNode *other, bool flag);
  INLINE bool has_no_collide_with(PhysRigidActorNode *other) const;

  void set_collision_group(unsigned int collision_group);
  INLINE unsigned int get_collision_group() const;

  void set_contents_mask(BitMask32 contents_mask);
  INLINE BitMask32 get_contents_mask() const;

  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);
  MAKE_SEQ_PROPERTY(shapes, get_num_shapes, get_shape);

public:
  INLINE void set_sync_enabled(bool flag);
  INLINE bool get_sync_enabled() const;

protected:
  PhysRigidActorNode(const std::string &name);

  virtual void parents_changed() override;
  virtual void transform_changed() override;

  virtual void do_transform_changed();

private:
  void do_set_collide_with(PhysRigidActorNode *other, bool flag);

  // Set by the PhysScene when applying the simulation result onto the node.
  // Stops transform_changed() from being called while doing it.
  bool _sync_enabled;

  PT(CallbackObject) _wake_callback;
  PT(CallbackObject) _sleep_callback;
  PT(CallbackObject) _trigger_callback;
  PT(CallbackObject) _contact_callback;
  PT(CallbackObject) _advance_callback;

  typedef pvector<PhysRigidActorNode *> Actors;
  Actors _no_collisions;

  unsigned int _collision_group;
  BitMask32 _contents_mask;

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
