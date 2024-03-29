/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlLoader.h
 * @author brian
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
#include "assetBase.h"
#include "animChannel.h"

class PandaNode;
class Character;
class AnimChannelTable;
class AnimChannel;

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
  std::string _name = "";
  // Filename of animation .egg file.
  Filename _anim_filename = "";
  // Frames per second of animation.
  int _fps = 30;
};

class PMDLIKEvent {
public:
  enum Type {
    T_invalid = -1,
    T_lock, // Lock IK chain to pose before animation channel was applied.
    T_touch, // Lock IK chain to an offset from another joint before animation channel was applied.
    T_release,
  };

  Type _type = T_invalid;

  std::string _chain_name;

  std::string _touch_joint;
  // Touch offsets are determined by overlaying the animation with the IK event
  // on top of this source animation.
  std::string _touch_source_anim;

  // How to blend in IK.
  float _start_frame = 0;
  float _peak_frame = 0;
  float _tail_frame = 0;
  float _end_frame = 0;
  bool _spline = false;
  // If non-empty, pose parameter controls blending and not cycle of channel.
  std::string _pose_param = "";
};

class PMDLSequenceLayer {
public:
  PMDLSequenceLayer() { }
  std::string _sequence_name = "";
  float _start_frame = 0;
  float _peak_frame = 0;
  float _tail_frame = 0;
  float _end_frame = 0;
  bool _spline = false;
  bool _no_blend = false;
  bool _xfade = false;
  std::string _pose_param = "";
};

class PMDLSequenceEvent {
public:
  PMDLSequenceEvent() { }
  int _frame = 0;
  std::string _event = "";
  int _type = 0;
  std::string _options = "";
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
    BT_1d, // 1-dimensional linear blend space.
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

  bool _motion_x = false;
  bool _motion_y = false;
  bool _motion_z = false;

  bool _real_time = false;

  // If not -1, overrides the frame rate of all animations in sequence.
  int _fps = -1;

  int _num_frames = -1;

  float _fade_in = 0.2f;
  float _fade_out = 0.2f;
  std::string _weight_list_name;
  pvector<PMDLSequenceLayer> _layers;

  pvector<PMDLSequenceEvent> _events;

  pvector<PMDLIKEvent> _ik_events;

  std::string _activity = "";
  int _activity_weight = 1;
};

class PMDLAttachmentInfluence {
public:
  PMDLAttachmentInfluence() {
    _local_pos.set(0, 0, 0);
    _local_hpr.set(0, 0, 0);
    _weight = 1.0f;
    _parent_joint = "";
  }

  std::string _parent_joint;
  LPoint3 _local_pos;
  LVecBase3 _local_hpr;
  float _weight = 1.0f;
};

/**
 *
 */
class PMDLAttachment {
public:
  std::string _name;
  pvector<PMDLAttachmentInfluence> _influences;
};

/**
 *
 */
class PMDLEyeball {
public:
  PMDLEyeball() {
    _name = "";
    _material_name = "";
    _parent = "";
    _pos = LPoint3(0);
    _eye_shift = LVecBase3(0);
    _diameter = 1.0f;
    _iris_size = 1.0f;
    _eye_size = 0.0f;
    _z_offset = 0.0f;
  }
  std::string _name;
  std::string _material_name;
  std::string _parent;
  LPoint3 _pos;
  LVecBase3 _eye_shift;
  float _diameter;
  float _iris_size;
  float _eye_size;
  float _z_offset;
};

/**
 *
 */
class PMDLPhysicsJoint {
public:
  std::string _joint_name = "";
  // Non-zero if explicitly specified, otherwise uses the
  // `_use_exact_geometry` property of the collision model, which
  // indicates if the geometry is a convex hull or triangle mesh.
  int _concave = -1;
  vector_string _collide_with;
  float _mass_bias = 1;
  float _rot_damping = -1;
  float _damping = -1;
  float _inertia = -1;
  LVecBase2 _limit_x;
  LVecBase2 _limit_y;
  LVecBase2 _limit_z;
};

/**
 *
 */
class PMDLPhysicsModel {
public:
  std::string _name = "";
  // Name of Egg group that contains the physics geometry.
  std::string _mesh_name = "";
  // Create triangle mesh instead of convex mesh.
  bool _use_exact_geometry = false;
  // Automatically calculate mass from volume of mesh.  Only for convex meshes.
  bool _auto_mass = true;
  // Explicit mass if auto mass is false.
  float _mass_override = 100.0f;
  float _rot_damping = 0.0f;
  float _damping = 0.0f;
  float _density = 900.0f;
  float _thickness = 0.0f;
  float _inertia = 1.0f;

  pvector<PMDLPhysicsJoint> _joints;
};

/**
 * Main data structure for a .pmdl file loaded from disk.
 */
class EXPCL_PANDA_EGG2PG PMDLDataDesc : public AssetBase {
public:
  PMDLDataDesc() { _scale.set(1, 1, 1); _pos.set(0, 0, 0); _hpr.set(0, 0, 0); }

  // AssetBase interface.
  virtual bool load(const Filename &filename, const DSearchPath &search_path = get_model_path()) override;
  virtual std::string get_name() override;
  virtual std::string get_source_extension() override;
  virtual std::string get_built_extension() override;
  virtual void get_dependencies(vector_string &filenames) override;
  virtual PT(AssetBase) make_new() const override;

  // Filename of the .egg model file.
  Filename _model_filename; //
  LVecBase3 _scale; //
  LVecBase3 _hpr; //
  LPoint3 _pos; //
  pvector<PMDLMaterialGroup> _material_groups; //
  pvector<PMDLLODSwitch> _lod_switches; //
  pvector<PMDLIKChain> _ik_chains; // TODO
  pvector<PMDLPoseParameter> _pose_parameters; //
  pvector<PMDLAnim> _anims; //
  pvector<PMDLSequence> _sequences; //
  pvector<PMDLWeightList> _weight_lists; //
  pvector<PMDLHitBox> _hit_boxes;
  pvector<PMDLAttachment> _attachments;
  pvector<PMDLEyeball> _eyeballs;
  vector_string _joint_merges; //
  pvector<Filename> _material_paths;

  PMDLPhysicsModel _phy;

  // Custom PDX element attached to model, for game-specific data.
  PT(PDXElement) _custom_data; //

  Filename _filename;
  Filename _fullpath;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AssetBase::init_type();
    register_type(_type_handle, "PMDLDataDesc",
                  AssetBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
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

  DSearchPath _search_path;

  PT(AnimChannel) make_blend_channel(const PMDLSequenceBlend &blend, int fps);
  PT(AnimChannel) make_layered_channel(const PMDLSequence *seq);

  AnimChannel *find_or_load_anim(const std::string &anim_name);
  AnimChannelTable *load_anim(const std::string &name, const Filename &filename);

  void calc_ik_touch_offsets(AnimChannel *chan, AnimChannel::IKEvent &ik_event, const std::string &source_anim);

  pmap<std::string, AnimChannelTable *> _anims_by_name;
  pmap<std::string, AnimChannel *> _chans_by_name;
};

#include "pmdlLoader.I"

#endif // PMDLLOADER_H
