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

class FactoryParams;
class AnimBundle;
class Loader;

/**
 * An animated character.  Defines a hierarchy of joints that influence the
 * position of vertices.  May also contain one or more sliders, which influence
 * morph targets.
 */
class EXPCL_PANDA_ANIM Character : public TypedWritableReferenceCount, public Namable {
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

  INLINE void set_frame_blend_flag(bool frame_blend_flag);
  INLINE bool get_frame_blend_flag() const;

  INLINE void set_root_xform(const LMatrix4 &root_xform);
  //INLINE void xform(const LMatrix4 &mat);
  INLINE const LMatrix4 &get_root_xform() const;

  MAKE_PROPERTY(frame_blend_flag, get_frame_blend_flag, set_frame_blend_flag);
  MAKE_PROPERTY(root_xform, get_root_xform, set_root_xform);

  CharacterJoint *make_joint(const std::string &name, int parent = -1);
  CharacterSlider *make_slider(const std::string &name);

  INLINE int get_num_joints() const;
  INLINE CharacterJoint *get_joint(int n);

  INLINE int get_num_sliders() const;
  INLINE CharacterSlider *get_slider(int n);

  PT(AnimControl) bind_anim(AnimBundle *anim,
                            int hierarchy_match_flags = 0,
                            const PartSubset &subset = PartSubset());
  PT(AnimControl) load_bind_anim(Loader *loader,
                                 const Filename &filename,
                                 int hierarchy_match_flags,
                                 const PartSubset &subset,
                                 bool allow_async);

  bool update();

public:
  bool do_bind_anim(AnimControl *control, AnimBundle *anim,
                    int hierarchy_match_flags, const PartSubset &subset);

private:
  bool check_hierarchy(const AnimBundle *anim, int hierarchy_match_flags) const;

private:
  typedef pvector<CharacterJoint> Joints;
  Joints _joints;

  typedef pvector<CharacterSlider> Sliders;
  Sliders _sliders;

  double _update_delay;

  COWPT(AnimPreloadTable) _anim_preload;

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
  virtual void write_datagram(BamWriter* manager, Datagram &me);

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
};

#include "character.I"

#endif // CHARACTER_H
