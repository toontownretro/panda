/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file actorNode.cxx
 * @author charles
 * @date 2000-08-07
 */

#include "actorNode.h"
#include "config_physics.h"
#include "physicsObject.h"

#include "transformState.h"

TypeHandle ActorNode::_type_handle;

/**
 * Constructor
 */
ActorNode::
ActorNode(const string &name) :
    PhysicalNode(name) {
  _contact_vector = LVector3::zero();
  add_physical(new Physical(1, true));
  _mass_center = get_physical(0)->get_phys_body();
  _mass_center->set_active(true);
  #ifndef NDEBUG
    _mass_center->set_name(name);
  #endif
  _ok_to_callback = true;
  _transform_limit = 0.0;
}

/**
 * Copy Constructor.
 */
ActorNode::
ActorNode(const ActorNode &copy) :
  PhysicalNode(copy) {
  _contact_vector = LVector3::zero();
  _ok_to_callback = true;
  _mass_center = get_physical(0)->get_phys_body();
  _transform_limit = copy._transform_limit;
}

/**
 * destructor
 */
ActorNode::
~ActorNode() {
}

/**
 * this sets the transform generated by the contained Physical, moving the
 * node and subsequent geometry.  i.e.  copy from PhysicsObject to PandaNode
 */
void ActorNode::
update_transform() {
  LMatrix4 lcs = _mass_center->get_lcs();

  // lock the callback so that this doesn't call transform_changed.
  _ok_to_callback = false;
  set_transform(TransformState::make_mat(lcs));
  _ok_to_callback = true;
}

/**
 * this tests the transform to make sure it's within the specified limits.
 * It's done so we can assert to see when an invalid transform is being
 * applied.
 */
void ActorNode::
test_transform(const TransformState *ts) const {
  LPoint3 pos(ts->get_pos());
  nassertv(pos[0] < _transform_limit);
  nassertv(pos[0] > -_transform_limit);
  nassertv(pos[1] < _transform_limit);
  nassertv(pos[1] > -_transform_limit);
  nassertv(pos[2] < _transform_limit);
  nassertv(pos[2] > -_transform_limit);
}

/**
 * node hook.  This function handles outside (non-physics) actions on the
 * actor and updates the internal representation of the node.  i.e.  copy from
 * PandaNode to PhysicsObject
 */
void ActorNode::
transform_changed() {
  PandaNode::transform_changed();

  // this callback could be triggered by update_transform, BAD.
  if (!_ok_to_callback) {
    return;
  }

  // get the transform
  CPT(TransformState) transform = get_transform();

  if (_transform_limit > 0.0) {
    test_transform(transform);
  }

  // extract the orientation
  if (_mass_center->get_oriented() == true) {
    _mass_center->set_orientation(transform->get_quat());
  }

  // apply
  _mass_center->set_position(transform->get_pos());
}


/**
 * Write a string representation of this instance to <out>.
 */
void ActorNode::
write(ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"ActorNode:\n";
  out.width(indent+2); out<<""; out<<"_ok_to_callback "<<_ok_to_callback<<"\n";
  out.width(indent+2); out<<""; out<<"_mass_center\n";
  _mass_center->write(out, indent+4);
  PhysicalNode::write(out, indent+2);
  #endif //] NDEBUG
}
