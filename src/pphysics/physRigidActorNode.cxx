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

#include "nodePath.h"

TypeHandle PhysRigidActorNode::_type_handle;

/**
 *
 */
PhysRigidActorNode::
PhysRigidActorNode(const std::string &name) :
  PandaNode(name),
  _sync_enabled(true),
  _collision_group(0),
  _contents_mask(BitMask32::all_on()),
  _solid_mask(BitMask32::all_on())
{
}

/**
 * Adds this node into the indicated PhysScene.
 */
void PhysRigidActorNode::
add_to_scene(PhysScene *scene) {
  scene->get_scene()->addActor(*get_rigid_actor());
}

/**
 * Removes this node from the indicated PhysScene.
 */
void PhysRigidActorNode::
remove_from_scene(PhysScene *scene) {
  scene->get_scene()->removeActor(*get_rigid_actor());
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
 * Changes the collision group of the node.
 */
void PhysRigidActorNode::
set_collision_group(unsigned int collision_group) {
  if (_collision_group == collision_group) {
    return;
  }

  _collision_group = collision_group;

  // Update all shapes to use the new collision group.
  update_shape_filter_data();
}

/**
 * Changes the contents mask of the node.
 */
void PhysRigidActorNode::
set_contents_mask(BitMask32 contents_mask) {
  if (_contents_mask == contents_mask) {
    return;
  }

  _contents_mask = contents_mask;

  // Update all shapes to use the new contents mask.
  update_shape_filter_data();
}

/**
 * Sets the mask of contents that are solid to the node.
 */
void PhysRigidActorNode::
set_solid_mask(BitMask32 solid_mask) {
  if (_solid_mask == solid_mask) {
    return;
  }

  _solid_mask = solid_mask;

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
  physx::PxShape *shape = _shapes[n]->get_shape();
  physx::PxFilterData data = shape->getSimulationFilterData();
  physx::PxFilterData qdata = shape->getQueryFilterData();
  data.word0 = _collision_group;
  data.word1 = _contents_mask.get_word();
  data.word2 = _solid_mask.get_word();
  // For queries, the collision group goes on word1.  The contents mask needs
  // to go on word0 for PhysX's fixed-function filtering.
  qdata.word0 = _contents_mask.get_word();
  qdata.word1 = _collision_group;
  shape->setSimulationFilterData(data);
  shape->setQueryFilterData(qdata);
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
}

/**
 * Copies the world-space transform of this node onto the PhysX actor
 * immediately.
 */
void PhysRigidActorNode::
sync_transform() {
  do_transform_changed();
}
