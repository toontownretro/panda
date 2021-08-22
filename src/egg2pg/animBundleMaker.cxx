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
#include "animChannelTable.h"
#include "animChannelBundle.h"
#include "vector_stdfloat.h"

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
}


/**
 *
 */
PT(AnimChannelBundle) AnimBundleMaker::
make_node() {
  PT(AnimChannelBundle) node = new AnimChannelBundle(_root->get_name());
  node->add_channel(make_bundle());
  return node;
}

/**
 *
 */
PT(AnimChannelTable) AnimBundleMaker::
make_bundle() {
  PT(AnimChannelTable) bundle = new AnimChannelTable(_root->get_name(), _fps, _num_frames);

  EggTable::const_iterator ci;
  for (ci = _root->begin(); ci != _root->end(); ++ci) {
    if ((*ci)->is_of_type(EggTable::get_class_type())) {
      EggTable *child = DCAST(EggTable, *ci);
      build_hierarchy(child, bundle);
    }
  }

  bundle->set_joint_table(std::move(_joint_table));
  bundle->set_slider_table(std::move(_slider_table));

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
build_hierarchy(EggTable *egg_table, AnimChannelTable *bundle) {
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
                 AnimChannelTable *bundle) {
  SliderEntry slider;
  slider.name = name;
  slider.first_frame = (int)_slider_table.size();
  slider.num_frames = egg_anim->get_num_rows();

  bundle->add_slider_entry(slider);

  // First we have to copy the table data from PTA_double to PTA_stdfloat.
  for (int i = 0; i < egg_anim->get_num_rows(); i++) {
    _slider_table.push_back((PN_stdfloat)egg_anim->get_value(i));
  }
}


/**
 * Creates an AnimChannelMatrixXfmTable corresponding to the given EggNode
 * structure, if possible.
 */
void AnimBundleMaker::
create_xfm_channel(EggNode *egg_node, const std::string &name,
                   AnimChannelTable *bundle) {
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
                   AnimChannelTable *bundle) {
  // Ensure that the anim table is optimal and that it is standard order.
  egg_anim->optimize_to_standard_order();

  // The EggXfmSAnim structure has a number of children which are EggSAnimData
  // tables.  Each of these represents a separate component of the transform
  // data, and will be added to the table.

  JointEntry joint;
  joint.name = name;

  joint.first_frame = (int)_joint_table.size();
  joint.num_frames = 0;

  EggXfmSAnim::const_iterator ci;

  vector_stdfloat x, y, z;
  vector_stdfloat sx, sy, sz;
  vector_stdfloat h, p, r;

  for (ci = egg_anim->begin(); ci != egg_anim->end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *child = DCAST(EggSAnimData, *ci);

      char table_id = child->get_name()[0];

      for (int i = 0; i < child->get_num_rows(); i++) {
        switch (table_id) {
        case 'x':
          x.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'y':
          y.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'z':
          z.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'i':
          sx.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'j':
          sy.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'k':
          sz.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'h':
          h.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'p':
          p.push_back((PN_stdfloat)child->get_value(i));
          break;
        case 'r':
          r.push_back((PN_stdfloat)child->get_value(i));
          break;
        default:
          break;
        }
      }
    }
  }

  // Check for any 0-length frames.

  if (h.empty()) {
    h.push_back(0.0f);
  }
  if (p.empty()) {
    p.push_back(0.0f);
  }
  if (r.empty()) {
    r.push_back(0.0f);
  }

  if (x.empty()) {
    x.push_back(0.0f);
  }
  if (y.empty()) {
    y.push_back(0.0f);
  }
  if (z.empty()) {
    z.push_back(0.0f);
  }

  if (sx.empty()) {
    sx.push_back(1.0f);
  }
  if (sy.empty()) {
    sy.push_back(1.0f);
  }
  if (sz.empty()) {
    sz.push_back(1.0f);
  }

  joint.num_frames = std::max(x.size(), std::max(y.size(), z.size()));
  joint.num_frames = std::max((size_t)joint.num_frames, std::max(h.size(), std::max(p.size(), r.size())));
  joint.num_frames = std::max((size_t)joint.num_frames, std::max(sx.size(), std::max(sy.size(), sz.size())));

  for (int i = 0; i < joint.num_frames; i++) {
    LVecBase3 pos(
      x[i % x.size()],
      y[i % y.size()],
      z[i % z.size()]
    );

    LVecBase3 scale(
      sx[i % sx.size()],
      sy[i % sy.size()],
      sz[i % sz.size()]
    );

    LVector3 hpr(
      h[i % h.size()],
      p[i % p.size()],
      r[i % r.size()]
    );

    LQuaternion quat;
    quat.set_hpr(hpr);

    JointFrame frame;
    frame.pos = std::move(pos);
    frame.quat = std::move(quat);
    frame.scale = std::move(scale);

    _joint_table.push_back(std::move(frame));
  }

  bundle->add_joint_entry(joint);
}
