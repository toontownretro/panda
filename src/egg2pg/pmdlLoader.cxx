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
#include "materialPool.h"
#include "pdxList.h"
#include "animSequence.h"
#include "weightList.h"
#include "poseParameter.h"
#include "animBlendNode2D.h"
#include "mathutil_misc.h"

/**
 *
 */
bool PMDLDataDesc::
load(const Filename &filename, const DSearchPath &search_path) {
  Filename fullpath = filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(fullpath, search_path)) {
    return false;
  }

  _filename = filename;
  _fullpath = fullpath;

  PDXValue val;
  if (!val.read(fullpath, search_path)) {
    return false;
  }

  PDXElement *data = val.get_element();
  if (data == nullptr) {
    return false;
  }

  if (data->has_attribute("model")) {
    _model_filename = data->get_attribute_value("model").get_string();

  } else {
    return false;
  }

  if (data->has_attribute("scale")) {
    _scale = data->get_attribute_value("scale").get_float();
  }

  if (data->has_attribute("joint_merges")) {
    PDXList *jm_list = data->get_attribute_value("joint_merges").get_list();
    if (jm_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < jm_list->size(); i++) {
      _joint_merges.push_back(jm_list->get(i).get_string());
    }
  }

  if (data->has_attribute("material_groups")) {
    PDXList *mg_list = data->get_attribute_value("material_groups").get_list();
    if (mg_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < mg_list->size(); i++) {
      PDXElement *mg_elem = mg_list->get(i).get_element();
      if (mg_elem == nullptr) {
        return false;
      }
      PMDLMaterialGroup group;
      if (mg_elem->has_attribute("name")) {
        group._name = mg_elem->get_attribute_value("name").get_string();
      }
      if (mg_elem->has_attribute("materials")) {
        PDXList *mat_list = mg_elem->get_attribute_value("materials").get_list();
        if (mat_list == nullptr) {
          return false;
        }
        for (size_t j = 0; j < mat_list->size(); j++) {
          group._materials.push_back(mat_list->get(j).get_string());
        }
      }
      _material_groups.push_back(group);
    }
  }

  if (data->has_attribute("lods")) {
    PDXList *lod_list = data->get_attribute_value("lods").get_list();
    if (lod_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < lod_list->size(); i++) {
      PDXElement *lod_elem = lod_list->get(i).get_element();
      if (lod_elem == nullptr) {
        return false;
      }
      PMDLLODSwitch lod;
      if (lod_elem->has_attribute("distance")) {
        lod._distance = lod_elem->get_attribute_value("distance").get_float();
      }
      if (lod_elem->has_attribute("fade_time")) {
        lod._fade_in_time = lod_elem->get_attribute_value("fade_time").get_float();
      }
      if (lod_elem->has_attribute("center")) {
        if (!lod_elem->get_attribute_value("center").to_vec3(lod._center)) {
          return false;
        }
      }
      if (lod_elem->has_attribute("groups")) {
        PDXList *groups_list = lod_elem->get_attribute_value("groups").get_list();
        if (groups_list == nullptr) {
          return false;
        }
        for (size_t j = 0; j < groups_list->size(); j++) {
          lod._groups.push_back(groups_list->get(j).get_string());
        }
      }
      _lod_switches.push_back(lod);
    }
  }

  if (data->has_attribute("ik_chains")) {
    PDXList *chains_list = data->get_attribute_value("ik_chains").get_list();
    if (chains_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < chains_list->size(); i++) {
      PDXElement *chain_elem = chains_list->get(i).get_element();
      if (chain_elem == nullptr) {
        return false;
      }
      PMDLIKChain chain;
      if (chain_elem->has_attribute("name")) {
        chain._name = chain_elem->get_attribute_value("name").get_string();
      }
      if (chain_elem->has_attribute("end_joint")) {
        chain._end_joint = chain_elem->get_attribute_value("end_joint").get_string();
      }
      if (chain_elem->has_attribute("middle_joint_dir")) {
        if (!chain_elem->get_attribute_value("middle_joint_dir").to_vec3(chain._middle_joint_dir)) {
          return false;
        }
      }
      if (chain_elem->has_attribute("center")) {
        if (!chain_elem->get_attribute_value("center").to_vec3(chain._center)) {
          return false;
        }
      }
      if (chain_elem->has_attribute("height")) {
        chain._height = chain_elem->get_attribute_value("height").get_float();
      }
      if (chain_elem->has_attribute("floor")) {
        chain._floor = chain_elem->get_attribute_value("floor").get_float();
      }
      if (chain_elem->has_attribute("pad")) {
        chain._pad = chain_elem->get_attribute_value("pad").get_float();
      }
      _ik_chains.push_back(chain);
    }
  }

  if (data->has_attribute("pose_parameters")) {
    PDXList *pp_list = data->get_attribute_value("pose_parameters").get_list();
    if (pp_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < pp_list->size(); i++) {
      PDXElement *pp_elem = pp_list->get(i).get_element();
      if (pp_elem == nullptr) {
        return false;
      }
      PMDLPoseParameter pp;
      if (pp_elem->has_attribute("name")) {
        pp._name = pp_elem->get_attribute_value("name").get_string();
      }
      if (pp_elem->has_attribute("min")) {
        pp._min = pp_elem->get_attribute_value("min").get_float();
      }
      if (pp_elem->has_attribute("max")) {
        pp._max = pp_elem->get_attribute_value("max").get_float();
      }
      if (pp_elem->has_attribute("loop")) {
        pp._loop = pp_elem->get_attribute_value("loop").get_float();
      }
      _pose_parameters.push_back(pp);
    }
  }

  if (data->has_attribute("animations")) {
    PDXList *anims_list = data->get_attribute_value("animations").get_list();
    if (anims_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < anims_list->size(); i++) {
      PDXElement *anime = anims_list->get(i).get_element();
      if (anime == nullptr) {
        return false;
      }
      PMDLAnim anim;
      if (anime->has_attribute("name")) {
        anim._name = anime->get_attribute_value("name").get_string();
      }
      if (anime->has_attribute("filename")) {
        anim._anim_filename = anime->get_attribute_value("filename").get_string();
      }
      if (anime->has_attribute("fps")) {
        anim._fps = anime->get_attribute_value("fps").get_int();
      }
      _anims.push_back(anim);
    }
  }

  if (data->has_attribute("sequences")) {
    PDXList *seq_list = data->get_attribute_value("sequences").get_list();
    if (seq_list == nullptr) {
      return false;
    }
    for (size_t i = 0; i < seq_list->size(); i++) {
      PDXElement *seqe = seq_list->get(i).get_element();
      if (seqe == nullptr) {
        return false;
      }
      PMDLSequence seq;
      if (seqe->has_attribute("name")) {
        seq._name = seqe->get_attribute_value("name").get_string();
      }
      if (seqe->has_attribute("delta")) {
        seq._delta = seqe->get_attribute_value("delta").get_bool();
      }
      if (seqe->has_attribute("pre_delta")) {
        seq._pre_delta = seqe->get_attribute_value("pre_delta").get_bool();
      }
      if (seqe->has_attribute("loop")) {
        seq._loop = seqe->get_attribute_value("loop").get_bool();
      }
      if (seqe->has_attribute("snap")) {
        seq._snap = seqe->get_attribute_value("snap").get_bool();
      }
      if (seqe->has_attribute("zero_x")) {
        seq._zero_x = seqe->get_attribute_value("zero_x").get_bool();
      }
      if (seqe->has_attribute("zero_y")) {
        seq._zero_y = seqe->get_attribute_value("zero_y").get_bool();
      }
      if (seqe->has_attribute("zero_z")) {
        seq._zero_z = seqe->get_attribute_value("zero_z").get_bool();
      }
      if (seqe->has_attribute("real_time")) {
        seq._real_time = seqe->get_attribute_value("real_time").get_bool();
      }
      if (seqe->has_attribute("fps")) {
        seq._fps = seqe->get_attribute_value("fps").get_int();
      }
      if (seqe->has_attribute("fade_in")) {
        seq._fade_in = seqe->get_attribute_value("fade_in").get_float();
      }
      if (seqe->has_attribute("fade_out")) {
        seq._fade_out = seqe->get_attribute_value("fade_out").get_float();
      }
      if (seqe->has_attribute("weight_list")) {
        seq._weight_list_name = seqe->get_attribute_value("weight_list").get_string();
      }
      if (seqe->has_attribute("activity")) {
        seq._activity = seqe->get_attribute_value("activity").get_int();
      }
      if (seqe->has_attribute("activity_weight")) {
        seq._activity_weight = seqe->get_attribute_value("activity_weight").get_int();
      }
      if (seqe->has_attribute("anim")) {
        seq._animation_name = seqe->get_attribute_value("anim").get_string();
      }
      if (seqe->has_attribute("blend")) {
        PDXElement *blende = seqe->get_attribute_value("blend").get_element();
        nassertr(blende != nullptr, false);
        //if (blende->has_attribute("type")) {
        //  std::string blend_type = blende->get_attribute_value("type").get_string();
        //  if (blend_type == "2d") {
        //    seq._blend._blend_type = PMDLSequenceBlend::BT_2d;
        //  } else if (blend_type == "1d") {
        //    seq._blend._blend_type = PMDLSequenceBlend::BT_1d;
        //  } else {
        //    nassert_raise("Invalid blend type");
        //    return false;
        //  }
        //}
        seq._blend._blend_type = PMDLSequenceBlend::BT_2d;
        if (blende->has_attribute("width")) {
          seq._blend._blend_width = blende->get_attribute_value("width").get_int();
        }
        if (blende->has_attribute("blend_center")) {
          seq._blend._blend_center_sequence = blende->get_attribute_value("blend_center").get_string();
        }
        if (blende->has_attribute("blend_ref")) {
          seq._blend._blend_ref_sequence = blende->get_attribute_value("blend_ref").get_string();
        }
        if (blende->has_attribute("anims")) {
          PDXList *anims = blende->get_attribute_value("anims").get_list();
          nassertr(anims != nullptr, false);
          for (size_t j = 0; j < anims->size(); j++) {
            seq._blend._animations.push_back(anims->get(j).get_string());
          }
        }
        /*
        if (blende->has_attribute("controllers")) {
          PDXList *controllers = blende->get_attribute_value("controllers").get_list();
          nassertr(controllers != nullptr, false);
          for (size_t j = 0; j < controllers->size(); j++) {
            PDXElement *controller = controllers->get(j).get_element();
            nassertr(controller != nullptr, false);
            PMDLSequenceBlendController c;
            if (controller->has_attribute("pose_parameter")) {
              c._pose_parameter = controller->get_attribute_value("pose_parameter").get_string();
            }
            if (controller->has_attribute("min")) {
              c._min = controller->get_attribute_value("min").get_float();
            }
            if (controller->has_attribute("max")) {
              c._max = controller->get_attribute_value("max").get_float();
            }
            seq._blend._blend_controllers.push_back(c);
          }
        }
        */
       if (blende->has_attribute("blend_x")) {
         seq._blend._x_pose_param = blende->get_attribute_value("blend_x").get_string();
       }
       if (blende->has_attribute("blend_y")) {
         seq._blend._y_pose_param = blende->get_attribute_value("blend_y").get_string();
       }
      }
      if (seqe->has_attribute("layers")) {
        PDXList *layers = seqe->get_attribute_value("layers").get_list();
        nassertr(layers != nullptr, false);
        for (size_t j = 0; j < layers->size(); j++) {
          PDXElement *layere = layers->get(j).get_element();
          nassertr(layere != nullptr, false);
          PMDLSequenceLayer layer;
          if (layere->has_attribute("sequence")) {
            layer._sequence_name = layere->get_attribute_value("sequence").get_string();
          }
          if (layere->has_attribute("start_frame")) {
            layer._start_frame = layere->get_attribute_value("start_frame").get_int();
          }
          if (layere->has_attribute("peak_frame")) {
            layer._peak_frame = layere->get_attribute_value("peak_frame").get_int();
          }
          if (layere->has_attribute("tail_frame")) {
            layer._tail_frame = layere->get_attribute_value("tail_frame").get_int();
          }
          if (layere->has_attribute("end_frame")) {
            layer._end_frame = layere->get_attribute_value("end_frame").get_int();
          }
          if (layere->has_attribute("spline")) {
            layer._spline = layere->get_attribute_value("spline").get_bool();
          }
          if (layere->has_attribute("no_blend")) {
            layer._no_blend = layere->get_attribute_value("no_blend").get_bool();
          }
          seq._layers.push_back(layer);
        }
      }
      if (seqe->has_attribute("ik_locks")) {
        PDXList *locks = seqe->get_attribute_value("ik_locks").get_list();
        nassertr(locks != nullptr, false);
        for (size_t j = 0; j < locks->size(); j++) {
          PDXElement *locke = locks->get(j).get_element();
          nassertr(locke != nullptr, false);
          PMDLIKLock lock;
          if (locke->has_attribute("chain")) {
            lock._chain_name = locke->get_attribute_value("chain").get_string();
          }
          if (locke->has_attribute("pos")) {
            lock._pos_weight = locke->get_attribute_value("pos").get_float();
          }
          if (locke->has_attribute("rot")) {
            lock._rot_weight = locke->get_attribute_value("rot").get_float();
          }
          seq._ik_locks.push_back(lock);
        }
      }
      if (seqe->has_attribute("events")) {
        PDXList *events = seqe->get_attribute_value("events").get_list();
        nassertr(events != nullptr, false);
        for (size_t j = 0; j < events->size(); j++) {
          PDXElement *evente = events->get(j).get_element();
          nassertr(evente != nullptr, false);
          PMDLSequenceEvent event;
          if (evente->has_attribute("frame")) {
            event._frame = evente->get_attribute_value("frame").get_int();
          }
          if (evente->has_attribute("event")) {
            event._event = evente->get_attribute_value("event").get_int();
          }
          if (evente->has_attribute("type")) {
            event._type = evente->get_attribute_value("type").get_int();
          }
          if (evente->has_attribute("data")) {
            event._options = evente->get_attribute_value("data").get_string();
          }
          seq._events.push_back(event);
        }
      }
      _sequences.push_back(seq);
    }
  }

  if (data->has_attribute("hit_boxes")) {
    PDXList *hit_boxes = data->get_attribute_value("hit_boxes").get_list();
    nassertr(hit_boxes != nullptr, false);
    for (size_t i = 0; i < hit_boxes->size(); i++) {
      PDXElement *hitboxe = hit_boxes->get(i).get_element();
      nassertr(hitboxe != nullptr, false);
      PMDLHitBox hitbox;
      if (hitboxe->has_attribute("joint")) {
        hitbox._joint_name = hitboxe->get_attribute_value("joint").get_string();
      }
      if (hitboxe->has_attribute("group")) {
        hitbox._group = hitboxe->get_attribute_value("group").get_int();
      }
      if (hitboxe->has_attribute("min")) {
        if (!hitboxe->get_attribute_value("min").to_vec3(hitbox._min)) {
          return false;
        }
      }
      if (hitboxe->has_attribute("max")) {
        if (!hitboxe->get_attribute_value("max").to_vec3(hitbox._max)) {
          return false;
        }
      }
    }
  }

  if (data->has_attribute("weight_lists")) {
    PDXList *weight_lists = data->get_attribute_value("weight_lists").get_list();
    nassertr(weight_lists != nullptr, false);
    for (size_t i = 0; i < weight_lists->size(); i++) {
      PDXElement *weight_liste = weight_lists->get(i).get_element();
      nassertr(weight_liste != nullptr, false);
      PMDLWeightList wl;
      if (weight_liste->has_attribute("name")) {
        wl._name = weight_liste->get_attribute_value("name").get_string();
      }
      if (weight_liste->has_attribute("weights")) {
        PDXElement *weightse = weight_liste->get_attribute_value("weights").get_element();
        nassertr(weightse != nullptr, false);
        for (size_t j = 0; j < weightse->get_num_attributes(); j++) {
          wl._weights[weightse->get_attribute_name(j)] = weightse->get_attribute_value(j).get_float();
        }
      }
      _weight_lists.push_back(wl);
    }
  }

  if (data->has_attribute("attachments")) {
    PDXList *attaches = data->get_attribute_value("attachments").get_list();
    nassertr(attaches != nullptr, false);
    for (size_t i = 0; i < attaches->size(); i++) {
      PDXElement *attache = attaches->get(i).get_element();
      nassertr(attache != nullptr, false);
      PMDLAttachment attach;
      if (attache->has_attribute("name")) {
        attach._name = attache->get_attribute_value("name").get_string();
      }
      if (attache->has_attribute("parent")) {
        attach._parent_joint = attache->get_attribute_value("parent").get_string();
      }
      if (attache->has_attribute("pos")) {
        if (!attache->get_attribute_value("pos").to_vec3(attach._local_pos)) {
          return false;
        }
      }
      if (attache->has_attribute("hpr")) {
        if (!attache->get_attribute_value("hpr").to_vec3(attach._local_hpr)) {
          return false;
        }
      }
    }
  }

  if (data->has_attribute("custom_data")) {
    _custom_data = data->get_attribute_value("custom_data").get_element();
  }

  return true;
}

