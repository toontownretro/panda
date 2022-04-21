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
#include "mathutil_simd.h"
#include "vector_string.h"
#include "bitArray.h"

class FactoryParams;

/**
 *
 */
typedef pvector<vector_float> FrameDatas;

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
  enum MotionFlags {
    MF_none = 0,
    MF_linear_x = 1,
    MF_linear_y = 2,
    MF_linear_z = 4,
  };

  // Flags that indicate which transform components are specified throughout
  // the animation for each joint.
  enum JointFormat : uint16_t {
    JF_none = 0,
    JF_x = 1 << 0,
    JF_y = 1 << 1,
    JF_z = 1 << 2,
    JF_h = 1 << 3,
    JF_p = 1 << 4,
    JF_r = 1 << 5,
    JF_i = 1 << 6,
    JF_j = 1 << 7,
    JF_k = 1 << 8,
    JF_a = 1 << 9,
    JF_b = 1 << 10,
    JF_c = 1 << 11,
  };

  INLINE explicit AnimChannelTable(const std::string &name, PN_stdfloat fps, int num_frames);

  INLINE void set_joint_table(FrameDatas &&table);
  INLINE const FrameDatas &get_joint_table() const;

  INLINE void set_joint_names(vector_string &&names);
  INLINE const vector_string &get_joint_names() const;

  INLINE void set_slider_table(vector_stdfloat &&table);
  INLINE const vector_stdfloat &get_slider_table() const;

  INLINE void get_scalar(int slider, int frame, PN_stdfloat &scalar) const;
  INLINE void get_scalar(const SliderEntry &slider, int frame, PN_stdfloat &scalar) const;

  int find_joint_channel(const std::string &name) const;
  int find_slider_channel(const std::string &name) const;

  INLINE void add_slider_entry(const SliderEntry &slider);
  INLINE const SliderEntry &get_slider_entry(int n) const;

  INLINE int get_num_joint_entries() const;
  INLINE int get_num_slider_entries() const;

  void extract_frame_data(int frame, AnimEvalData &data,
                          const AnimEvalContext &context, const vector_int &joint_map) const;
  void extract_frame0_data(AnimEvalData &data,
                           const AnimEvalContext &context, const vector_int &joint_map) const;
  int get_non0_joint_component_offset(int joint, JointFormat component) const;
  float extract_component_delta(int joint, JointFormat component);
  void offset_joint_component(int joint, JointFormat component, float offset);

  virtual PT(AnimChannel) make_copy() const override;
  virtual PN_stdfloat get_length(Character *character) const override;
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data) override;
  virtual LVector3 get_root_motion_vector(Character *character) const override;

  void calc_root_motion(unsigned int flags, int root_joint = 0);

public:
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;

protected:
  INLINE AnimChannelTable();
  INLINE AnimChannelTable(const AnimChannelTable &copy);

  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // Matches up to indices in frame data.
  vector_string _joint_names;
  typedef pvector<uint16_t> JointFormats;
  JointFormats _joint_formats;
  FrameDatas _frames;

  // Legacy storage for sliders, since we're not sampling them at the moment.
  SliderEntry _slider_entries[max_character_joints];
  size_t _num_slider_entries;
  vector_stdfloat _slider_table;

  LVector3 _root_motion_vector;

  friend class Character;
  friend class AnimBundleMaker;
};

#include "animChannelTable.I"

#endif // ANIMCHANNELTABLE_H
