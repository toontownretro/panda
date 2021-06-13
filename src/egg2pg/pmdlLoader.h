/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlLoader.h
 * @author lachbr
 * @date 2021-02-13
 */

#ifndef PMDLLOADER_H
#define PMDLLOADER_H

#include "pandabase.h"
#include "pointerTo.h"
#include "pvector.h"
#include "filename.h"
#include "vector_string.h"
#include "luse.h"
#include "pdxElement.h"

class PMDLData;
class PandaNode;

/**
 * A single material group.
 */
class PMDLMaterialGroup {
public:
  PMDLMaterialGroup() = default;
  std::string _name;
  typedef pvector<Filename> Filenames;
  Filenames _materials;
};

/**
 * Single lod switch.
 */
class PMDLLODSwitch {
public:
  float _distance = 0.0f;
  float _fade_in_time = 0.0f;
  LPoint3f _center;
  vector_string _groups;
};

/**
 *
 */
class PMDLPoseParameter {
public:
  std::string _name;
  float _min = 0.0f;
  float _max = 0.0f;
  float _loop = 0.0f;
};

/**
 *
 */
class PMDLIKChain {
public:
  PMDLIKChain() = default;
  std::string _name;
  std::string _end_joint;
  LVector3 _middle_joint_dir;
  LPoint3 _center;
  float _height = 0.0f;
  float _floor = 0.0f;
  float _pad = 0.0f;
};

/**
 *
 */
class PMDLHitBox {
public:
  PMDLHitBox() { }
  int _group = 0;
  std::string _joint_name;
  LPoint3 _min;
  LPoint3 _max;
};

/**
 *
 */
class PMDLAnim {
public:
  PMDLAnim() { }
  std::string _name;
  // Filename of animation .egg file.
  Filename _anim_filename;
  // Frames per second of animation.
  int _fps = -1;
};

/**
 * Solves a particular IK chain to keep the end joint in the same position
 * and/or rotation after applying an animation layer.
 */
class PMDLIKLock {
public:
  // The name of the IK chain to solve.
  std::string _chain_name;
  // Weights of the lock.  0 is essentially not performing the lock at all,
  // while 1 is ensuring the foot/hand is exactly where it was before the
  // animation was applied.
  float _pos_weight = 1.0f;
  float _rot_weight = 1.0f;
};

// TODO: IK rules

class PMDLSequenceLayer {
public:
  PMDLSequenceLayer() { }
  std::string _sequence_name;
  int _start_frame = -1;
  int _peak_frame = -1;
  int _tail_frame = -1;
  int _end_frame = -1;
  bool _spline = false;
  bool _no_blend = false;
};

class PMDLSequenceEvent {
public:
  PMDLSequenceEvent() { }
  int _frame = 0;
  int _event = 0;
  int _type = 0;
  std::string _options;
};

class PMDLWeightList {
public:
  PMDLWeightList() { }
  std::string _name;
  pmap<std::string, float> _weights;
};

class PMDLSequenceBlendController {
public:
  std::string _pose_parameter;
  float _min = 0.0f;
  float _max = 0.0f;
};

/**
 * How to blend multi-animation sequences.
 */
class PMDLSequenceBlend {
public:
  enum BlendType {
    BT_invalid = -1,
    BT_1d, // Not actually implemented in libanim yet.  1-dimensional linear blend space.
    BT_2d, // 2-dimensional blend space.  Each animation assigned to 2-d coordinate.
  };

  BlendType _blend_type = BT_invalid;

  // Since it's a linear list of animations, determines how to divide up the
  // animations for 2-D blends.
  int _blend_width = -1;

  std::string _blend_center_sequence;
  std::string _blend_ref_sequence;

  // Names of animations involved in the blend.  Can also just be the .egg
  // filenames.
  vector_string _animations;

  // Pose parameters controlling the blend.
  //pvector<PMDLSequenceBlendController> _blend_controllers;
  std::string _x_pose_param;
  std::string _y_pose_param;
};

/**
 *
 */
class PMDLSequence {
public:
  PMDLSequence() { }
  std::string _name;

  // Name of animation if it's a single-animation sequence.  Can also just
  // be the .egg filename if you don't want to explicitly define an animation.
  std::string _animation_name;
  // Otherwise, multi-animation sequences use this.
  PMDLSequenceBlend _blend;

  // True if the sequence is an additive animation.
  bool _delta = false;
  bool _pre_delta = false;

  bool _loop = false;

  // If true, a transition to this sequence is not blended.
  bool _snap = false;

  // Root joint translational axes to ignore.
  bool _zero_x = false;
  bool _zero_y = false;
  bool _zero_z = false;

  bool _real_time = false;

  int _fps = -1;
  int _num_frames = -1;
  float _fade_in = 0.2f;
  float _fade_out = 0.2f;
  std::string _weight_list_name;
  pvector<PMDLSequenceLayer> _layers;

  pvector<PMDLSequenceEvent> _events;

  pvector<PMDLIKLock> _ik_locks;

  int _activity;
  int _activity_weight;
};

/**
 *
 */
class PMDLAttachment {
public:
  std::string _name;
  std::string _parent_joint;
  LPoint3 _local_pos;
  LVecBase3 _local_hpr;
};

/**
 * Main data structure for a .pmdl file loaded from disk.
 */
class PMDLDataDesc : public ReferenceCount {
public:
  PMDLDataDesc() { _scale = 1.0f; }

  bool load(const Filename &filename, const DSearchPath &search_path = get_model_path());

  // Filename of the .egg model file.
  Filename _model_filename; //
  float _scale; //
  pvector<PMDLMaterialGroup> _material_groups; //
  pvector<PMDLLODSwitch> _lod_switches; //
  pvector<PMDLIKChain> _ik_chains; // TODO
  pvector<PMDLPoseParameter> _pose_parameters; //
  pvector<PMDLAnim> _anims; //
  pvector<PMDLSequence> _sequences; //
  pvector<PMDLWeightList> _weight_lists; //
  pvector<PMDLHitBox> _hit_boxes;
  pvector<PMDLAttachment> _attachments;
  vector_string _joint_merges; //

  // Custom PDX element attached to model, for game-specific data.
  PT(PDXElement) _custom_data; //

  Filename _filename;
  Filename _fullpath;
};

/**
 * This class converts a high-level model description from a .pmdl file into a
 * scene graph suitable for rendering or saving to a Bam file.
 */
class EXPCL_PANDA_EGG2PG PMDLLoader {
PUBLISHED:
  PMDLLoader(PMDLDataDesc *data);

  void build_graph();

public:
  PT(PMDLDataDesc) _data;
  PT(PandaNode) _root;

  Character *_part_bundle = nullptr;

  AnimBundle *find_or_load_anim(const std::string &anim_name);
  AnimBundle *load_anim(const std::string &name, const Filename &filename);

  pmap<std::string, AnimBundle *> _anims_by_name;
};

#include "pmdlLoader.I"

#endif // PMDLLOADER_H
