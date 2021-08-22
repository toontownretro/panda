/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterMaker.cxx
 * @author drose
 * @date 2002-03-06
 */

#include "characterMaker.h"
#include "eggLoader.h"
#include "config_egg2pg.h"
#include "eggBinner.h"
#include "eggGroup.h"
#include "eggPrimitive.h"
#include "eggBin.h"
#include "characterJoint.h"
#include "characterSlider.h"
#include "character.h"
#include "geomNode.h"
#include "transformState.h"
#include "eggSurface.h"
#include "eggCurve.h"
#include "characterVertexSlider.h"
#include "jointVertexTransform.h"
#include "userVertexTransform.h"
#include "eggAnimPreload.h"

using std::string;





/**
 *
 */
CharacterMaker::
CharacterMaker(EggGroup *root, EggLoader &loader, bool structured)
  : _loader(loader), _egg_root(root) {

  _character_node = new CharacterNode(_egg_root->get_name());
  _bundle = _character_node->get_character();

  _structured = structured;
}

/**
 *
 */
CharacterNode *CharacterMaker::
make_node() {
  make_bundle();
  return _character_node;
}

/**
 * Returns the name of the character.
 */
string CharacterMaker::
get_name() const {
  return _egg_root->get_name();
}

/**
 * Returns the CharacterJoint index associated with the given egg node, or -1
 * if there is no association.
 */
int CharacterMaker::
egg_to_joint(EggNode *egg_node) const {
  NodeMap::const_iterator jmi = _joint_map.find(egg_node);
  if (jmi == _joint_map.end()) {
    return -1;
  }

  return (*jmi).second;
}

/**
 * Returns a JointVertexTransform suitable for applying the animation
 * associated with the given egg node (which should be a joint).  Returns an
 * identity transform if the egg node is not a joint in the character's
 * hierarchy.
 */
VertexTransform *CharacterMaker::
egg_to_transform(EggNode *egg_node) {
  int index = egg_to_joint(egg_node);
  if (index < 0) {
    // Not a joint in the hierarchy.
    return get_identity_transform();
  }

  VertexTransforms::iterator vi = _vertex_transforms.find(index);
  if (vi != _vertex_transforms.end()) {
    return (*vi).second;
  }

  PT(VertexTransform) vt = new JointVertexTransform(_bundle, index);
  _vertex_transforms[index] = vt;

  return vt;
}

/**
 * Returns the scene graph node associated with the given PartGroup node, if
 * there is one.  If the PartGroup does not have an associated node, returns
 * the character's top node.
 */
PandaNode *CharacterMaker::
part_to_node(const string &name) const {
  PandaNode *node = _character_node;

  //if (part->is_character_joint()) {
    //CharacterJoint *joint = DCAST(CharacterJoint, part);
    //if (joint->_geom_node != nullptr) {
    //  node = joint->_geom_node;
    //}
  //}

  // We should always return a GeomNode, so that all polysets created at the
  // same level will get added into the same GeomNode.  Look for a child of
  // this node.  If it doesn't have a child yet, add a GeomNode and return it.
  // Otherwise, if it already has a child, return that.
  if (node->is_geom_node() && node->get_name() == name) {
    return node;
  }
  for (int i = 0; i < node->get_num_children(); i++) {
    PandaNode *child = node->get_child(i);
    if (child->is_geom_node() && child->get_name() == name) {
      return child;
    }
  }
  PT(GeomNode) geom_node = new GeomNode(name);
  node->add_child(geom_node);
  return geom_node;
}


/**
 * Creates a new morph slider of the given name, and returns its index.
 */
int CharacterMaker::
create_slider(const string &name) {
  int slider = _bundle->make_slider(name);
  return slider;
}

/**
 * Returns the VertexSlider corresponding to the indicated egg slider name.
 */
VertexSlider *CharacterMaker::
egg_to_vertex_slider(const string &name) {
  VertexSliders::iterator vi = _vertex_sliders.find(name);
  if (vi != _vertex_sliders.end()) {
    return (*vi).second;
  }

  int index = create_slider(name);
  PT(VertexSlider) slider =
    new CharacterVertexSlider(_bundle, index);
  _vertex_sliders[name] = slider;
  return slider;
}


/**
 *
 */
