/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundleMaker.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef ANIMBUNDLEMAKER_H
#define ANIMBUNDLEMAKER_H

#include "pandabase.h"
#include "animChannelTable.h"
#include "vector_stdfloat.h"
#include "numeric_types.h"

class EggNode;
class EggGroupNode;
class EggTable;
class EggXfmSAnim;
class EggSAnimData;
class AnimGroup;
class AnimChannelBundle;

/**
 * Converts an EggTable hierarchy, beginning with a <Bundle> entry, into an
 * AnimBundle hierarchy.
 */
class EXPCL_PANDA_EGG2PG AnimBundleMaker {
public:
  explicit AnimBundleMaker(EggTable *root);

  PT(AnimChannelBundle) make_node();

private:
  PT(AnimChannelTable) make_bundle();

  void inspect_tree(EggNode *node);
  void build_hierarchy(EggTable *egg_table, AnimChannelTable *bundle);

  void
  create_s_channel(EggSAnimData *egg_anim, const std::string &name,
                   AnimChannelTable *bundle);
  void
  create_xfm_channel(EggNode *egg_node, const std::string &name,
                     AnimChannelTable *bundle);
  void
  create_xfm_channel(EggXfmSAnim *egg_anim, const std::string &name,
                     AnimChannelTable *bundle);

  PN_stdfloat _fps;
  int _num_frames;
  bool _ok_fps;
  bool _ok_num_frames;
  int _num_joints;
  int _num_sliders;

  EggTable *_root;

};

#endif
