/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundleMaker.cxx
 * @author drose
 * @date 1999-02-22
 */

#include "animBundleMaker.h"
#include "config_egg2pg.h"

#include "eggTable.h"
#include "eggAnimData.h"
#include "eggSAnimData.h"
#include "eggXfmAnimData.h"
#include "eggXfmSAnim.h"
#include "eggGroupNode.h"
#include "animBundle.h"
#include "animBundleNode.h"

using std::min;

/**
 *
 */
AnimBundleMaker::
AnimBundleMaker(EggTable *root) : _root(root) {
  _fps = 0.0f;
  _num_frames = 1;
  _num_joints = 0;
  _num_sliders = 0;

  _ok_fps = true;
  _ok_num_frames = true;

  inspect_tree(root);

  if (!_ok_fps) {
    egg2pg_cat.warning()
      << "AnimBundle " << _root->get_name()
      << " specifies contradictory frame rates.\n";
  } else if (_fps == 0.0f) {
    egg2pg_cat.warning()
      << "AnimBundle " << _root->get_name()
      << " does not specify a frame rate.\n";
    _fps = 24.0f;
  }

  if (!_ok_num_frames) {
    egg2pg_cat.warning()
      << "AnimBundle " << _root->get_name()
      << " specifies contradictory number of frames.\n";
  }

  egg2pg_cat.info()
    << "counted " << _num_joints << " joints, " << _num_frames << " frames\n";

  _joint_data.resize(_num_joints * _num_frames);
  _slider_data.resize(_num_sliders * _num_frames);

  _slider_index = 0;
  _joint_index = 0;
}


/**
 *
 */
AnimBundleNode *AnimBundleMaker::
make_node() {
  return new AnimBundleNode(_root->get_name(), make_bundle());
}

/**
 *
 */
AnimBundle *AnimBundleMaker::
make_bundle() {
  AnimBundle *bundle = new AnimBundle(_root->get_name(), _fps, _num_frames);

  EggTable::const_iterator ci;
  for (ci = _root->begin(); ci != _root->end(); ++ci) {
    if ((*ci)->is_of_type(EggTable::get_class_type())) {
      EggTable *child = DCAST(EggTable, *ci);
      build_hierarchy(child, bundle);
    }
  }

  bundle->set_joint_channel_data(_joint_data);
  bundle->set_slider_channel_data(_slider_data);

  return bundle;
}


/**
 * Walks the egg tree, getting out the fps and the number of frames.
 */
void AnimBundleMaker::
inspect_tree(EggNode *egg_node) {
  if (egg_node->is_of_type(EggAnimData::get_class_type())) {
    // Check frame rate.
    EggAnimData *egg_anim = DCAST(EggAnimData, egg_node);
    if (egg_anim->has_fps()) {
      if (_fps == 0.0f) {
        _fps = egg_anim->get_fps();
      } else if (_fps != egg_anim->get_fps()) {
        // Whoops!  This table differs in opinion from the other tables.
        _fps = min(_fps, (PN_stdfloat)egg_anim->get_fps());
        _ok_fps = false;
      }
    }
  }

  if (egg_node->is_of_type(EggXfmSAnim::get_class_type())) {
    // Check frame rate.
    EggXfmSAnim *egg_anim = DCAST(EggXfmSAnim, egg_node);
    if (egg_anim->has_fps()) {
      if (_fps == 0.0f) {
        _fps = egg_anim->get_fps();
      } else if (_fps != egg_anim->get_fps()) {
        // Whoops!  This table differs in opinion from the other tables.
        _fps = min(_fps, (PN_stdfloat)egg_anim->get_fps());
        _ok_fps = false;
      }
    }

    _num_joints++;
  }

  if (egg_node->is_of_type(EggSAnimData::get_class_type())) {
    // Check number of frames.
    EggSAnimData *egg_anim = DCAST(EggSAnimData, egg_node);
    int num_frames = egg_anim->get_num_rows();

    if (num_frames > 1) {
      if (_num_frames == 1) {
        _num_frames = num_frames;
      } else if (_num_frames != num_frames) {
        // Whoops!  Another disagreement.
        _num_frames = min(_num_frames, num_frames);
        _ok_num_frames = false;
      }
    }

    _num_sliders++;
  }

  if (egg_node->is_of_type(EggXfmAnimData::get_class_type())) {
    // Check number of frames.
    EggXfmAnimData *egg_anim = DCAST(EggXfmAnimData, egg_node);
    int num_frames = egg_anim->get_num_rows();

    if (num_frames > 1) {
      if (_num_frames == 1) {
        _num_frames = num_frames;
      } else if (_num_frames != num_frames) {
        // Whoops!  Another disagreement.
        _num_frames = min(_num_frames, num_frames);
        _ok_num_frames = false;
      }
    }

    _num_joints++;
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    // Now recurse.
    EggGroupNode *group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::const_iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      inspect_tree(*ci);
    }
  }
}