/**
 *
 */
PMDLLoader::
PMDLLoader(PMDLDataDesc *data) :
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
  for (size_t i = 0; i < _data->_material_groups.size(); i++) {
    PMDLMaterialGroup *group = &_data->_material_groups[i];
    MaterialCollection coll;
    for (size_t j = 0; j < group->_materials.size(); j++) {
      Filename mat_fname = group->_materials[j];
      coll.add_material(MaterialPool::load_material(mat_fname, search_path));
    }
    mdl_root->add_material_group(coll);
  }

  // LODs
  if (_data->_lod_switches.size() > (size_t)1) {
    PT(LODNode) lod_node = new LODNode("lod");

    // Figure out where to place the LODNode.  For now we'll naively use the
    // common ancestor between the first groups of the first two switches.
    NodePath group0 = root_np.find("**/" + _data->_lod_switches[0]._groups[0]);
    NodePath group1 = root_np.find("**/" + _data->_lod_switches[1]._groups[1]);
    NodePath lod_parent = group0.get_common_ancestor(group1);
    lod_parent.node()->add_child(lod_node);

    for (size_t i = 0; i < _data->_lod_switches.size(); i++) {
      PMDLLODSwitch *lod_switch = &_data->_lod_switches[i];

      float in_distance = lod_switch->_distance;
      float out_distance = i < (_data->_lod_switches.size() - 1) ?
        _data->_lod_switches[i + 1]._distance : FLT_MAX;

      if (lod_switch->_groups.size() > (size_t)1) {
        std::stringstream ss;
        ss << "switch_" << in_distance
           << "_" << out_distance;

        PT(PandaNode) switch_root = new PandaNode(ss.str());

        lod_node->add_child(switch_root);

        // There's more than one node/mesh in the group.

        for (size_t j = 0; j < lod_switch->_groups.size(); j++) {
          std::string group_name = lod_switch->_groups[j];
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
        std::string group_name = lod_switch->_groups[0];
        NodePath group_np = root_np.find("**/" + group_name);
        if (group_np.is_empty()) {
          egg2pg_cat.warning()
              << "Unable to find group " << group_name << " for LOD placement.\n";
            continue;
        }

        group_np.reparent_to(NodePath(lod_node));
      }

      lod_node->add_switch(out_distance,
                           in_distance);
    }
  }

  NodePath char_np = root_np.find("**/+CharacterNode");
  if (!char_np.is_empty()) {
    // This is an animated character.

    CharacterNode *char_node = DCAST(CharacterNode, char_np.node());
    Character *part_bundle = DCAST(Character, char_node->get_character());
    _part_bundle = part_bundle;

    pmap<std::string, AnimSequence *> seqs_by_name;
    pmap<std::string, PT(WeightList)> wls_by_name;

    // JOINT MERGES
    for (size_t i = 0; i < _data->_joint_merges.size(); i++) {
      int joint_idx = part_bundle->find_joint(_data->_joint_merges[i]);
      if (joint_idx == -1) {
        egg2pg_cat.error()
          << "Joint merge requested on a joint named " << _data->_joint_merges[i]
          << " but it does not exist in the character.\n";
        continue;
      }

      part_bundle->set_joint_merge(joint_idx, true);
    }

    // POSE PARAMETERS
    for (size_t i = 0; i < _data->_pose_parameters.size(); i++) {
      PMDLPoseParameter *pp = &_data->_pose_parameters[i];
      part_bundle->add_pose_parameter(pp->_name, pp->_min, pp->_max, pp->_loop);
    }

    // WEIGHT LISTS
    for (size_t i = 0; i < _data->_weight_lists.size(); i++) {
      PMDLWeightList *wl = &_data->_weight_lists[i];
      WeightListDesc desc(wl->_name);
      desc.set_weights(wl->_weights);
      PT(WeightList) list = new WeightList(part_bundle, desc);
      wls_by_name[wl->_name] = list;
    }

    // ANIMATIONS
    for (size_t i = 0; i < _data->_anims.size(); i++) {
      PMDLAnim *anim = &_data->_anims[i];
      AnimBundle *anim_bundle = load_anim(anim->_name, anim->_anim_filename);
      if (anim_bundle == nullptr) {
        continue;
      }
      if (anim->_fps != -1) {
        anim_bundle->set_base_frame_rate(anim->_fps);
      }
    }

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
    for (size_t i = 0; i < _data->_sequences.size(); i++) {
      PMDLSequence *pmdl_seq = &_data->_sequences[i];
      PT(AnimSequence) seq = new AnimSequence(pmdl_seq->_name);

      unsigned int flags = 0;
      if (pmdl_seq->_loop) {
        flags |= AnimSequence::F_looping;
      }
      if (pmdl_seq->_delta) {
        flags |= AnimSequence::F_delta | AnimSequence::F_post;
      } else if (pmdl_seq->_pre_delta) {
        flags |= AnimSequence::F_delta;
      }
      if (pmdl_seq->_zero_x) {
        flags |= AnimSequence::F_zero_root_x;
      }
      if (pmdl_seq->_zero_y) {
        flags |= AnimSequence::F_zero_root_y;
      }
      if (pmdl_seq->_zero_z) {
        flags |= AnimSequence::F_zero_root_z;
      }
      if (pmdl_seq->_snap) {
        flags |= AnimSequence::F_snap;
      }
      if (pmdl_seq->_real_time) {
        flags |= AnimSequence::F_real_time;
      }
      if (pmdl_seq->_animation_name.empty() && pmdl_seq->_blend._animations.empty()) {
        flags |= AnimSequence::F_all_zeros;
      }
      seq->set_flags(flags);

      seq->set_fade_out(pmdl_seq->_fade_out);
      seq->set_fade_in(pmdl_seq->_fade_in);

      if (pmdl_seq->_fps != -1) {
        seq->set_frame_rate(pmdl_seq->_fps);
      }
      if (pmdl_seq->_num_frames != -1) {
        seq->set_num_frames(pmdl_seq->_num_frames);
      }

      seq->set_activity(pmdl_seq->_activity, pmdl_seq->_activity_weight);

      if (!pmdl_seq->_animation_name.empty()) {
        // Single-animation sequence.
        AnimBundle *anim_bundle = find_or_load_anim(pmdl_seq->_animation_name);
        if (anim_bundle != nullptr) {
          seq->set_base(anim_bundle);
        }

      } else if (!pmdl_seq->_blend._animations.empty()) {
        // Blended multi-animation sequence.

        PT(AnimBlendNode2D) blend_node = new AnimBlendNode2D("blend_" + pmdl_seq->_name);

        int num_rows = pmdl_seq->_blend._animations.size() / pmdl_seq->_blend._blend_width;
        int num_cols = pmdl_seq->_blend._blend_width;
        for (size_t row = 0; row < num_rows; row++) {
          for (size_t col = 0; col < num_cols; col++) {
            size_t anim_index = (row * num_cols) + col;
            AnimBundle *anim_bundle = find_or_load_anim(pmdl_seq->_blend._animations[anim_index]);
            blend_node->add_input(anim_bundle,
              LPoint2(remap_val_clamped(col, 0, num_cols - 1, -1, 1),
                      remap_val_clamped(row, 0, num_rows - 1, -1, 1)));
          }
        }

        blend_node->set_input_x(part_bundle->find_pose_parameter(pmdl_seq->_blend._x_pose_param));
        blend_node->set_input_y(part_bundle->find_pose_parameter(pmdl_seq->_blend._y_pose_param));

        seq->set_base(blend_node);
      }

      // Sequence auto-layers.
      for (size_t j = 0; j < pmdl_seq->_layers.size(); j++) {
        PMDLSequenceLayer *pmdl_layer = &pmdl_seq->_layers[j];
        auto it = seqs_by_name.find(pmdl_layer->_sequence_name);
        if (it == seqs_by_name.end()) {
          egg2pg_cat.error()
            << "Layer sequence " << pmdl_layer->_sequence_name << " not found\n";
          continue;
        }
        AnimSequence *layer_seq = (*it).second;
        seq->add_layer(layer_seq, pmdl_layer->_start_frame,
                       pmdl_layer->_peak_frame, pmdl_layer->_tail_frame,
                       pmdl_layer->_end_frame, pmdl_layer->_spline,
                       pmdl_layer->_no_blend);
      }

      // Sequence events.
      for (size_t j = 0; j < pmdl_seq->_events.size(); j++) {
        PMDLSequenceEvent *event = &pmdl_seq->_events[j];
        seq->add_event(event->_type, event->_event, event->_frame, event->_options);
      }

      // Per-joint weight list.
      if (!pmdl_seq->_weight_list_name.empty()) {
        auto it = wls_by_name.find(pmdl_seq->_weight_list_name);
        if (it == wls_by_name.end()) {
          egg2pg_cat.error()
            << "Weight list " << pmdl_seq->_weight_list_name << " not found\n";
          continue;
        }
        seq->set_weight_list((*it).second);
      }

      // TODO: sequence ik locks and ik rules.

      seqs_by_name[pmdl_seq->_name] = seq;
      std::cout << "Ad sequence " << pmdl_seq->_name << "\n";
      part_bundle->add_sequence(seq);
    }

    pmap<int, NodePath> exposed_joints;

#if 0
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

  mdl_root->set_custom_data(_data->_custom_data);

  // Lightly flatten any extra transforms or attributes we applied to the
  // leaves.
  root_np.flatten_light();
}

/**
 *
 */
AnimBundle *PMDLLoader::
find_or_load_anim(const std::string &anim_name) {
  auto it = _anims_by_name.find(anim_name);
  if (it != _anims_by_name.end()) {
    return (*it).second;
  }

  return load_anim(anim_name, anim_name);
}

/**
 *
 */
AnimBundle *PMDLLoader::
load_anim(const std::string &anim_name, const Filename &filename) {
  Loader *loader = Loader::get_global_ptr();
  PT(PandaNode) anim_model = loader->load_sync(filename);
  if (anim_model == nullptr) {
    egg2pg_cat.error()
      << "Failed to load animation model " << filename << "\n";
    return nullptr;
  }
  NodePath anim_np(anim_model);
  NodePath anim_bundle_np = anim_np.find("**/+AnimBundleNode");
  if (anim_bundle_np.is_empty()) {
    egg2pg_cat.error()
      << "Model " << filename << " is not an animation!\n";
    return nullptr;
  }
  AnimBundleNode *anim_bundle_node = DCAST(AnimBundleNode, anim_bundle_np.node());
  AnimBundle *anim_bundle = anim_bundle_node->get_bundle();
  int anim_index = _part_bundle->bind_anim(anim_bundle);
  if (anim_index == -1) {
    egg2pg_cat.error()
      << "Failed to bind anim " << filename << " to character " << _part_bundle->get_name() << "\n";
    return nullptr;
  }
  _anims_by_name[anim_name] = anim_bundle;
  return anim_bundle;
}
