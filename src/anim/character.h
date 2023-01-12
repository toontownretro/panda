/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.h
 * @author brian
 * @date 2021-02-22
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "characterJoint.h"
#include "characterSlider.h"
#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataLockedReader.h"
#include "cycleDataStageWriter.h"
#include "filename.h"
#include "poseParameter.h"
#include "characterAttachment.h"
#include "ikChain.h"
#include "animLayer.h"
#include "animChannel.h"
#include "animEvalContext.h"
#include "ikTarget.h"

class FactoryParams;
class Loader;
class CharacterNode;
class AnimChannelTable;
class AnimEventQueue;

/**
 * An animated character.  Defines a hierarchy of joints that influence the
 * position of vertices.  May also contain one or more sliders, which influence
 * morph targets.
 */
class EXPCL_PANDA_ANIM Character final : public TypedWritableReferenceCount, public Namable {
private:
  class CData;

  Character(const Character &copy);

PUBLISHED:
  Character(const std::string &name);

  INLINE int add_pose_parameter(const std::string &name, PN_stdfloat min_val, PN_stdfloat max_val,
                                PN_stdfloat looping = 0.0f);
  INLINE int get_num_pose_parameters() const;
  INLINE PoseParameter &get_pose_parameter(int n);
  INLINE int find_pose_parameter(const std::string &name) const;

  INLINE void set_frame_blend_flag(bool frame_blend_flag);
  INLINE bool get_frame_blend_flag() const;

  INLINE void set_root_xform(const LMatrix4 &root_xform);
  void xform(const LMatrix4 &mat);
  INLINE const LMatrix4 &get_root_xform() const;

  INLINE int get_num_nodes() const;
  INLINE CharacterNode *get_node(int n) const;
  MAKE_SEQ(get_nodes, get_num_nodes, get_node);

  MAKE_PROPERTY(frame_blend_flag, get_frame_blend_flag, set_frame_blend_flag);
  MAKE_PROPERTY(root_xform, get_root_xform, set_root_xform);
  MAKE_SEQ_PROPERTY(nodes, get_num_nodes, get_node);

  int make_joint(const std::string &name, int parent = -1,
                 const LMatrix4 &default_value = LMatrix4::ident_mat());
  int make_slider(const std::string &name, PN_stdfloat default_value = 0.0f);

  INLINE void set_joint_forced_value(int n, const LMatrix4 &value);
  INLINE void clear_joint_forced_value(int n);

  INLINE int get_num_joints() const;
  INLINE int find_joint(const std::string &name) const;
  INLINE const std::string &get_joint_name(int n) const;

  INLINE int get_num_sliders() const;
  INLINE void set_slider_value(int n, PN_stdfloat value);
  INLINE int find_slider(const std::string &name) const;
  INLINE PN_stdfloat get_slider_value(int n, Thread *current_thread = Thread::get_current_thread()) const;
  INLINE const std::string &get_slider_name(int n) const;
  INLINE void set_vertex_slider(int n, CharacterVertexSlider *slider);

  INLINE void set_joint_merge_character(Character *character);
  INLINE Character *get_joint_merge_character() const;

  INLINE void set_joint_default_value(int n, const LMatrix4 &value);
  INLINE void set_joint_merge(int n, bool merge);
  INLINE bool get_joint_merge(int n) const;
  INLINE int get_joint_parent(int n) const;
  INLINE int get_joint_num_children(int n) const;
  INLINE int get_joint_child(int joint, int child) const;
  INLINE LMatrix4 get_joint_skinning_matrix(int n, Thread *current_thread = Thread::get_current_thread()) const;
  INLINE LMatrix4 get_joint_net_transform(int n) const;
  INLINE LMatrix4 get_joint_transform(int n) const;
  INLINE LMatrix4 get_joint_initial_net_transform_inverse(int n) const;
  INLINE LMatrix4 get_joint_default_value(int n) const;
  INLINE LMatrix4 get_joint_value(int n) const;

  INLINE void set_joint_controller_node(int n, PandaNode *node);
  INLINE void clear_joint_controller_node(int n);
  INLINE PandaNode *get_joint_controller_node(int n) const;

  INLINE int add_channel(AnimChannel *channel);
  INLINE int get_num_channels() const;
  INLINE bool is_valid_channel_index(int n) const;
  INLINE AnimChannel *get_channel(int n) const;

