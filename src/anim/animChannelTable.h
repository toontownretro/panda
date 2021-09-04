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
#include "vector_stdfloat.h"
#include "vector_int.h"

class FactoryParams;

/**
 * There is one instance of this class for each joint in an animation.  It
 * specifies the indices for a joint into the animation tables for each
 * component.  It also specifies the number of sequential frames for a joint
 * for each component, because egg files optimize out components that remain
 * constant.
 */
class EXPCL_PANDA_ANIM JointEntry final {
PUBLISHED:
  std::string name;

  int first_frame, num_frames;

  INLINE bool operator == (const JointEntry &other) const {
    return name == other.name &&
           first_frame == other.first_frame &&
           num_frames == other.num_frames;
  }
};

class EXPCL_PANDA_ANIM JointFrame final {
PUBLISHED:
  LVecBase3 pos;
  LVecBase3 scale;
  LVecBase3 shear;
  LQuaternion quat;

  JointFrame() = default;

  JointFrame(const JointFrame &copy) :
    quat(copy.quat),
    pos(copy.pos),
    scale(copy.scale),
    shear(copy.shear)
  {
  }

  JointFrame(JointFrame &&other) :
    quat(std::move(other.quat)),
    pos(std::move(other.pos)),
    scale(std::move(other.scale)),
    shear(std::move(other.shear))
  {
  }

  void operator = (JointFrame &&other) {
    quat = std::move(other.quat);
    pos = std::move(other.pos);
    scale = std::move(other.scale);
    shear = std::move(other.shear);
  }

  void operator = (const JointFrame &other) {
    quat = other.quat;
    pos = other.pos;
    scale = other.scale;
    shear = other.shear;
  }

  bool operator == (const JointFrame &other) const {
    return quat == other.quat &&
           pos == other.pos &&
           scale == other.scale &&
           shear == other.shear;
  }
};
typedef pvector<JointFrame> JointFrames;

/**
 * There is one instance of this class for each slider in an animation.  It
 * specifies the index for a slider into the animation table.
 */
class EXPCL_PANDA_ANIM SliderEntry final {
PUBLISHED:
  std::string name;

  int first_frame, num_frames;

  bool operator == (const SliderEntry &other) const {
    return name == other.name &&
           first_frame == other.first_frame &&
           num_frames == other.num_frames;
  }
};

/**
 * This is an AnimChannel that gets its pose by sampling a static joint
 * animation table.  This corresponds directly to an animation that was
 * created in a modelling package and loaded from an Egg file.
 */
class EXPCL_PANDA_ANIM AnimChannelTable final : public AnimChannel {
  DECLARE_CLASS(AnimChannelTable, AnimChannel);

PUBLISHED:
  INLINE explicit AnimChannelTable(const std::string &name, PN_stdfloat fps, int num_frames);

  INLINE void set_joint_table(JointFrames &&table);
  INLINE const JointFrames &get_joint_table() const;

  INLINE const JointFrame &get_joint_frame(int joint, int frame) const;
  INLINE const JointFrame &get_joint_frame(const JointEntry &joint, int frame) const;

  INLINE void set_slider_table(vector_stdfloat &&table);
  INLINE const vector_stdfloat &get_slider_table() const;

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
  JointEntry _joint_entries[max_character_joints];
  size_t _num_joint_entries;
  JointFrames _joint_frames;

  SliderEntry _slider_entries[max_character_joints];
  size_t _num_slider_entries;
  vector_stdfloat _slider_table;
};

#include "animChannelTable.I"

#endif // ANIMCHANNELTABLE_H
