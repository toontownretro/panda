// Filename: actorNode.cxx
// Created by:  charles (07Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "actorNode.h"
#include "config_physics.h"
#include "physicsObject.h"

#include <transformTransition.h>

TypeHandle ActorNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : ActorNode
//       Access : public
//  Description : Constructor
////////////////////////////////////////////////////////////////////
ActorNode::
ActorNode(const string &name) :
  PhysicalNode(name), _parent_arc(NULL) {
  add_physical(new Physical(1, true));
  _mass_center = get_physical(0)->get_phys_body();
  _mass_center->set_active(true);
  _ok_to_callback = true;
}

////////////////////////////////////////////////////////////////////
//     Function : ActorNode
//       Access : public
//  Description : Copy Constructor.  This does NOT copy the parent
//                arc, but does deep copy the physical/physicsObject
////////////////////////////////////////////////////////////////////
ActorNode::
ActorNode(const ActorNode &copy) :
  PhysicalNode(copy) {
  _parent_arc = (RenderRelation *) NULL;
  _ok_to_callback = true;
  _mass_center = get_physical(0)->get_phys_body();
}

////////////////////////////////////////////////////////////////////
//     Function : ~ActorNode
//       Access : public
//  Description : destructor
////////////////////////////////////////////////////////////////////
ActorNode::
~ActorNode(void) {
}

////////////////////////////////////////////////////////////////////
//     Function : update_arc
//       Access : public
//  Description : this populates the parent arc with the transform
//                generated by the contained Physical, moving the
//                node and subsequent geometry.
////////////////////////////////////////////////////////////////////
void ActorNode::
update_arc(void) {
  nassertv(_parent_arc != (RenderRelation *) NULL);

  LMatrix4f lcs = _mass_center->get_lcs();

  // lock the callback so that this doesn't call transform_changed.
  _ok_to_callback = false;
  _parent_arc->set_transition(new TransformTransition(lcs));
  _ok_to_callback = true;
}

////////////////////////////////////////////////////////////////////
//     Function : transform_changed
//       Access : private, virtual
//  Description : node hook.  This function handles outside
//                (non-physics) actions on the actor
//                and updates the internal representation of the node.
////////////////////////////////////////////////////////////////////
void ActorNode::
transform_changed(NodeRelation *arc) {
  // this callback could be triggered by update_arc, BAD.
  if (_ok_to_callback == false)
    return;

  TransformTransition *tt;

  // get the transition
  tt = (TransformTransition *)
    arc->get_transition(TransformTransition::get_class_type());

  // extract the position

  LPoint3f pos;

  tt->get_matrix().get_row3(pos,3);

  // extract the orientation
  if (_mass_center->get_oriented() == true) {
    LOrientationf orientation(tt->get_matrix());
    _mass_center->set_orientation(orientation);
  }

  // apply
  _mass_center->set_position(pos);
}