  bool bind_anim(AnimChannelTable *anim);

  int get_channel_for_activity(int activity, int curr_channel, unsigned long seed = 0) const;

  void set_joint_vertex_transform(JointVertexTransform *transform, int joint);

  bool update(bool update_attachment_nodes = true);
  bool force_update(bool update_attachment_nodes = true);

  void recompute_joint_net_transforms();
  void recompute_joint_net_transform(int joint);

  int add_attachment(const std::string &name);
  void add_attachment_parent(int attachment, int parent, const LPoint3 &local_pos = LPoint3(0),
                             const LVecBase3 &local_hpr = LVecBase3(0), float weight = 1.0f);
  void remove_attachment_parent(int attachment, int parent);
  void set_attachment_node(int attachment, PandaNode *node);
  void clear_attachment_node(int attachment);
  PandaNode *get_attachment_node(int attachment) const;
  const TransformState *get_attachment_transform(int attachment) const;
  CPT(TransformState) get_attachment_net_transform(int attachment) const;
  int get_num_attachments() const;
  int find_attachment(const std::string &name) const;
  void compute_attachment_transform(int index, bool force_update_node);
  void remove_attachment(int attachment);
  void remove_all_attachments();

  int add_ik_chain(const std::string &name,
                   int top_joint, int middle_joint, int end_joint,
                   const LVector3 &middle_joint_dir = LVector3::forward(),
                   const LPoint3 &center = LPoint3(0),
                   PN_stdfloat height = 0.0f, PN_stdfloat floor = 0.0f,
                   PN_stdfloat pad = 0.0f);
  int add_ik_chain(const IKChain &chain);
  int add_ik_chain(IKChain &&chain);
  INLINE int get_num_ik_chains() const;
  INLINE IKChain *get_ik_chain(int n);

  int add_ik_target();
  INLINE int get_num_ik_targets() const;
  INLINE IKTarget *get_ik_target(int n);

  void advance();

  INLINE int get_num_anim_layers() const;
  INLINE bool is_valid_layer_index(int n) const;
  INLINE AnimLayer *get_anim_layer(int n);

  INLINE void set_auto_advance_flag(bool flag);
  INLINE bool get_auto_advance_flag() const;

  INLINE void set_channel_transition_flag(bool flag);
  INLINE bool get_channel_transition_flag() const;

  PT(Character) make_copy() const;
  PT(Character) copy_subgraph() const;

  void get_events(AnimEventQueue &queue, int type);

  void play(int channel, int layer = 0, PN_stdfloat play_rate = 1.0f,
            bool autokill = false, PN_stdfloat blend_in = 0.0f,
            PN_stdfloat blend_out = 0.0f);
  void play(int channel, double from, double to, int layer = 0,
            PN_stdfloat play_rate = 1.0f,
            bool autokill = false, PN_stdfloat blend_in = 0.0f,
            PN_stdfloat blend_out = 0.0f);
  void loop(int channel, bool restart, int layer = 0,
            PN_stdfloat play_rate = 1.0f, PN_stdfloat blend_in = 0.0f);
  void loop(int channel, bool restart, double from, double to, int layer = 0,
            PN_stdfloat play_rate = 1.0f, PN_stdfloat blend_in = 0.0f);
  void pingpong(int channel, bool restart, int layer = 0,
                PN_stdfloat play_rate = 1.0f, PN_stdfloat blend_in = 0.0f);
  void pingpong(int channel, bool restart, double from, double to, int layer = 0,
                PN_stdfloat play_rate = 1.0f, PN_stdfloat blend_in = 0.0f);
  void pose(int channel, double frame, int layer = 0, PN_stdfloat blend_in = 0.0f,
            PN_stdfloat blend_out = 0.0f);
  //void start(int channel, bool restart, int layer = 0,
  //           PN_stdfloat play_rate = 1.0f, PN_stdfloat blend_in = 0.0f,
  //           PN_stdfloat blend_out = 0.0f);
  //void start(int channel, bool restart, double from, double to, int layer = 0,
  //           PN_stdfloat play_rate = 1.0f, PN_stdfloat blend_in = 0.0f,
  //           PN_stdfloat blend_out = 0.0f);
  void stop(int layer = -1, bool kill = false);