Character *CharacterMaker::
make_bundle() {
  build_joint_hierarchy(_egg_root, -1);

  // if we are structured, the egg loader is going to take care of making the
  // geometry
  if(!_structured) {
    make_geometry(_egg_root);
  }
  //_bundle->sort_descendants();

  parent_joint_nodes();

  // Now call update() one more time, to ensure that all of the joints have
  // their correct transform (since we might have modified the default
  // transform after construction).
  _bundle->recompute_joint_net_transforms();

  return _bundle;
}

/**
 *
 */
void CharacterMaker::
build_joint_hierarchy(EggNode *egg_node, int parent) {
  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, egg_node);

    // Each joint we come across is significant, and gets added to the
    // hierarchy.  Non-joints we encounter are ignored.
    if (egg_group->get_group_type() == EggGroup::GT_joint) {
      // We need to get the transform of the joint, and then convert it to
      // single-precision.
      LMatrix4d matd;

      // First, we get the original, initial transform from the <Transform>
      // entry.
      if (egg_group->has_transform()) {
        matd = egg_group->get_transform3d();
      } else {
        matd = LMatrix4d::ident_mat();
      }

      LMatrix4 matf = LCAST(PN_stdfloat, matd);

      int index = _bundle->make_joint(egg_group->get_name(), parent, matf);
      _joint_map[egg_group] = index;
      parent = index;

      // Now that we have computed _net_transform (which we need to convert
      // the vertices), update the default transform from the <DefaultPose>
      // entry.
      if (egg_group->get_default_pose().has_transform()) {
        matd = egg_group->get_default_pose().get_transform3d();
        matf = LCAST(PN_stdfloat, matd);
        _bundle->set_joint_default_value(index, matf);
      }

      if (egg_group->has_dcs_type()) {
        // If the joint requested an explicit DCS, create a node for it.
        PT(ModelNode) geom_node = new ModelNode(egg_group->get_name());

        // To prevent flattening from messing with geometry on exposed joints
        geom_node->set_preserve_transform(ModelNode::PT_net);

        _joint_dcs[index] = geom_node;
      }

      //part = joint;
    }

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      build_joint_hierarchy((*ci), parent);
    }
  }
}

/**
 * Walks the joint hierarchy, and parents any explicit nodes created for the
 * joints under the character node.
 */
void CharacterMaker::
parent_joint_nodes() {
  for (JointDCS::const_iterator jdi = _joint_dcs.begin(); jdi != _joint_dcs.end(); ++jdi) {
    int joint = (*jdi).first;
    ModelNode *joint_node = (*jdi).second;

    _character_node->add_child(joint_node);

    int attachment = _bundle->add_attachment(joint_node->get_name());
    // Parent the attachment to the joint.
    _bundle->add_attachment_parent(attachment, joint);
    _bundle->set_attachment_node(attachment, joint_node);
  }
}

/**
 * Walks the hierarchy, looking for bins that represent polysets, which are to
 * be animated with the character.  Invokes the egg loader to create the
 * animated geometry.
 */
void CharacterMaker::
make_geometry(EggNode *egg_node) {
  if (egg_node->is_of_type(EggBin::get_class_type())) {
    EggBin *egg_bin = DCAST(EggBin, egg_node);

    if (!egg_bin->empty() &&
        (egg_bin->get_bin_number() == EggBinner::BN_polyset ||
         egg_bin->get_bin_number() == EggBinner::BN_patches)) {
      EggGroupNode *bin_home = _egg_root;
      // This is a dynamic polyset that lives under the character's root
      // node.

      PandaNode *parent = part_to_node(/*egg_to_part(bin_home),*/ egg_bin->get_name());
      LMatrix4d transform =
        egg_bin->get_vertex_frame() *
        bin_home->get_node_frame_inv();

      _loader.make_polyset(egg_bin, parent, &transform, true,
                           this);
    }
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);

    EggGroupNode::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_geometry(*ci);
    }
  }
}

/**
 * Returns a VertexTransform that represents the root of the character--it
 * never animates.
 */
VertexTransform *CharacterMaker::
get_identity_transform() {
  if (_identity_transform == nullptr) {
    _identity_transform = new UserVertexTransform("root");
  }
  return _identity_transform;
}
