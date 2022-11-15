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
#include "deg_2_rad.h"
#include "cmath.h"

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
  // Record the name of the slider.
  bundle->_slider_names.push_back(name);

  // Record if it is animated to optimize memory storage and
  // table lookup during animation computation.
  bool has_anim = egg_anim->get_num_rows() > 1;
  bundle->_slider_formats.push_back(has_anim);

  if (has_anim || egg_anim->get_value(0) != 0.0) {
    // If the slider has several frames recorded (meaning it's animated),
    // or the single held value is non-zero, mark on the table that we
    // have slider animation.  This makes us store and compute the slider
    // animation at runtime.
    bundle->_table_flags |= AnimChannelTable::TF_sliders;
  }

  for (int i = 0; i < egg_anim->get_num_rows(); i++) {
    bundle->_slider_frames[i].push_back((float)egg_anim->get_value(i));
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

  // Record name of the anim joint.
  bundle->_joint_names.push_back(name);

  EggXfmSAnim::const_iterator ci;

  vector_float x, y, z, h, p, r, i, j, k, a, b, c;

  for (ci = egg_anim->begin(); ci != egg_anim->end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *child = DCAST(EggSAnimData, *ci);

      char table_id = child->get_name()[0];

      for (int ri = 0; ri < child->get_num_rows(); ri++) {
        switch (table_id) {
        case 'x':
          x.push_back((float)child->get_value(ri));
          break;
        case 'y':
          y.push_back((float)child->get_value(ri));
          break;
        case 'z':
          z.push_back((float)child->get_value(ri));
          break;
        case 'i':
          i.push_back((float)child->get_value(ri));
          break;
        case 'j':
          j.push_back((float)child->get_value(ri));
          break;
        case 'k':
          k.push_back((float)child->get_value(ri));
          break;
        // NOTE: hpr stored in radians so we don't have to convert to radians
        // when constructing quaternion during animation eval.
        case 'h':
          h.push_back(csin(deg_2_rad((float)child->get_value(ri)) * 0.5f));
          break;
        case 'p':
          p.push_back(csin(deg_2_rad((float)child->get_value(ri)) * 0.5f));
          break;
        case 'r':
          r.push_back(csin(deg_2_rad((float)child->get_value(ri)) * 0.5f));
          break;
        case 'a':
          a.push_back((float)child->get_value(ri));
          break;
        case 'b':
          b.push_back((float)child->get_value(ri));
          break;
        case 'c':
          c.push_back((float)child->get_value(ri));
          break;
        default:
          break;
        }
      }
    }
  }

  bool has_any = !(x.empty() && y.empty() && z.empty() && h.empty() &&
                   p.empty() && r.empty() && i.empty() && j.empty() &&
                   k.empty() && a.empty() && b.empty() && c.empty());
  if (has_any) {
    bundle->_table_flags |= AnimChannelTable::TF_joints;
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
  if (h.empty()) {
    h.push_back(0.0f);
  }
  if (p.empty()) {
    p.push_back(0.0f);
  }
  if (r.empty()) {
    r.push_back(0.0f);
  }
  if (i.empty()) {
    i.push_back(1.0f);
  }
  if (j.empty()) {
    j.push_back(1.0f);
  }
  if (k.empty()) {
    k.push_back(1.0f);
  }
  if (a.empty()) {
    a.push_back(0.0f);
  }
  if (b.empty()) {
    b.push_back(0.0f);
  }
  if (c.empty()) {
    c.push_back(0.0f);
  }

  uint16_t format = 0;
  if (x.size() > 1u) {
    format |= AnimChannelTable::JF_x;
  }
  if (y.size() > 1u) {
    format |= AnimChannelTable::JF_y;
  }
  if (z.size() > 1u) {
    format |= AnimChannelTable::JF_z;
  }
  if (h.size() > 1u) {
    format |= AnimChannelTable::JF_h;
  }
  if (p.size() > 1u) {
    format |= AnimChannelTable::JF_p;
  }
  if (r.size() > 1u) {
    format |= AnimChannelTable::JF_r;
  }
  if (i.size() > 1u) {
    format |= AnimChannelTable::JF_i;
  }
  if (j.size() > 1u) {
    format |= AnimChannelTable::JF_j;
  }
  if (k.size() > 1u) {
    format |= AnimChannelTable::JF_k;
  }
  if (a.size() > 1u) {
    format |= AnimChannelTable::JF_a;
  }
  if (b.size() > 1u) {
    format |= AnimChannelTable::JF_b;
  }
  if (c.size() > 1u) {
    format |= AnimChannelTable::JF_c;
  }
  bundle->_joint_formats.push_back(format);

  for (int fi = 0; fi < x.size(); ++fi) {
    bundle->_frames[fi].push_back(x[fi]);
  }
  for (int fi = 0; fi < y.size(); ++fi) {
    bundle->_frames[fi].push_back(y[fi]);
  }
  for (int fi = 0; fi < z.size(); ++fi) {
    bundle->_frames[fi].push_back(z[fi]);
  }
  for (int fi = 0; fi < h.size(); ++fi) {
    bundle->_frames[fi].push_back(h[fi]);
  }
  for (int fi = 0; fi < p.size(); ++fi) {
    bundle->_frames[fi].push_back(p[fi]);
  }
  for (int fi = 0; fi < r.size(); ++fi) {
    bundle->_frames[fi].push_back(r[fi]);
  }
  for (int fi = 0; fi < i.size(); ++fi) {
    bundle->_frames[fi].push_back(i[fi]);
  }
  for (int fi = 0; fi < j.size(); ++fi) {
    bundle->_frames[fi].push_back(j[fi]);
  }
  for (int fi = 0; fi < k.size(); ++fi) {
    bundle->_frames[fi].push_back(k[fi]);
  }
  for (int fi = 0; fi < a.size(); ++fi) {
    bundle->_frames[fi].push_back(a[fi]);
  }
  for (int fi = 0; fi < b.size(); ++fi) {
    bundle->_frames[fi].push_back(b[fi]);
  }
  for (int fi = 0; fi < c.size(); ++fi) {
    bundle->_frames[fi].push_back(c[fi]);
  }
}
