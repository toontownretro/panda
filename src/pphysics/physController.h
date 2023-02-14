/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physController.h
 * @author brian
 * @date 2021-04-27
 */

#ifndef PHYSCONTROLLER_H
#define PHYSCONTROLLER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "luse.h"
#include "collideMask.h"
#include "pvector.h"
#include "deg_2_rad.h"
#include "physRigidDynamicNode.h"

#include "physx_includes.h"
#include "physx_utils.h"

class PhysShape;
class PhysRigidActorNode;
class PhysController;
class PhysBaseQueryFilter;

/**
 * Shared data for character and shape hits.
 */
class EXPCL_PANDA_PPHYSICS PhysControllerHitData {
PUBLISHED:
  INLINE PhysController *get_controller() const;
  INLINE const LPoint3 &get_world_pos() const;
  INLINE const LVector3 &get_world_normal() const;
  INLINE const LVector3 &get_motion_dir() const;
  INLINE PN_stdfloat get_motion_length() const;

private:
  PhysController *_controller;
  LPoint3 _world_pos;
  LVector3 _world_normal;
  LVector3 _motion_dir;
  PN_stdfloat _motion_length;

  friend class PhysControllerHitCallback;
};

/**
 * Data for a hit between a moving character and a non-moving shape.
 */
class EXPCL_PANDA_PPHYSICS PhysControllerShapeHitData : public PhysControllerHitData {
PUBLISHED:
  INLINE PhysShape *get_shape() const;
  INLINE PhysRigidActorNode *get_actor() const;
  INLINE uint32_t get_triangle_index() const;

private:
  PhysShape *_shape;
  PhysRigidActorNode *_actor;
  uint32_t _triangle_index;

  friend class PhysControllerHitCallback;
};

/**
 * Data for a hit between two characters.
 */
class EXPCL_PANDA_PPHYSICS PhysControllersHitData : public PhysControllerHitData {
PUBLISHED:
  INLINE PhysController *get_other_controller() const;

private:
  PhysController *_other;

  friend class PhysControllerHitCallback;
};

/**
 * Implementation of a PhysX callback when a character hits a shape or another
 * character.  Fills in the PhysController's shape and controller hit vectors.
 */
class PhysControllerHitCallback : public physx::PxUserControllerHitReport {
public:
  virtual void onShapeHit(const physx::PxControllerShapeHit &hit) override;
  virtual void onControllerHit(const physx::PxControllersHit &hit) override;
  virtual void onObstacleHit(const physx::PxControllerObstacleHit &hit) override;

  INLINE static PhysControllerHitCallback *get_global_ptr();

private:
  static PhysControllerHitCallback *_global_ptr;
};

/**
 * Filters collisions between character controllers.
 */
class PhysControllerFilterCallback : public physx::PxControllerFilterCallback {
public:
  virtual bool filter(const physx::PxController &a, const physx::PxController &b) override;
};

/**
 * Base character controller.
 */
class EXPCL_PANDA_PPHYSICS PhysController : public ReferenceCount {
PUBLISHED:
  enum ShapeType {
    ST_box,
    ST_capsule,
  };

  enum CollisionFlags {
    CF_none = 0,
    CF_sides = 1 << 0,
    CF_up = 1 << 1,
    CF_down = 1 << 2,
  };

  INLINE PhysController();

  INLINE ShapeType get_shape_type() const;

  INLINE void set_position(const LPoint3 &pos);
  INLINE LPoint3 get_position() const;

  INLINE void set_foot_position(const LPoint3 &pos);
  INLINE LPoint3 get_foot_position() const;
  MAKE_PROPERTY(foot_position, get_foot_position, set_foot_position);

  INLINE void set_step_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_step_offset() const;

  INLINE void set_contact_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_contact_offset() const;

  INLINE void set_up_direction(const LVector3 &dir);
  INLINE LVector3 get_up_direction() const;

  INLINE void set_slope_limit(PN_stdfloat limit);
  INLINE PN_stdfloat get_slope_limit() const;

  virtual void resize(PN_stdfloat size);

  INLINE void set_into_collide_mask(BitMask32 mask);
  INLINE BitMask32 get_into_collide_mask() const;

  void set_from_collide_mask(BitMask32 mask);
  INLINE BitMask32 get_from_collide_mask() const;

  INLINE PhysRigidDynamicNode *get_actor_node() const;
  INLINE PhysShape *get_actor_shape() const;

  CollisionFlags move(double dt, const LVector3 &move_vector,
                      PN_stdfloat min_distance,
                      BitMask32 collide_mask,
                      CallbackObject *filter = nullptr);

  //
  // Methods to retrieve collision state after a move() call.
  //

  INLINE unsigned int get_collision_flags() const;
  MAKE_PROPERTY(collision_flags, get_collision_flags);

  INLINE size_t get_num_shape_hits() const;
  INLINE const PhysControllerShapeHitData *get_shape_hit(size_t n) const;

  INLINE size_t get_num_controller_hits() const;
  INLINE const PhysControllersHitData *get_controller_hit(size_t n) const;

  virtual void destroy()=0;

protected:
  virtual physx::PxController *get_controller() const = 0;

  // Wrapper node around the internally created PxActor for the controller.
  PT(PhysRigidDynamicNode) _actor_node;

  // The node that gets controlled.
  NodePath _np;

  //
  // These get cleared and filled in each move() call.
  //

  // Hits with shapes in the world.
  typedef pvector<PhysControllerShapeHitData> ShapeHits;
  ShapeHits _shape_hits;

  // Hits with other controllers.
  typedef pvector<PhysControllersHitData> ControllerHits;
  ControllerHits _controller_hits;

  // Directions we had collisions in.
  unsigned int _collision_flags;

  friend class PhysControllerHitCallback;
};

#include "physController.I"

#endif // PHYSCONTROLLER_H
