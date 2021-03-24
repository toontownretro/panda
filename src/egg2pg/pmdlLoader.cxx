/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlLoader.cxx
 * @author lachbr
 * @date 2021-02-13
 */

#include "pmdlLoader.h"
#include "pmdlData.h"
#include "load_egg_file.h"
#include "config_egg2pg.h"
#include "modelNode.h"
#include "modelRoot.h"
//#include "materialGroup.h"
//#include "renderStatePool.h"
#include "loader.h"
#include "lodNode.h"
#include "nodePath.h"
#include "characterNode.h"
#include "character.h"
//#include "characterJointBundle.h"
#include "virtualFileSystem.h"
//#include "ikChain.h"
#include "animBundleNode.h"
#include "animBundle.h"
#include "characterJointEffect.h"

/**
 *
 */
PMDLLoader::
PMDLLoader(PMDLData *data) :
  _data(data) {
}

/**
 * Builds up the scene graph from the .pmdl file data.
 */
void PMDLLoader::
build_graph() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  DSearchPath search_path = get_model_path();
  search_path.append_directory(_data->_fullpath.get_dirname());

  Filename model_filename = _data->_model_filename;
  if (!vfs->resolve_filename(model_filename, search_path)) {
    egg2pg_cat.error()
      << "Couldn't find pmdl model file " << model_filename << " on search path "
      << search_path << "\n";
    return;
  }

  Loader *loader = Loader::get_global_ptr();

  _root = loader->load_sync(model_filename);
  if (!_root) {
    egg2pg_cat.error()
      << "Failed to build graph from model filename\n";
    return;
  }

  NodePath root_np(_root);
  ModelRoot *mdl_root = DCAST(ModelRoot, _root);

  // SCALE
  if (_data->_scale != 1.0) {
    root_np.set_scale(_data->_scale);
  }

  // MATERIAL GROUPS
  #if 0
  for (size_t i = 0; i < _data->_texture_groups.size(); i++) {
    PMDLTextureGroup *group = _data->_texture_groups[i];
    PT(MaterialGroup) mat_group = new MaterialGroup;
    for (size_t j = 0; j < group->get_num_materials(); j++) {
      Filename mat_fname = group->get_material(j);
      mat_group->add_material(RenderStatePool::load_state(mat_fname));
    }
    mdl_root->add_material_group(mat_group);
  }
  #endif

  // LODs
  if (_data->_lod_switches.size() > (size_t)1) {
    PT(LODNode) lod_node = new LODNode("lod");

    // Figure out where to place the LODNode.  For now we'll naively use the
    // common ancestor between the first groups of the first two switches.
    NodePath group0 = root_np.find("**/" + _data->_lod_switches[0]->get_group(0));
    NodePath group1 = root_np.find("**/" + _data->_lod_switches[1]->get_group(0));
    NodePath lod_parent = group0.get_common_ancestor(group1);
    lod_parent.node()->add_child(lod_node);

    for (size_t i = 0; i < _data->_lod_switches.size(); i++) {
      PMDLSwitch *lod_switch = _data->_lod_switches[i];

      if (lod_switch->get_num_groups() > (size_t)1) {
        std::stringstream ss;
        ss << "switch_" << lod_switch->get_in_distance()
           << "_" << lod_switch->get_out_distance();

        PT(PandaNode) switch_root = new PandaNode(ss.str());

        lod_node->add_child(switch_root);

        // There's more than one node/mesh in the group.

        for (size_t j = 0; j < lod_switch->get_num_groups(); j++) {
          std::string group_name = lod_switch->get_group(j);
          NodePath group_np = root_np.find("**/" + group_name);
          if (group_np.is_empty()) {
            egg2pg_cat.warning()
              << "Unable to find group " << group_name << " for LOD placement.\n";
            continue;
          }

          group_np.reparent_to(NodePath(switch_root));
        }

      } else {
        // Just one node in the group.  We can parent it directly to the
        // LODNode.
        std::string group_name = lod_switch->get_group(0);
        NodePath group_np = root_np.find("**/" + group_name);
        if (group_np.is_empty()) {
          egg2pg_cat.warning()
              << "Unable to find group " << group_name << " for LOD placement.\n";
            continue;
        }

        group_np.reparent_to(NodePath(lod_node));
      }

      lod_node->add_switch(lod_switch->get_out_distance(),
                           lod_switch->get_in_distance());
    }
  }

  NodePath char_np = root_np.find("**/+CharacterNode");
  if (!char_np.is_empty()) {
    CharacterNode *char_node = DCAST(CharacterNode, char_np.node());
    Character *part_bundle = DCAST(Character, char_node->get_character());

#if 0
    // IK CHAINS
    for (PMDLData::IKChains::const_iterator ici = _data->_ik_chains.begin();
         ici != _data->_ik_chains.end(); ++ici) {
      PMDLIKChain *pmdl_chain = (*ici).second;

      PartGroup *group = part_bundle->find_child(pmdl_chain->get_foot_joint());
      if (!group->is_of_type(CharacterJoint::get_class_type())) {
        egg2pg_cat.error()
          << "foot joint " << pmdl_chain->get_foot_joint() << " of ik chain "
          << pmdl_chain->get_name() << " is not of type CharacterJoint!\n";
        continue;
      }

      CharacterJoint *foot = DCAST(CharacterJoint, group);

      PT(IKChain) chain = new IKChain(pmdl_chain->get_name(), foot);
      chain->set_knee_direction(pmdl_chain->get_knee_direction());
      chain->set_center(pmdl_chain->get_center());
      chain->set_floor(pmdl_chain->get_floor());
      chain->set_height(pmdl_chain->get_height());
      chain->set_pad(pmdl_chain->get_pad());

      part_bundle->add_ik_chain(chain);

      egg2pg_cat.debug()
        << "Added ik chain " << pmdl_chain->get_name() << "\n";
    }
#endif
    // SEQUENCES
    for (PMDLData::Sequences::const_iterator si = _data->_sequences.begin();
         si != _data->_sequences.end(); ++si) {
      PMDLSequence *pmdl_seq = (*si).second;

      if (pmdl_seq->get_anim_filename().empty()) {
        egg2pg_cat.error()
          << "PMDL sequence " << pmdl_seq->get_name() << " has no animation file\n";
        continue;
      }

      Filename anim_filename = pmdl_seq->get_anim_filename();
      if (!vfs->resolve_filename(anim_filename, search_path)) {
        egg2pg_cat.error()
          << "Could not find anim file " << anim_filename.get_fullpath()
          << " for PMDL sequence " << pmdl_seq->get_name() << " on search path "
          << search_path << "\n";
        continue;
      }

      PT(PandaNode) anim_root = loader->load_sync(anim_filename);
      if (anim_root == nullptr) {
        egg2pg_cat.error()
          << "Could not load anim file " << anim_filename << " for PMDL "
          << "sequence " << pmdl_seq->get_name() << "\n";
        continue;
      }

      NodePath anim_root_np(anim_root);
      NodePath anim_bundle_np = anim_root_np.find("**/+AnimBundleNode");

      if (anim_bundle_np.is_empty()) {
        egg2pg_cat.error()
          << "Anim file " << anim_filename << " does not contain an AnimBundleNode!\n";
        continue;
      }

      AnimBundleNode *anim_bundle_node = DCAST(AnimBundleNode, anim_bundle_np.node());
      AnimBundle *anim_bundle = anim_bundle_node->get_bundle();

      // Apply some properties onto the AnimBundle that were specified in the
      // sequence.
      anim_bundle_node->set_name(pmdl_seq->get_name());
      anim_bundle->set_base_frame_rate(pmdl_seq->get_fps());

      // Move the AnimBundleNode to underneath the zero model.
      anim_bundle_np.reparent_to(root_np);
    }

    pmap<int, NodePath> exposed_joints;

#if 1
    // EXPOSES
    for (PMDLData::StringMap::const_iterator ei = _data->_exposes.begin();
         ei != _data->_exposes.end(); ++ei) {
      int joint = part_bundle->find_joint((*ei).first);
      if (joint == -1) {
        egg2pg_cat.error()
          << "expose joint " << (*ei).first << " not found.\n";
        continue;
      }
      NodePath np(new ModelNode((*ei).second));
      part_bundle->add_net_transform(joint, np.node());
      np.reparent_to(char_np);
      exposed_joints[joint] = np;
    }

    // ATTACHMENTS
    for (PMDLData::Attachments::const_iterator ai = _data->_attachments.begin();
         ai != _data->_attachments.end(); ++ai) {
      PMDLAttachment *attach = (*ai).second;

      PT(ModelNode) mod_node = new ModelNode(attach->get_name());
      mod_node->set_preserve_transform(ModelNode::PT_local);
      NodePath np(mod_node);
      np.set_pos(attach->get_pos());
      np.set_hpr(attach->get_hpr());

      if (attach->get_parent_joint().empty()) {
        // Just a static node relative to character root.
        np.reparent_to(char_np);

      } else {
        int joint = part_bundle->find_joint(attach->get_parent_joint());
        if (joint == -1) {
          egg2pg_cat.error()
            << "attachment parent joint " << attach->get_parent_joint()
            << " not found.\n";
          continue;
        }

        np.set_effect(CharacterJointEffect::make(char_node));

        auto ei = exposed_joints.find(joint);
        if (ei != exposed_joints.end()) {
          // We've already exposed this parent joint.
          np.reparent_to((*ei).second);

        } else {
          // Need to expose it.

          NodePath expose_np(new ModelNode(attach->get_parent_joint()));
          expose_np.reparent_to(char_np);
          part_bundle->add_net_transform(joint, expose_np.node());

          np.reparent_to(expose_np);

          exposed_joints[joint] = expose_np;
        }
      }
    }

#endif

  }

  // Lightly flatten any extra transforms or attributes we applied to the
  // leaves.
  root_np.flatten_light();
}
