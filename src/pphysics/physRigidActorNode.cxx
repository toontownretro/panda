/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidActorNode.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physRigidActorNode.h"
#include "physScene.h"
#include "boundingBox.h"

#include "nodePath.h"

TypeHandle PhysRigidActorNode::_type_handle;

/**
 *
 */
PhysRigidActorNode::
PhysRigidActorNode(const std::string &name) :
  PandaNode(name),
  _sync_enabled(true),
  _from_collide_mask(BitMask32::all_on()),
  _into_collide_mask(BitMask32::all_on()),
  _needs_interpolation(false)
{
  _iv_pos.set_interpolation_amount(0.0f);
  _iv_rot.set_interpolation_amount(0.0f);
}

/**
 *
 */
PT(BoundingVolume) PhysRigidActorNode::
get_phys_bounds() const {
  physx::PxBounds3 px_bounds = get_rigid_actor()->getWorldBounds();
  PT(BoundingBox) bbox = new BoundingBox(physx_vec_to_panda(px_bounds.minimum), physx_vec_to_panda(px_bounds.maximum));
  return bbox;
}

/**
 * Adds this node into the indicated PhysScene.
 */
void PhysRigidActorNode::
add_to_scene(PhysScene *scene) {
  scene->get_scene()->addActor(*get_rigid_actor());
  scene->add_actor(this);
  on_new_scene();
}

/**
 * Callback hook when the actor is added to a new scene.
 */
void PhysRigidActorNode::
on_new_scene() {
}

/**
 * Removes this node from the indicated PhysScene.
 */
void PhysRigidActorNode::
remove_from_scene(PhysScene *scene) {
  scene->get_scene()->removeActor(*get_rigid_actor());
  scene->remove_actor(this);
}

/**
 *
 */
void PhysRigidActorNode::
set_collide_with(PhysRigidActorNode *other, bool flag) {
  do_set_collide_with(other, flag);
  other->do_set_collide_with(this, flag);
}

/**
 * Changes the contents mask of the node.
 */
void PhysRigidActorNode::
set_from_collide_mask(BitMask32 contents_mask) {
  if (_from_collide_mask == contents_mask) {
    return;
  }

  _from_collide_mask = contents_mask;

  // Update all shapes to use the new contents mask.
  update_shape_filter_data();
}

/**
 * Sets the mask of contents that are solid to the node.
 */
void PhysRigidActorNode::
set_into_collide_mask(BitMask32 solid_mask) {
  if (_into_collide_mask == solid_mask) {
    return;
  }

  _into_collide_mask = solid_mask;

  // Update all shapes to use the new solid mask.
  update_shape_filter_data();
}

/**
 *
 */
void PhysRigidActorNode::
update_shape_filter_data() {
  for (size_t i = 0; i < _shapes.size(); i++) {
    update_shape_filter_data(i);
  }
}

/**
 *
 */
void PhysRigidActorNode::
update_shape_filter_data(size_t n) {
  _shapes[n]->set_from_collide_mask(_from_collide_mask);
  _shapes[n]->set_into_collide_mask(_into_collide_mask);
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * PandaNode by duplicating instances (by calling dupe_for_flatten()), false
 * otherwise (for instance, a Camera cannot be safely flattened, because the
 * Camera pointer itself is meaningful).
 */
bool PhysRigidActorNode::
safe_to_flatten() const {
  return false;
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool PhysRigidActorNode::
safe_to_combine() const {
  return false;
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void PhysRigidActorNode::
xform(const LMatrix4 &mat) {
  // Transform all shapes by the matrix.  Can only be translated and
  // rotated.
  CPT(TransformState) trans = TransformState::make_mat(mat);
  nassertv(trans->get_scale().almost_equal(LVecBase3(1.0f)));
  nassertv(trans->get_shear().almost_equal(LVecBase3(0.0f)));
  physx::PxTransform pxtrans = panda_trans_to_physx(trans);
  for (size_t i = 0; i < _shapes.size(); i++) {
    physx::PxShape *shape = _shapes[i]->get_shape();
    shape->setLocalPose(pxtrans.transform(shape->getLocalPose()));
  }
  mark_internal_bounds_stale();
}

/**
 *
 */
void PhysRigidActorNode::
do_set_collide_with(PhysRigidActorNode *other, bool flag) {
  if (!flag) {
    Actors::const_iterator it = std::find(_no_collisions.begin(), _no_collisions.end(), other);
    if (it == _no_collisions.end()) {
      _no_collisions.push_back(other);
    }

  } else {
    Actors::const_iterator it = std::find(_no_collisions.begin(), _no_collisions.end(), other);
    if (it != _no_collisions.end()) {
      _no_collisions.erase(it);
    }
  }
}

/**
 *
 */
void PhysRigidActorNode::
parents_changed() {
  if (get_num_parents() > 0) {
    do_transform_changed();
  }
}

/**
 *
 */
void PhysRigidActorNode::
transform_changed() {
  do_transform_changed();
}

/**
 * Called when something other than the PhysX simulation caused the transform
 * of the node to change.  Synchronizes the node's new transform with the
 * associated PhysX actor.
 */
void PhysRigidActorNode::
do_transform_changed() {
  if (!_sync_enabled) {
    return;
  }

  NodePath np = NodePath::any_path((PandaNode *)this);

  CPT(TransformState) net_transform = np.get_net_transform();
  get_rigid_actor()->setGlobalPose(panda_trans_to_physx(net_transform));

  // Clear interpolation history.
  _iv_pos.reset(net_transform->get_pos());
  _iv_rot.reset(net_transform->get_norm_quat());
}

/**
 * Copies the world-space transform of this node onto the PhysX actor
 * immediately.
 */
void PhysRigidActorNode::
sync_transform() {
  do_transform_changed();
}
