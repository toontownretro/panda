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
#include "config_egg2pg.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "loader.h"
#include "lodNode.h"
#include "nodePath.h"
#include "characterNode.h"
#include "character.h"
#include "virtualFileSystem.h"
#include "ikChain.h"
#include "animChannelTable.h"
#include "animChannelBundle.h"
#include "animChannelBlend1D.h"
#include "animChannelBlend2D.h"
#include "animChannelLayered.h"
#include "materialPool.h"
#include "pdxList.h"
#include "weightList.h"
#include "poseParameter.h"
#include "mathutil_misc.h"
#include "animActivity.h"
#include "animEvent.h"
#include "eyeballNode.h"
#include "deg_2_rad.h"
#include "geomNode.h"
#include "materialAttrib.h"
#include "material.h"
#include "geomVertexReader.h"
#include "internalName.h"
#include "executionEnvironment.h"
#include "geomVertexReader.h"
#include "transformTable.h"
#include "jointVertexTransform.h"

#ifdef HAVE_PHYSX
#include "physConvexMeshData.h"
#endif

TypeHandle PMDLDataDesc::_type_handle;

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
      if (seqe->has_attribute("num_frames")) {
        seq._num_frames = seqe->get_attribute_value("num_frames").get_int();
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
        seq._activity = seqe->get_attribute_value("activity").get_string();
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
          if (layere->has_attribute("start")) {
            layer._start_frame = layere->get_attribute_value("start").get_float();
          }
          if (layere->has_attribute("peak")) {
            layer._peak_frame = layere->get_attribute_value("peak").get_float();
          }
          if (layere->has_attribute("tail")) {
            layer._tail_frame = layere->get_attribute_value("tail").get_float();
          }
          if (layere->has_attribute("end")) {
            layer._end_frame = layere->get_attribute_value("end").get_float();
          }
          if (layere->has_attribute("spline")) {
            layer._spline = layere->get_attribute_value("spline").get_bool();
          }
          if (layere->has_attribute("no_blend")) {
            layer._no_blend = layere->get_attribute_value("no_blend").get_bool();
          }
          if (layere->has_attribute("xfade")) {
            layer._xfade = layere->get_attribute_value("xfade").get_bool();
          }
          if (layere->has_attribute("pose_parameter")) {
            layer._pose_param = layere->get_attribute_value("pose_parameter").get_string();
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
            event._event = evente->get_attribute_value("event").get_string();
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
      if (attache->has_attribute("influences")) {
        PDXList *inf_list = attache->get_attribute_value("influences").get_list();
        nassertr(inf_list != nullptr, false);
        for (size_t j = 0; j < inf_list->size(); j++) {
          PDXElement *infe = inf_list->get(j).get_element();
          nassertr(infe != nullptr, false);
          PMDLAttachmentInfluence inf;
          if (infe->has_attribute("parent")) {
            inf._parent_joint = infe->get_attribute_value("parent").get_string();
          }
          if (infe->has_attribute("weight")) {
            inf._weight = infe->get_attribute_value("weight").get_float();
          }
          if (infe->has_attribute("pos")) {
            if (!infe->get_attribute_value("pos").to_vec3(inf._local_pos)) {
              return false;
            }
          }
          if (infe->has_attribute("hpr")) {
            if (!infe->get_attribute_value("hpr").to_vec3(inf._local_hpr)) {
              return false;
            }
          }
          attach._influences.push_back(inf);
        }
      }
      _attachments.push_back(attach);
    }
  }

  if (data->has_attribute("eyeballs")) {
    PDXList *eyes_list = data->get_attribute_value("eyeballs").get_list();
    nassertr(eyes_list != nullptr, false);
    for (size_t i = 0; i < eyes_list->size(); i++) {
      PDXElement *eyee = eyes_list->get(i).get_element();
      nassertr(eyee != nullptr, false);
      PMDLEyeball eye;
      if (eyee->has_attribute("name")) {
        eye._name = eyee->get_attribute_value("name").get_string();
      }
      if (eyee->has_attribute("material")) {
        eye._material_name = eyee->get_attribute_value("material").get_string();
      }
      if (eyee->has_attribute("parent")) {
        eye._parent = eyee->get_attribute_value("parent").get_string();
      }
      if (eyee->has_attribute("shift")) {
        if (!eyee->get_attribute_value("shift").to_vec3(eye._eye_shift)) {
          return false;
        }
      }
      if (eyee->has_attribute("pos")) {
        if (!eyee->get_attribute_value("pos").to_vec3(eye._pos)) {
          return false;
        }
      }
      if (eyee->has_attribute("diameter")) {
        eye._diameter = eyee->get_attribute_value("diameter").get_float();
      }
      if (eyee->has_attribute("iris_size")) {
        eye._iris_size = eyee->get_attribute_value("iris_size").get_float();
      }
      if (eyee->has_attribute("size")) {
        eye._eye_size = eyee->get_attribute_value("size").get_float();
      }
      if (eyee->has_attribute("z_offset")) {
        eye._z_offset = eyee->get_attribute_value("z_offset").get_float();
      }

      _eyeballs.push_back(eye);
    }
  }

  if (data->has_attribute("physics_model")) {
    PDXElement *pme = data->get_attribute_value("physics_model").get_element();
    nassertr(pme != nullptr, false);
    if (pme->has_attribute("name")) {
      _phy._name = pme->get_attribute_value("name").get_string();
    }
    if (pme->has_attribute("mesh")) {
      _phy._mesh_name = pme->get_attribute_value("mesh").get_string();
    }
    if (pme->has_attribute("concave")) {
      _phy._use_exact_geometry = pme->get_attribute_value("concave").get_bool();
    }
    if (pme->has_attribute("auto_mass")) {
      _phy._auto_mass = pme->get_attribute_value("auto_mass").get_bool();
    }
    if (pme->has_attribute("mass")) {
      // If we got explicit mass then we are not doing auto-mass.
      _phy._auto_mass = false;
      _phy._mass_override = pme->get_attribute_value("mass").get_float();
    }
    if (pme->has_attribute("rot_damping")) {
      _phy._rot_damping = pme->get_attribute_value("rot_damping").get_float();
    }
    if (pme->has_attribute("damping")) {
      _phy._damping = pme->get_attribute_value("damping").get_float();
    }
    if (pme->has_attribute("density")) {
      _phy._density = pme->get_attribute_value("density").get_float();
    }

    if (pme->has_attribute("joints")) {
      // Defines a jointed collision model.  The physics mesh is expected to
      // be associated with joints on the character model, each "piece"
      // hard-skinned to one joint.
      PDXList *joints_list = pme->get_attribute_value("joints").get_list();
      nassertr(joints_list != nullptr, false);

      for (size_t i = 0; i < joints_list->size(); i++) {
        PDXElement *jointe = joints_list->get(i).get_element();
        nassertr(jointe != nullptr, false);
        PMDLPhysicsJoint joint;
        if (jointe->has_attribute("name")) {
          joint._joint_name = jointe->get_attribute_value("name").get_string();
        }
        if (jointe->has_attribute("mass_bias")) {
          joint._mass_bias = jointe->get_attribute_value("mass_bias").get_float();
        }
        if (jointe->has_attribute("rot_damping")) {
          joint._rot_damping = jointe->get_attribute_value("rot_damping").get_float();
        }
        if (jointe->has_attribute("limit_x")) {
          jointe->get_attribute_value("limit_x").to_vec2(joint._limit_x);
        }
        if (jointe->has_attribute("limit_y")) {
          jointe->get_attribute_value("limit_y").to_vec2(joint._limit_y);
        }
        if (jointe->has_attribute("limit_z")) {
          jointe->get_attribute_value("limit_z").to_vec2(joint._limit_z);
        }
        if (jointe->has_attribute("collide")) {
          // Explicit collide-with list.
          PDXList *collide = jointe->get_attribute_value("collide").get_list();
          for (size_t j = 0; j < collide->size(); j++) {
            joint._collide_with.push_back(collide->get(j).get_string());
          }
        }
        _phy._joints.push_back(joint);
      }
    }
  }

  if (data->has_attribute("pos")) {
    if (!data->get_attribute_value("pos").to_vec3(_pos)) {
      return false;
    }
  }

  if (data->has_attribute("hpr")) {
    if (!data->get_attribute_value("hpr").to_vec3(_hpr)) {
      return false;
    }
  }

  if (data->has_attribute("scale")) {
    if (!data->get_attribute_value("scale").to_vec3(_scale)) {
      return false;
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

  _search_path = get_model_path();
  _search_path.append_directory(_data->_fullpath.get_dirname());

  Filename model_filename = _data->_model_filename;
  if (!vfs->resolve_filename(model_filename, _search_path)) {
    egg2pg_cat.error()
      << "Couldn't find pmdl model file " << model_filename << " on search path "
      << _search_path << "\n";
    return;
  }

  Loader *loader = Loader::get_global_ptr();

  _root = loader->load_sync(model_filename);
  if (_root == nullptr) {
    egg2pg_cat.error()
      << "Unable to build graph from egg file " << model_filename << "\n";
    return;
  }
  NodePath root_np(_root);
  ModelRoot *mdl_root = DCAST(ModelRoot, _root);

  // SCALE
  root_np.set_scale(_data->_scale);
  root_np.set_pos(_data->_pos);
  root_np.set_hpr(_data->_hpr);

  // MATERIAL GROUPS
  for (size_t i = 0; i < _data->_material_groups.size(); i++) {
    PMDLMaterialGroup *group = &_data->_material_groups[i];
    MaterialCollection coll;
    for (size_t j = 0; j < group->_materials.size(); j++) {
      Filename mat_fname = group->_materials[j];
      coll.add_material(MaterialPool::load_material(mat_fname, _search_path));
    }
    mdl_root->add_material_group(coll);
  }

  // LODs
  if (_data->_lod_switches.size() > (size_t)1) {
    PT(LODNode) lod_node = new LODNode("lod");

    // Figure out where to place the LODNode.  For now we'll naively use the
    // common ancestor between the first groups of the first two switches.
    NodePath group0 = root_np.find("**/" + _data->_lod_switches[0]._groups[0]);
    NodePath group1 = root_np.find("**/" + _data->_lod_switches[1]._groups[0]);
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

    AnimActivity *activities = AnimActivity::ptr();
    AnimEvent *events = AnimEvent::ptr();

    CharacterNode *char_node = DCAST(CharacterNode, char_np.node());
    Character *part_bundle = DCAST(Character, char_node->get_character());
    _part_bundle = part_bundle;

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

    // IK CHAINS
    for (size_t i = 0; i < _data->_ik_chains.size(); i++) {
      PMDLIKChain *chain = &_data->_ik_chains[i];

      int end_joint = part_bundle->find_joint(chain->_end_joint);
      if (end_joint == -1) {
        egg2pg_cat.error()
          << "IK chain " << chain->_name << ": end joint " << chain->_end_joint << " not found\n";
        continue;
      }

      int middle_joint = part_bundle->get_joint_parent(end_joint);
      if (middle_joint == -1) {
        egg2pg_cat.error()
          << "IK chain " << chain->_name << ": end joint " << chain->_end_joint << " must have a parent\n";
        continue;
      }

      int top_joint = part_bundle->get_joint_parent(middle_joint);
      if (top_joint == -1) {
        egg2pg_cat.error()
          << "IK chain " << chain->_name << ": middle joint " << part_bundle->get_joint_name(middle_joint)
          << " must have a parent\n";
        continue;
      }

      part_bundle->add_ik_chain(
        chain->_name, top_joint, middle_joint, end_joint,
        chain->_middle_joint_dir, chain->_center,
        chain->_height, chain->_floor, chain->_pad);

      egg2pg_cat.debug()
        << "Added ik chain " << chain->_name << "\n";
    }

    // SEQUENCES

    for (size_t i = 0; i < _data->_sequences.size(); i++) {
      PMDLSequence *pmdl_seq = &_data->_sequences[i];

      PT(AnimChannel) chan;
      bool layered = false;

      if (!pmdl_seq->_layers.empty()) {
        chan = make_layered_channel(pmdl_seq);
        layered = true;

      } else if (!pmdl_seq->_blend._animations.empty()) {
        chan = make_blend_channel(pmdl_seq->_blend, pmdl_seq->_fps);

      } else if (!pmdl_seq->_animation_name.empty()) {
        chan = find_or_load_anim(pmdl_seq->_animation_name);
      }

      if (chan == nullptr) {
        continue;
      }

      chan->set_name(pmdl_seq->_name);

      unsigned int flags = 0;
      if (pmdl_seq->_loop) {
        flags |= AnimChannel::F_looping;
      }
      if (!layered) {
        // For a layered channel these flags should only apply to the base
        // layer, not the overall layered channel.
        if (pmdl_seq->_real_time) {
          flags |= AnimChannel::F_real_time;
        }
        if (pmdl_seq->_zero_x) {
          flags |= AnimChannel::F_zero_root_x;
        }
        if (pmdl_seq->_zero_y) {
          flags |= AnimChannel::F_zero_root_y;
        }
        if (pmdl_seq->_zero_z) {
          flags |= AnimChannel::F_zero_root_z;
        }
        if (pmdl_seq->_delta) {
          flags |= AnimChannel::F_delta;
        } else if (pmdl_seq->_pre_delta) {
          flags |= AnimChannel::F_pre_delta;
        }
      }
      if (pmdl_seq->_snap) {
        flags |= AnimChannel::F_snap;
      }
      chan->set_flags(flags);

      chan->set_fade_out(pmdl_seq->_fade_out);
      chan->set_fade_in(pmdl_seq->_fade_in);

      if (pmdl_seq->_fps != -1) {
        chan->set_frame_rate(pmdl_seq->_fps);
      }
      if (pmdl_seq->_num_frames != -1) {
        chan->set_num_frames(pmdl_seq->_num_frames);
      }

      chan->add_activity(activities->get_value_id(pmdl_seq->_activity),
                         pmdl_seq->_activity_weight);

      // Sequence events.
      for (size_t j = 0; j < pmdl_seq->_events.size(); j++) {
        PMDLSequenceEvent *event = &pmdl_seq->_events[j];
        chan->add_event(event->_type, events->get_value_id(event->_event),
                        event->_frame, event->_options);
      }

      // Per-joint weight list.
      if (!pmdl_seq->_weight_list_name.empty()) {
        auto it = wls_by_name.find(pmdl_seq->_weight_list_name);
        if (it == wls_by_name.end()) {
          egg2pg_cat.error()
            << "Weight list " << pmdl_seq->_weight_list_name << " not found\n";
          continue;
        }
        chan->set_weight_list((*it).second);
      }

      // TODO: sequence ik locks and ik rules.

      _chans_by_name[pmdl_seq->_name] = chan;
      part_bundle->add_channel(chan);
    }

    // ATTACHMENTS
    for (size_t i = 0; i < _data->_attachments.size(); i++) {
      PMDLAttachment *pmdl_attach = &_data->_attachments[i];
      int index = part_bundle->add_attachment(pmdl_attach->_name);
      for (size_t j = 0; j < pmdl_attach->_influences.size(); j++) {
        PMDLAttachmentInfluence *pmdl_inf = &pmdl_attach->_influences[j];
        int parent = -1;
        if (!pmdl_inf->_parent_joint.empty()) {
          parent = part_bundle->find_joint(pmdl_inf->_parent_joint);
        }
        part_bundle->add_attachment_parent(index, parent, pmdl_inf->_local_pos,
                                           pmdl_inf->_local_hpr, pmdl_inf->_weight);
      }

      // Create a node to contain the attachment's transform.
      PT(ModelNode) attach_node = new ModelNode(pmdl_attach->_name);
      char_node->add_child(attach_node);

      // Link the node up with the attachment.
      part_bundle->set_attachment_node(index, attach_node);
    }
  }

  NodePathCollection all_geom_nodes = root_np.find_all_matches("**/+GeomNode");
  NodePathCollection eye_geom_nodes;

  for (size_t i = 0; i < _data->_eyeballs.size(); i++) {
    PMDLEyeball *pmdl_eye = &_data->_eyeballs[i];
    int parent_joint = _part_bundle->find_joint(pmdl_eye->_parent);
    if (parent_joint == -1) {
      egg2pg_cat.error()
        << "Eyeball " << pmdl_eye->_name << " parent joint " << pmdl_eye->_parent << " not found\n";
      continue;
    }

    // Need to create a copy of the eyeball for each unique parent of all eye
    // geom nodes.
    pmap<NodePath, NodePath> eyes_by_parent;

    eye_geom_nodes.clear();
    // Find all the geoms with the material that the eyeball specifies.  If
    // that is the only geom or all geoms in the node use the same material,
    // the geom node is moved directly under the eye.  Otherwise, the geoms
    // using the eyeball material are extracted into their own geom node and
    // parented to the eye.  If two eyeball geom nodes have different parents,
    // a copy of the eyeball is created for each unique parent.
    for (size_t j = 0; j < all_geom_nodes.size(); j++) {
      NodePath geom_np = all_geom_nodes[j];
      GeomNode *geom_node = DCAST(GeomNode, geom_np.node());
      vector_int eye_geoms;
      for (size_t k = 0; k < geom_node->get_num_geoms(); k++) {
        const Geom *geom = geom_node->get_geom(k);
        const RenderState *state = geom_node->get_geom_state(k);
        const MaterialAttrib *mattr;
        state->get_attrib_def(mattr);
        Material *mat = mattr->get_material();
        if (mat == nullptr) {
          continue;
        }
        if (mat->get_filename().get_basename_wo_extension() == pmdl_eye->_material_name) {
          // This is a geom for this eyeball.
          eye_geoms.push_back(k);
        }
      }

      if (eye_geoms.size() == 0) {
        continue;
      }

      if (eye_geoms.size() == geom_node->get_num_geoms()) {
        // All geoms use the eye material, so just use the geom node as-is.
        eye_geom_nodes.add_path(geom_np);

      } else {
        // Need to extract out just the eyeball geoms.
        std::ostringstream ss;
        ss << pmdl_eye->_material_name << "_eyeball_geom_" << eye_geom_nodes.size();
        PT(GeomNode) eye_geom_node = new GeomNode(ss.str());
        for (size_t k = 0; k < eye_geoms.size(); k++) {
          CPT(Geom) geom = geom_node->get_geom(eye_geoms[k]);
          CPT(RenderState) state = geom_node->get_geom_state(eye_geoms[k]);
          geom_node->remove_geom(eye_geoms[k]);
          eye_geom_node->add_geom((Geom *)geom.p(), (RenderState *)state.p());
        }
        geom_np.get_parent().node()->add_child(eye_geom_node);
        eye_geom_nodes.add_path(NodePath(eye_geom_node));
      }
    }

    PT(EyeballNode) eye = new EyeballNode(pmdl_eye->_name, _part_bundle, parent_joint);
    eye->set_radius(pmdl_eye->_diameter / 2.0f);
    eye->set_iris_scale(1.0f / pmdl_eye->_iris_size);
    eye->set_eye_size(pmdl_eye->_eye_size);
    eye->set_eye_shift(pmdl_eye->_eye_shift);
    eye->set_z_offset(std::tan(deg_2_rad(pmdl_eye->_z_offset)));
    // Convert character-space eye position to parent joint offset.
    CPT(TransformState) parent_joint_trans = TransformState::make_mat(
      _part_bundle->get_joint_net_transform(parent_joint));
    CPT(TransformState) eye_offset = parent_joint_trans->invert_compose(
      TransformState::make_pos(pmdl_eye->_pos));
    eye->set_eye_offset(eye_offset->get_pos());

    // Create a NodePath for the purpose of copying the eye to each unique
    // parent.
    NodePath copy_eye_np(eye);

    for (size_t k = 0; k < eye_geom_nodes.size(); k++) {
      NodePath eye_geom_np = eye_geom_nodes[k];
      NodePath parent = eye_geom_np.get_parent();
      nassertv(!parent.is_empty());
      auto it = eyes_by_parent.find(parent);
      NodePath eye_np;
      if (it == eyes_by_parent.end()) {
        // Haven't created an eyeball under this parent, copy the eye there.
        eye_np = copy_eye_np.copy_to(parent);
        eyes_by_parent[parent] = eye_np;
      } else {
        // Move the eye geom node under the existing eye.
        eye_np = (*it).second;
      }
      eye_geom_np.reparent_to(eye_np);
    }
  }

#ifdef HAVE_PHYSX
  if (!_data->_phy._mesh_name.empty()) {
    NodePath phy_mesh_np = root_np.find("**/" + _data->_phy._mesh_name);
    nassertv(!phy_mesh_np.is_empty());
    GeomNode *phy_mesh_node;
    DCAST_INTO_V(phy_mesh_node, phy_mesh_np.node());
    // Turn all the primitives into triangles.
    phy_mesh_node->decompose();

    if (_data->_phy._joints.empty()) {
      // Non-jointed, single-part collision model.

      PN_stdfloat mass = _data->_phy._mass_override;

      // Fill the convex mesh.
      PT(PhysConvexMeshData) mesh_data = new PhysConvexMeshData;
      for (size_t i = 0; i < phy_mesh_node->get_num_geoms(); i++) {
        const Geom *geom = phy_mesh_node->get_geom(i);
        const GeomVertexData *vdata = geom->get_vertex_data();
        GeomVertexReader reader(vdata, InternalName::get_vertex());
        for (size_t j = 0; j < geom->get_num_primitives(); j++) {
          const GeomPrimitive *prim = geom->get_primitive(j);
          for (size_t k = 0; k < prim->get_num_primitives(); k++) {
            size_t start = prim->get_primitive_start(k);
            size_t end = prim->get_primitive_end(k);
            for (size_t l = start; l < end; l++) {
              reader.set_row(prim->get_vertex(l));
              mesh_data->add_point(reader.get_data3f());
            }
          }
        }
      }
      if (!mesh_data->cook_mesh()) {
        egg2pg_cat.error()
          << "Failed to build convex mesh from physics geometry\n";
      } else if (!mesh_data->generate_mesh()) {
        egg2pg_cat.error()
          << "Failed to generate convex mesh\n";
      } else {
        if (_data->_phy._auto_mass) {
          mesh_data->get_mass_information(&mass, nullptr, nullptr);
        }
      }

      PT(ModelRoot::CollisionInfo) cinfo = new ModelRoot::CollisionInfo;
      ModelRoot::CollisionPart part;
      part.parent = -1;
      part.mass = mass;
      part.damping = _data->_phy._damping;
      part.rot_damping = std::max(0.0f, _data->_phy._rot_damping);
      part.mesh_data = mesh_data->get_mesh_data();
      cinfo->add_part(part);
      mdl_root->set_collision_info(cinfo);

    } else {
      // A multi-part jointed collision model.

      // Construct a convex mesh for each part of the collision model.
      // For each listed joint, find all the vertices inside the physics
      // mesh that are associated with the joint.

      PT(ModelRoot::CollisionInfo) cinfo = new ModelRoot::CollisionInfo;

      for (size_t i = 0; i < _data->_phy._joints.size(); i++) {
        const PMDLPhysicsJoint &pjoint = _data->_phy._joints[i];

        int char_joint = _part_bundle->find_joint(pjoint._joint_name);
        if (char_joint == -1) {
          egg2pg_cat.error()
            << "Collision model joint " << pjoint._joint_name << " does not exist in the Character!\n";
          continue;
        }

        // Now collect all the vertices in the physics mesh that are associated
        // with the corresponding character joint.
        PT(PhysConvexMeshData) mesh_data = new PhysConvexMeshData;

        for (size_t j = 0; j < phy_mesh_node->get_num_geoms(); j++) {
          const Geom *geom = phy_mesh_node->get_geom(j);
          const GeomVertexData *vdata = geom->get_vertex_data();
          const TransformTable *table = vdata->get_transform_table();
          nassertv(table != nullptr);
          GeomVertexReader vreader(vdata, InternalName::get_vertex());
          GeomVertexReader ireader(vdata, InternalName::get_transform_index());
          for (size_t k = 0; k < geom->get_num_primitives(); k++) {
            const GeomPrimitive *prim = geom->get_primitive(k);
            for (size_t l = 0; l < prim->get_num_primitives(); l++) {
              size_t start = prim->get_primitive_start(l);
              size_t end = prim->get_primitive_end(l);
              for (size_t m = start; m < end; m++) {
                size_t vertex = prim->get_vertex(m);
                vreader.set_row(vertex);
                ireader.set_row(vertex);
                int transform_index = ireader.get_data4i()[0];
                const JointVertexTransform *jvt = (JointVertexTransform *)table->get_transform(transform_index);
                nassertv(jvt != nullptr);
                if (jvt->get_joint() == char_joint) {
                  // Vertex is associated with this joint, add it to the convex mesh.
                  LPoint3 point = vreader.get_data3f();
                  // Move the vertex to be relative to the joint.
                  point = _part_bundle->get_joint_initial_net_transform_inverse(char_joint).xform_point(point);
                  mesh_data->add_point(point);
                }
              }
            }
          }
        }

        // We've now built up a list of vertices that are all associated with
        // the corresponding character joint.  Bake the convex mesh.
        if (!mesh_data->cook_mesh()) {
          egg2pg_cat.error()
            << "Failed to build convex mesh from physics geometry for joint " << pjoint._joint_name << "\n";
          return;
        } else if (!mesh_data->generate_mesh()) {
          egg2pg_cat.error()
            << "Failed to generate convex mesh for joint " << pjoint._joint_name << "\n";
          return;
        }

        PN_stdfloat part_volume;
        mesh_data->get_mass_information(&part_volume, nullptr, nullptr);

        ModelRoot::CollisionPart part;
        part.name = pjoint._joint_name;
        part.limit_x = pjoint._limit_x;
        part.limit_y = pjoint._limit_y;
        part.limit_z = pjoint._limit_z;
        part.mass = part_volume;
        part.damping = _data->_phy._damping;
        if (pjoint._rot_damping < 0.0f) {
          part.rot_damping = _data->_phy._rot_damping;
        } else {
          part.rot_damping = pjoint._rot_damping;
        }
        part.mesh_data = mesh_data->get_mesh_data();

        bool got_parent = false;
        int curr_char_joint = char_joint;
        do {
          int consider_parent = _part_bundle->get_joint_parent(curr_char_joint);
          if (consider_parent == -1) {
            // Hit top of hierarchy, no parent.
            part.parent = -1;
            got_parent = true;
            break;
          }
          std::string parent_name = _part_bundle->get_joint_name(consider_parent);
          // Find the part that is associated with this character joint.  If
          // none, continue searching up.
          for (size_t j = 0; j < _data->_phy._joints.size(); j++) {
            const PMDLPhysicsJoint &ppjoint = _data->_phy._joints[j];
            if (ppjoint._joint_name == parent_name) {
              // There is a collision part associated with our considered
              // parent joint!
              part.parent = (int)j;
              got_parent = true;
              break;
            }
          }
          // This is not a valid parent, try the next character joint one level
          // higher in the hierarchy.
          curr_char_joint = consider_parent;

        } while (!got_parent);

        // Build explicit collide list.
        for (const std::string &collide_name : pjoint._collide_with) {
          for (size_t j = 0; j < _data->_phy._joints.size(); j++) {
            if (_data->_phy._joints[j]._joint_name == collide_name) {
              part.collide_with.push_back((int)j);
            }
          }
        }

        cinfo->add_part(part);
      }

      PN_stdfloat total_mass = _data->_phy._mass_override;
      if (_data->_phy._auto_mass) {
        total_mass = 0.0f;
        for (size_t i = 0; i < _data->_phy._joints.size(); i++) {
          const ModelRoot::CollisionPart *part = cinfo->get_part(i);
          total_mass += part->mass * _data->_phy._density;
        }
      }

      PN_stdfloat total_volume = 0.0f;
      for (size_t i = 0; i < _data->_phy._joints.size(); i++) {
        const ModelRoot::CollisionPart *part = cinfo->get_part(i);
        const PMDLPhysicsJoint &pjoint = _data->_phy._joints[i];
        total_volume += part->mass * pjoint._mass_bias;
      }

      // Distribute total mass to parts.
      for (size_t i = 0; i < _data->_phy._joints.size(); i++) {
        const PMDLPhysicsJoint &pjoint = _data->_phy._joints[i];
        ModelRoot::CollisionPart *part = cinfo->modify_part(i);
        part->mass = ((part->mass * pjoint._mass_bias) / total_volume) * total_mass;
        if (part->mass < 1.0f) {
          part->mass = 1.0f;
        }
      }

      cinfo->total_mass = total_mass;

      mdl_root->set_collision_info(cinfo);
    }

    // Now remove the GeomNode that contained the physics geometry.
    phy_mesh_np.remove_node();
  }
#endif // HAVE_PHYSX

  mdl_root->set_custom_data(_data->_custom_data);
  mdl_root->set_final(true);

  // Lightly flatten any extra transforms or attributes we applied to the
  // leaves.
  root_np.flatten_light();
}

/**
 *
 */
PT(AnimChannel) PMDLLoader::
make_blend_channel(const PMDLSequenceBlend &blend, int fps) {
  int num_rows = blend._animations.size() / blend._blend_width;
  int num_cols = blend._blend_width;

  if (num_rows == 1) {
    // 1D blend space.
    PT(AnimChannelBlend1D) chan = new AnimChannelBlend1D("1dblend");

    for (size_t col = 0; col < num_cols; col++) {
      AnimChannelTable *anim_bundle = find_or_load_anim(blend._animations[col]);
      if (fps != -1) {
        anim_bundle->set_frame_rate(fps);
      }
      chan->add_channel(anim_bundle, (PN_stdfloat)col / (num_cols - 1));
    }

    chan->set_blend_param(_part_bundle->find_pose_parameter(blend._x_pose_param));

    return chan;

  } else {
    // 2D blend space.
    PT(AnimChannelBlend2D) chan = new AnimChannelBlend2D("2dblend");

    for (size_t row = 0; row < num_rows; row++) {
      for (size_t col = 0; col < num_cols; col++) {
        size_t anim_index = (row * num_cols) + col;
        AnimChannelTable *anim_bundle = find_or_load_anim(blend._animations[anim_index]);
        if (fps != -1) {
          anim_bundle->set_frame_rate(fps);
        }
        LPoint2 pt((PN_stdfloat)col / (num_cols - 1),
                    (PN_stdfloat)row / (num_rows - 1));
        chan->add_channel(anim_bundle, pt);
      }
    }

    chan->set_blend_x(_part_bundle->find_pose_parameter(blend._x_pose_param));
    chan->set_blend_y(_part_bundle->find_pose_parameter(blend._y_pose_param));

    return chan;
  }
}

/**
 *
 */
PT(AnimChannel) PMDLLoader::
make_layered_channel(const PMDLSequence *seq) {
  PT(AnimChannelLayered) chan = new AnimChannelLayered("layered");

  PT(AnimChannel) base_chan;
  // First start with the base layer.
  if (!seq->_blend._animations.empty()) {
    base_chan = make_blend_channel(seq->_blend, seq->_fps);

  } else if (!seq->_animation_name.empty()) {
    base_chan = find_or_load_anim(seq->_animation_name);
    if (seq->_fps != -1) {
      base_chan->set_frame_rate(seq->_fps);
    }
  }

  if (base_chan != nullptr) {
    chan->add_channel(base_chan);

    // If these flags appear on the sequence they should be applied to the
    // channel of the base layer, not the entire layered channel.
    unsigned int flags = 0;
    if (seq->_real_time) {
      flags |= AnimChannel::F_real_time;
    }
    if (seq->_zero_x) {
      flags |= AnimChannel::F_zero_root_x;
    }
    if (seq->_zero_y) {
      flags |= AnimChannel::F_zero_root_y;
    }
    if (seq->_zero_z) {
      flags |= AnimChannel::F_zero_root_z;
    }
    if (seq->_delta) {
      flags |= AnimChannel::F_delta;
    } else if (seq->_pre_delta) {
      flags |= AnimChannel::F_pre_delta;
    }
    base_chan->set_flags(flags);
  }

  for (int i = 0; i < seq->_layers.size(); i++) {
    const PMDLSequenceLayer *pmdl_layer = &seq->_layers[i];
    auto it = _chans_by_name.find(pmdl_layer->_sequence_name);
    if (it == _chans_by_name.end()) {
      egg2pg_cat.error()
        << "Layer sequence " << pmdl_layer->_sequence_name << " not found\n";
      continue;
    }
    AnimChannel *layer_chan = (*it).second;
    int pose_param = -1;
    if (!pmdl_layer->_pose_param.empty()) {
      pose_param = _part_bundle->find_pose_parameter(pmdl_layer->_pose_param);
      if (pose_param == -1) {
        egg2pg_cat.error()
          << "Sequence " << seq->_name << " layer " << pmdl_layer->_sequence_name
          << " pose parameter " << pmdl_layer->_pose_param << " not found\n";
        continue;
      }
    }
    chan->add_channel(layer_chan, pmdl_layer->_start_frame,
                      pmdl_layer->_peak_frame, pmdl_layer->_tail_frame,
                      pmdl_layer->_end_frame, pmdl_layer->_spline,
                      pmdl_layer->_no_blend, pmdl_layer->_xfade, pose_param);
  }

  return chan;
}

/**
 *
 */
AnimChannelTable *PMDLLoader::
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
AnimChannelTable *PMDLLoader::
load_anim(const std::string &anim_name, const Filename &filename) {
  Loader *loader = Loader::get_global_ptr();

  Filename fullpath = filename;
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(fullpath, _search_path)) {
    egg2pg_cat.error()
      << "Could not find animation model " << filename << "\n";
    return nullptr;
  }

  PT(PandaNode) anim_model = loader->load_sync(fullpath);
  if (anim_model == nullptr) {
    egg2pg_cat.error()
      << "Failed to load animation model " << fullpath << "\n";
    return nullptr;
  }
  NodePath anim_np(anim_model);
  NodePath anim_bundle_np = anim_np.find("**/+AnimChannelBundle");
  if (anim_bundle_np.is_empty()) {
    egg2pg_cat.error()
      << "Model " << fullpath << " is not an animation!\n";
    return nullptr;
  }
  AnimChannelBundle *anim_bundle_node = DCAST(AnimChannelBundle, anim_bundle_np.node());
  if (anim_bundle_node->get_num_channels() == 0) {
    egg2pg_cat.error()
      << "Animation model " << fullpath << " contains no channels\n";
    return nullptr;
  }
  AnimChannelTable *anim_bundle = DCAST(AnimChannelTable, anim_bundle_node->get_channel(0));
  anim_bundle->set_frame_rate(30);
  anim_bundle->set_name(anim_name);
  if (!_part_bundle->bind_anim(anim_bundle)) {
    egg2pg_cat.error()
      << "Failed to bind anim " << fullpath << " to character " << _part_bundle->get_name() << "\n";
    return nullptr;
  }
  _anims_by_name[anim_name] = anim_bundle;
  return anim_bundle;
}

