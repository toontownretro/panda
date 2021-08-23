/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelTable.h
 * @author brian
 * @date 2021-08-04
 */

#ifndef ANIMCHANNELTABLE_H
#define ANIMCHANNELTABLE_H

#include "pandabase.h"
#include "animChannel.h"
#include "luse.h"
#include "pointerToArray.h"
#include "pta_stdfloat.h"
#include "vector_int.h"

class FactoryParams;

/**
 * There is one instance of this class for each joint in an animation.  It
 * specifies the indices for a joint into the animation tables for each
 * component.  It also specifies the number of sequential frames for a joint
 * for each component, because egg files optimize out components that remain
 * constant.
 */
class EXPCL_PANDA_ANIM JointEntry : public TypedObject {
  DECLARE_CLASS(JointEntry, TypedObject);
PUBLISHED:
  std::string name;

  int first_frame, num_frames;

  INLINE bool operator == (const JointEntry &other) const {
    return name == other.name &&
           first_frame == other.first_frame &&
           num_frames == other.num_frames;
  }
};

class EXPCL_PANDA_ANIM ALIGN_16BYTE JointFrame : public TypedObject {
  DECLARE_CLASS(JointFrame, TypedObject);
PUBLISHED:
  LQuaternion quat;
  LVecBase3 pos;
  LVecBase3 scale;

  JointFrame() = default;

  JointFrame(const JointFrame &copy) :
    quat(copy.quat),
    pos(copy.pos),
    scale(copy.scale)
  {
  }

  JointFrame(JointFrame &&other) :
    quat(std::move(other.quat)),
    pos(std::move(other.pos)),
    scale(std::move(other.scale))
  {
  }

  void operator = (JointFrame &&other) {
    quat = std::move(other.quat);
    pos = std::move(other.pos);
    scale = std::move(other.scale);
  }

  void operator = (const JointFrame &other) {
    quat = other.quat;
    pos = other.pos;
    scale = other.scale;
  }

  bool operator == (const JointFrame &other) const {
    return quat == other.quat &&
           pos == other.pos &&
           scale == other.scale;
  }
};

/**
 * There is one instance of this class for each slider in an animation.  It
 * specifies the index for a slider into the animation table.
 */
class EXPCL_PANDA_ANIM SliderEntry : public TypedObject {
  DECLARE_CLASS(SliderEntry, TypedObject);
PUBLISHED:
  std::string name;

  int first_frame, num_frames;

  bool operator == (const SliderEntry &other) const {
    return name == other.name &&
           first_frame == other.first_frame &&
           num_frames == other.num_frames;
  }
};

BEGIN_PUBLISH

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToBase<ReferenceCountedVector<JointEntry> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArrayBase<JointEntry>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArray<JointEntry>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, ConstPointerToArray<JointEntry>)

typedef PointerToArray<JointEntry> JointEntries;
typedef ConstPointerToArray<JointEntry> CJointEntries;

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToBase<ReferenceCountedVector<JointFrame> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArrayBase<JointFrame>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArray<JointFrame>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, ConstPointerToArray<JointFrame>)

typedef PointerToArray<JointFrame> JointFrames;
typedef ConstPointerToArray<JointFrame> CJointFrames;

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToBase<ReferenceCountedVector<SliderEntry> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArrayBase<SliderEntry>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArray<SliderEntry>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, ConstPointerToArray<SliderEntry>)

typedef PointerToArray<SliderEntry> SliderEntries;
typedef ConstPointerToArray<SliderEntry> CSliderEntries;

END_PUBLISH

/**
 * This is an AnimChannel that gets its pose by sampling a static joint
 * animation table.  This corresponds directly to an animation that was
 * created in a modelling package and loaded from an Egg file.
 */
class EXPCL_PANDA_ANIM AnimChannelTable final : public AnimChannel {
  DECLARE_CLASS(AnimChannelTable, AnimChannel);

PUBLISHED:
  INLINE explicit AnimChannelTable(const std::string &name, PN_stdfloat fps, int num_frames);

  INLINE void set_joint_table(JointFrames table);
  INLINE JointFrames get_joint_table() const;

  INLINE const JointFrame &get_joint_frame(int joint, int frame) const;
  INLINE const JointFrame &get_joint_frame(const JointEntry &joint, int frame) const;

  INLINE void set_slider_table(PTA_stdfloat table);
  INLINE PTA_stdfloat get_slider_table() const;

  INLINE void get_scalar(int slider, int frame, PN_stdfloat &scalar) const;
  INLINE void get_scalar(const SliderEntry &slider, int frame, PN_stdfloat &scalar) const;

  int find_joint_channel(const std::string &name) const;
  int find_slider_channel(const std::string &name) const;

  INLINE void add_joint_entry(const JointEntry &joint);
  INLINE const JointEntry &get_joint_entry(int n) const;

  INLINE void add_slider_entry(const SliderEntry &slider);
  INLINE const SliderEntry &get_slider_entry(int n) const;

  INLINE int get_num_joint_entries() const;
  INLINE int get_num_slider_entries() const;

  INLINE void init_joint_mapping(int num_character_joints, int num_character_sliders);
  INLINE void map_character_joint_to_anim_joint(int character_joint, int anim_joint);
  INLINE void map_character_slider_to_anim_slider(int character_slider, int anim_slider);
  INLINE int get_anim_joint_for_character_joint(int character_joint) const;
  INLINE int get_anim_slider_for_character_slider(int character_slider) const;
  INLINE bool has_mapped_character() const;

  virtual PT(AnimChannel) make_copy() const override;
  virtual PN_stdfloat get_length(Character *character) const override;
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data) override;

public:
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;

protected:
  INLINE AnimChannelTable();
  INLINE AnimChannelTable(const AnimChannelTable &copy);

  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  JointEntries _joint_entries;
  JointFrames _joint_frames;

  SliderEntries _slider_entries;
  PTA_stdfloat _slider_table;

  // Maps joints on the corresponding character to joints on the animation.
  // This is needed because Egg files do not guarantee matching joint orders
  // between characters and their animations.  I don't expect an animation to
  // be used for multiple characters with different joint hierarchies, so a
  // single mapping should be fine.
  vector_int _joint_map;
  vector_int _slider_map;

  bool _has_character_bound;
};

#include "animChannelTable.I"

#endif // ANIMCHANNELTABLE_H
