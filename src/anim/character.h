/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.h
 * @author lachbr
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
#include "animControl.h"
#include "filename.h"
#include "partSubset.h"
#include "animPreloadTable.h"
#include "animGraphNode.h"
#include "poseParameter.h"

class FactoryParams;
class AnimBundle;
class Loader;
class CharacterNode;

/**
 * An animated character.  Defines a hierarchy of joints that influence the
 * position of vertices.  May also contain one or more sliders, which influence
 * morph targets.
 */
class EXPCL_PANDA_ANIM Character final : public TypedWritableReferenceCount, public Namable {
private:
  Character(const Character &copy);

PUBLISHED:
  // This enum defines bits which may be passed into check_hierarchy() and
  // PartBundle::bind_anim() to allow an inexact match of channel hierarchies.
  // This specifies conditions that we don't care about enforcing.
  enum HierarchyMatchFlags {
    HMF_ok_part_extra          = 0x01,
    HMF_ok_anim_extra          = 0x02,
    HMF_ok_wrong_root_name     = 0x04,
  };

  Character(const std::string &name);

  INLINE CPT(AnimPreloadTable) get_anim_preload() const;
  INLINE PT(AnimPreloadTable) modify_anim_preload();
  INLINE void set_anim_preload(AnimPreloadTable *table);
  INLINE void clear_anim_preload();
  void merge_anim_preloads(const Character *other);

  INLINE void add_pose_parameter(PoseParameter *param);
  INLINE PoseParameter *get_pose_parameter(const std::string &name) const;

  INLINE void set_anim_graph(AnimGraphNode *graph);
  INLINE AnimGraphNode *get_anim_graph() const;

  INLINE void set_frame_blend_flag(bool frame_blend_flag);
  INLINE bool get_frame_blend_flag() const;

  INLINE void set_root_xform(const LMatrix4 &root_xform);
  //INLINE void xform(const LMatrix4 &mat);
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
  INLINE PN_stdfloat get_slider_value(int n) const;
  INLINE const std::string &get_slider_name(int n) const;
  INLINE void set_vertex_slider(int n, CharacterVertexSlider *slider);

  INLINE void set_joint_default_value(int n, const LMatrix4 &value);
  INLINE int get_joint_parent(int n) const;
  INLINE int get_joint_num_children(int n) const;
  INLINE int get_joint_child(int joint, int child) const;
  INLINE const LMatrix4 &get_joint_skinning_matrix(int n) const;
  INLINE const LMatrix4 &get_joint_net_transform(int n) const;
  INLINE const LMatrix4 &get_joint_transform(int n) const;
  INLINE const LMatrix4 &get_joint_initial_net_transform_inverse(int n) const;
  INLINE const LMatrix4 &get_joint_default_value(int n) const;

  void set_joint_vertex_transform(JointVertexTransform *transform, int joint);

  PT(AnimControl) bind_anim(AnimBundle *anim,
                            int hierarchy_match_flags = 0,
                            const PartSubset &subset = PartSubset());
  PT(AnimControl) load_bind_anim(Loader *loader,
                                 const Filename &filename,
                                 int hierarchy_match_flags,
                                 const PartSubset &subset,
                                 bool allow_async);

  bool update();
  bool force_update();

  void recompute_joint_net_transforms();
  void recompute_joint_net_transform(int joint);

  bool add_net_transform(int joint, PandaNode *node);
  bool remove_net_transform(int joint, PandaNode *node);
  bool has_net_transform(int joint, PandaNode *node) const;
  void clear_net_transforms(int joint);
  NodePathCollection get_net_transforms(int joint);

  PT(Character) make_copy() const;
  PT(Character) copy_subgraph() const;

public:
  void add_node(CharacterNode *node);
  void remove_node(CharacterNode *node);

  bool do_bind_anim(AnimControl *control, AnimBundle *anim,
                    int hierarchy_match_flags, const PartSubset &subset);

  INLINE void set_update_delay(double delay);

private:
  void update_active_owner(CharacterNode *old_owner, CharacterNode *new_owner);

  bool check_hierarchy(const AnimBundle *anim, int hierarchy_match_flags) const;
  void find_bound_joints(int n, bool is_included, BitArray &bound_joints, const PartSubset &subset);
  void pick_channel_index(int n, plist<int> &holes, int &next) const;
  void bind_hierarchy(const AnimBundle *anim, int n, int channel_index,
                      bool is_included, BitArray &bound_joints, const PartSubset &subset);

  bool apply_pose(const LMatrix4 &root_xform, const AnimGraphEvalContext &context, Thread *current_thread);

private:
  typedef pvector<LMatrix4> Matrices;
  typedef pvector<JointVertexTransform *> VertexTransforms;
  typedef pvector<CharacterJoint> Joints;
  typedef ov_set<PT(PandaNode)> NodeList;
  typedef pvector<NodeList> NodeLists;
  typedef pmap<std::string, PT(PoseParameter)> PoseParameters;

  PoseParameters _pose_parameters;

  // These are filled in as the joint animates.
  Matrices _joint_values;
  Matrices _joint_net_transforms;
  // This is the product of the above; the matrix that gets applied to a
  // vertex (whose coordinates are in the coordinate space of the character
  // in its neutral pose) to transform it from its neutral position to its
  // animated position.
  Matrices _joint_skinning_matrices;
  Matrices _joint_initial_net_transform_inverse;
  //Matrices _joint_default_values;
  VertexTransforms _joint_vertex_transforms;
  NodeLists _joint_net_transform_nodes;
  Joints _joints;

  typedef pvector<CharacterSlider> Sliders;
  Sliders _sliders;

  double _update_delay;

  COWPT(AnimPreloadTable) _anim_preload;

  // The active owner of this Character.  All expose joint nodes are parented
  // to this CharacterNode.
  CharacterNode *_active_owner;

  typedef pvector<CharacterNode *> Nodes;
  Nodes _nodes;

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
    LMatrix4 _root_xform;
    PT(AnimGraphNode) _anim_graph;
    bool _anim_changed;
    double _last_update;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageWriter<CData> CDStageWriter;

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
};

#include "character.I"

#endif // CHARACTER_H