/**
 *
 */
std::string PMDLDataDesc::
get_name() {
  return "model";
}

/**
 *
 */
std::string PMDLDataDesc::
get_source_extension() {
  return "pmdl";
}

/**
 *
 */
std::string PMDLDataDesc::
get_built_extension() {
  return "bam";
}

/**
 *
 */
void PMDLDataDesc::
get_dependencies(vector_string &filenames) {
  // We depend on the model .egg file and any animation .egg files.
  // They must be exported before the .pmdl is built, and we must
  // rebuild the .pmdl if any of the .eggs change.

  DSearchPath search_path = get_model_path();
  search_path.append_directory(ExecutionEnvironment::get_cwd());
  search_path.append_directory(_fullpath.get_dirname());

  Filename fullpath = _model_filename;
  fullpath.resolve_filename(search_path);
  filenames.push_back(fullpath);

  for (const PMDLAnim &anim : _anims) {
    fullpath = anim._anim_filename;
    fullpath.resolve_filename(search_path);
    filenames.push_back(fullpath.get_fullpath());
  }

  for (const PMDLSequence &seq : _sequences) {
    if (!seq._animation_name.empty()) {
      if (!Filename(seq._animation_name).get_extension().empty()) {
        // Depend on the single-animation file.
        fullpath = seq._animation_name;
        fullpath.resolve_filename(search_path);
        filenames.push_back(fullpath.get_fullpath());
      }

    } else if (!seq._blend._animations.empty()) {
      // If it's a blend sequence, depend on all the .egg files.
      for (const std::string &anim_filename : seq._blend._animations) {
        if (!Filename(anim_filename).get_extension().empty()) {
          fullpath = anim_filename;
          fullpath.resolve_filename(search_path);
          filenames.push_back(fullpath.get_fullpath());
        }
      }
    }
  }
}

/**
 *
 */
PT(AssetBase) PMDLDataDesc::
make_new() const {
  return new PMDLDataDesc;
}