/**
 * Walks the egg tree again, creating the AnimChannels as appropriate.
 */
void AnimBundleMaker::
build_hierarchy(EggTable *egg_table, AnimBundle *bundle) {
  // First, scan the children of egg_table for anim data tables.  If any of
  // them is named "xform", it's a special case--this one stands for the
  // egg_table node itself.  Don't ask me why.

  bool got_channel = false;

  EggTable::const_iterator ci;
  for (ci = egg_table->begin(); ci != egg_table->end(); ++ci) {
    if ((*ci)->get_name() == "xform") {
      if (!got_channel) {
        create_xfm_channel((*ci), egg_table->get_name(), bundle);
        got_channel = true;
      } else {
        egg2pg_cat.warning()
          << "Duplicate xform table under node "
          << egg_table->get_name() << "\n";
      }
    }
  }

  // Now walk the children again, creating any leftover tables, and recursing.
  for (ci = egg_table->begin(); ci != egg_table->end(); ++ci) {
    if ((*ci)->get_name() == "xform") {
      // Skip this one.  We already got it.
    } else if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *egg_anim = DCAST(EggSAnimData, *ci);
      create_s_channel(egg_anim, egg_anim->get_name(), bundle);

    } else if ((*ci)->is_of_type(EggTable::get_class_type())) {
      EggTable *child = DCAST(EggTable, *ci);
      build_hierarchy(child, bundle);
    }
  }
}


/**
 * Creates an AnimChannelScalarTable corresponding to the given EggSAnimData
 * structure.
 */
void AnimBundleMaker::
create_s_channel(EggSAnimData *egg_anim, const std::string &name,
                 AnimBundle *bundle) {
  // First we have to copy the table data from PTA_double to PTA_stdfloat.
  for (int i = 0; i < egg_anim->get_num_rows(); i++) {
    int index = AnimBundle::get_channel_data_index(_num_sliders, i, _slider_index);
    _slider_data[i] = (PN_stdfloat)egg_anim->get_value(i);
  }

  bundle->record_slider_channel_name(_slider_index, name);

  _slider_index++;
}


/**
 * Creates an AnimChannelMatrixXfmTable corresponding to the given EggNode
 * structure, if possible.
 */
void AnimBundleMaker::
create_xfm_channel(EggNode *egg_node, const std::string &name,
                   AnimBundle *bundle) {
  if (egg_node->is_of_type(EggXfmAnimData::get_class_type())) {
    EggXfmAnimData *egg_anim = DCAST(EggXfmAnimData, egg_node);
    EggXfmSAnim new_anim(*egg_anim);
    create_xfm_channel(&new_anim, name, bundle);
    return;

  } else if (egg_node->is_of_type(EggXfmSAnim::get_class_type())) {
    EggXfmSAnim *egg_anim = DCAST(EggXfmSAnim, egg_node);
    create_xfm_channel(egg_anim, name, bundle);
    return;
  }

  egg2pg_cat.warning()
    << "Inappropriate node named xform under node "
    << name << "\n";
  return;
}


/**
 * Creates an AnimChannelMatrixXfmTable corresponding to the given EggXfmSAnim
 * structure.
 */
void AnimBundleMaker::
create_xfm_channel(EggXfmSAnim *egg_anim, const std::string &name,
                   AnimBundle *bundle) {
  // Ensure that the anim table is optimal and that it is standard order.
  egg_anim->normalize();

  egg_anim->write(std::cout, 0);

  // The EggXfmSAnim structure has a number of children which are EggSAnimData
  // tables.  Each of these represents a separate component of the transform
  // data, and will be added to the table.

  EggXfmSAnim::const_iterator ci;

  pvector<LVecBase3> hpr;
  hpr.resize(_num_frames);

  for (ci = egg_anim->begin(); ci != egg_anim->end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *child = DCAST(EggSAnimData, *ci);

      char table_id = child->get_name()[0];

      for (int i = 0; i < child->get_num_rows(); i++) {
        JointFrameData &frame_data = _joint_data[AnimBundle::get_channel_data_index(_num_joints, i, _joint_index)];

        if (table_id == 'x') {
          frame_data.pos[0] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'y') {
          frame_data.pos[1] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'z') {
          frame_data.pos[2] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'i') {
          frame_data.scale[0] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'j') {
          frame_data.scale[1] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'k') {
          frame_data.scale[2] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'h') {
          hpr[i][0] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'p') {
          hpr[i][1] = (PN_stdfloat)child->get_value(i);

        } else if (table_id == 'r') {
          hpr[i][2] = (PN_stdfloat)child->get_value(i);
        }
      }
    }
  }

  // Convert each HPR frame to a quaternion.
  for (size_t i = 0; i < hpr.size(); i++) {
    JointFrameData &frame_data = _joint_data[AnimBundle::get_channel_data_index(_num_joints, i, _joint_index)];
    frame_data.quat.set_hpr(hpr[i]);
  }

  bundle->record_joint_channel_name(_joint_index, name);

  _joint_index++;
}
