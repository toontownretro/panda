/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundle.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef PARTBUNDLE_H
#define PARTBUNDLE_H

#include "pandabase.h"

#include "partGroup.h"
#include "animControl.h"
#include "partSubset.h"
#include "animPreloadTable.h"
#include "pointerTo.h"
#include "thread.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "luse.h"
#include "pvector.h"
#include "transformState.h"
#include "weakPointerTo.h"
#include "copyOnWritePointer.h"
#include "animGraphNode.h"

class Loader;
class AnimBundle;
class PartBundleNode;
class PartBundleNode;
class TransformState;
class AnimPreloadTable;

/**
 * This is the root of a MovingPart hierarchy.  It defines the hierarchy of
 * moving parts that make up an animatable object.
 */
class EXPCL_PANDA_CHAN PartBundle : public PartGroup {
public:
  typedef pvector<PT(AnimControl)> ActiveControls;

protected:
  // The copy constructor is protected; use make_copy() or copy_subgraph().
  PartBundle(const PartBundle &copy);

PUBLISHED:
  explicit PartBundle(const std::string &name = "");
  virtual PartGroup *make_copy() const;

  INLINE CPT(AnimPreloadTable) get_anim_preload() const;
  INLINE PT(AnimPreloadTable) modify_anim_preload();
  INLINE void set_anim_preload(AnimPreloadTable *table);
  INLINE void clear_anim_preload();
  void merge_anim_preloads(const PartBundle *other);

  INLINE void set_anim_graph(AnimGraphNode *graph);
  INLINE AnimGraphNode *get_anim_graph() const;

  INLINE void set_frame_blend_flag(bool frame_blend_flag);
  INLINE bool get_frame_blend_flag() const;

  INLINE void set_root_xform(const LMatrix4 &root_xform);
  INLINE void xform(const LMatrix4 &mat);
  INLINE const LMatrix4 &get_root_xform() const;
  PT(PartBundle) apply_transform(const TransformState *transform);

  INLINE int get_num_nodes() const;
  INLINE PartBundleNode *get_node(int n) const;
  MAKE_SEQ(get_nodes, get_num_nodes, get_node);

  MAKE_PROPERTY(frame_blend_flag, get_frame_blend_flag, set_frame_blend_flag);
  MAKE_PROPERTY(root_xform, get_root_xform, set_root_xform);
  MAKE_SEQ_PROPERTY(nodes, get_num_nodes, get_node);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  PT(AnimControl) bind_anim(AnimBundle *anim,
                            int hierarchy_match_flags = 0,
                            const PartSubset &subset = PartSubset());
  PT(AnimControl) load_bind_anim(Loader *loader,
                                 const Filename &filename,
                                 int hierarchy_match_flags,
                                 const PartSubset &subset,
                                 bool allow_async);
  void wait_pending();

  bool freeze_joint(const std::string &joint_name, const TransformState *transform);
  bool freeze_joint(const std::string &joint_name, const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale);
  bool freeze_joint(const std::string &joint_name, PN_stdfloat value);
  bool control_joint(const std::string &joint_name, PandaNode *node);
  bool release_joint(const std::string &joint_name);

  bool update();
  bool force_update();

public:
  // The following functions aren't really part of the public interface;
  // they're just public so we don't have to declare a bunch of friends.
  virtual void control_activated(AnimControl *control);
  virtual void control_deactivated(AnimControl *control);

  INLINE void set_update_delay(double delay);

  INLINE void mark_anim_changed();

  bool do_bind_anim(AnimControl *control, AnimBundle *anim,
                    int hierarchy_match_flags, const PartSubset &subset);

protected:
  virtual void add_node(PartBundleNode *node);
  virtual void remove_node(PartBundleNode *node);

private:
  class CData;

  COWPT(AnimPreloadTable) _anim_preload;

  typedef pvector<PartBundleNode *> Nodes;
  Nodes _nodes;

  typedef pmap<WCPT(TransformState), WPT(PartBundle), std::owner_less<WCPT(TransformState)> > AppliedTransforms;
  AppliedTransforms _applied_transforms;

  double _update_delay;

  // This is the data that must be cycled between pipeline stages.
  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return PartBundle::get_class_type();
    }

    bool _frame_blend_flag;
    LMatrix4 _root_xform;
    PT(AnimGraphNode) _anim_graph;
    ActiveControls _active_controls;
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
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PartGroup::init_type();
    register_type(_type_handle, "PartBundle",
                  PartGroup::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class PartBundleNode;
  friend class Character;
  friend class MovingPartBase;
  friend class MovingPartMatrix;
  friend class MovingPartScalar;
};

inline std::ostream &operator <<(std::ostream &out, const PartBundle &bundle) {
  bundle.output(out);
  return out;
}

#include "partBundle.I"

#endif