  void reset_layer_channel(int layer, int channel, int activity, bool restart,
                           PN_stdfloat start_time, PN_stdfloat from, PN_stdfloat to,
                           AnimLayer::PlayMode mode, PN_stdfloat play_rate,
                           bool autokill, PN_stdfloat blend_in, PN_stdfloat blend_out);
  void ensure_layer_count(int count);


public:
  void add_node(CharacterNode *node);
  void remove_node(CharacterNode *node);

  INLINE void set_update_delay(double delay);

private:
  void build_joint_merge_map(Character *merge_char);

  void update_active_owner(CharacterNode *old_owner, CharacterNode *new_owner);

  bool apply_pose(CData *cdata, const LMatrix4 &root_xform,
                  const AnimEvalData &data, Thread *current_thread,
                  bool update_attachment_nodes);

  bool do_update(double now, CData *cdata, Thread *current_thread, bool update_attachment_nodes);
  void do_advance(double now, CData *cdata, Thread *current_thread);

private:
  typedef pvector<LMatrix4> Matrices;
  typedef pvector<JointVertexTransform *> VertexTransforms;
  typedef pvector<CharacterJoint> Joints;
  typedef pvector<CharacterJointPoseData> JointPoses;
  typedef pvector<PoseParameter> PoseParameters;
  typedef pvector<CharacterAttachment> Attachments;
  typedef pvector<IKChain> IKChains;
  typedef pvector<AnimLayer> AnimLayers;
  typedef pvector<PT(AnimChannel)> AnimChannels;
  typedef pvector<IKTarget> IKTargets;

  class ChannelBinding {
  public:
    vector_int _joint_map;
    vector_int _slider_map;
  };
  typedef pflat_hash_map<AnimChannel *, ChannelBinding, pointer_hash> ChannelBindings;

  AnimChannels _channels;
  // This maps AnimChannelTable pointers to a mapping of character joint
  // indices to animation joint indices.  This is needed because animations can
  // be shared by different characters with different joint hierarchies.
  // When computing the animation for this character, AnimChannelTables will
  // look up their pointer in this map to get the joint mapping.
  ChannelBindings _channel_bindings;

  AnimLayers _anim_layers;

  IKChains _ik_chains;
  IKTargets _ik_targets;

  PoseParameters _pose_parameters;

  Attachments _attachments;

  // These are filled in as the joint animates.
  Joints _joints;
  JointPoses _joint_poses;

  typedef pvector<CharacterSlider> Sliders;
  Sliders _sliders;

  double _update_delay;

  // The active owner of this Character.  All expose joint nodes are parented
  // to this CharacterNode.
  CharacterNode *_active_owner;

  typedef pvector<CharacterNode *> Nodes;
  Nodes _nodes;

  // Contains all of the bind poses for the character's joints.
  // This gets copied before the anim evaluation for the character so
  // we don't have to rebuild this structure every time.
  AnimEvalData _bind_pose;
  bool _built_bind_pose;

  // This is the data that must be cycled between pipeline stages.
  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *writer, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return Character::get_class_type();
    }

    bool _frame_blend_flag;
    bool _auto_advance_flag;
    bool _channel_transition_flag;
    LMatrix4 _root_xform;
    PT(Character) _joint_merge_character;
    bool _anim_changed;
    double _last_update;
  };

  // Contains data that is used to render the results of computed animation.
  // Contains the skinning matrix of each joint and slider values.
  // This data changes often and needs to be properly cycled for the Draw thread,
  // which uploads these matrices to the GPU to perform GPU vertex animation.
  class RenderCData : public CycleData {
  public:
    RenderCData() = default;
    RenderCData(const RenderCData &copy) = default;
    virtual CycleData *make_copy() const override;

    Matrices _joint_skinning_matrices;
    vector_float _slider_values;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  PipelineCycler<RenderCData> _render_cycler;
  typedef CycleDataReader<RenderCData> RenderCDReader;
  typedef CycleDataWriter<RenderCData> RenderCDWriter;

  // This is only used during Bam reading.
  pvector<ChannelBinding> _read_bindings;

public:
  static void register_with_read_factory();
  virtual void finalize(BamReader *manager);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "Character",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class CharacterNode;
  friend class AnimChannelTable;
  friend class IKHelper;
  friend class PMDLLoader;
};

#include "character.I"

#endif // CHARACTER_H
