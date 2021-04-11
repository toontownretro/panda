/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundle.h
 * @author drose
 * @date 1999-02-21
 */

#ifndef ANIMBUNDLE_H
#define ANIMBUNDLE_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "pointerTo.h"
#include "vector_stdfloat.h"
#include "luse.h"

class FactoryParams;

/**
 * There is one instance of this class for each joint in an animation.  It
 * specifies the indices for a joint into the animation tables for each
 * component.  It also specifies the number of sequential frames for a joint
 * for each component, because egg files optimize out components that remain
 * constant.
 */
class JointEntry {
PUBLISHED:
  std::string name;

  int first_frame, num_frames;
};
typedef pvector<JointEntry> JointEntries;

class ALIGN_16BYTE JointFrame {
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
};
typedef pvector<JointFrame> JointFrames;

/**
 * There is one instance of this class for each slider in an animation.  It
 * specifies the index for a slider into the animation table.
 */
class SliderEntry {
PUBLISHED:
  std::string name;

  int first_frame, num_frames;
};
typedef pvector<SliderEntry> SliderEntries;

/**
 * This is the root of an AnimChannel hierarchy.  It knows the frame rate and
 * number of frames of all the channels in the hierarchy (which must all
 * match).
 */
class EXPCL_PANDA_ANIM AnimBundle : public TypedWritableReferenceCount, public Namable {
protected:
  AnimBundle(const AnimBundle &copy);

PUBLISHED:
  INLINE explicit AnimBundle(const std::string &name, PN_stdfloat fps, int num_frames);

  PT(AnimBundle) copy_bundle() const;

  INLINE void set_base_frame_rate(PN_stdfloat fps);
  INLINE PN_stdfloat get_base_frame_rate() const;

  INLINE void set_num_frames(int num_frames);
  INLINE int get_num_frames() const;

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

  MAKE_PROPERTY(base_frame_rate, get_base_frame_rate, set_base_frame_rate);
  MAKE_PROPERTY(num_frames, get_num_frames, set_num_frames);

  virtual void output(std::ostream &out) const;

protected:
  INLINE AnimBundle();

  //virtual AnimGroup *make_copy(AnimGroup *parent) const;

private:
  PN_stdfloat _fps;
  int _num_frames;

  JointEntries _joint_entries;
  JointFrames _joint_frames;

  SliderEntries _slider_entries;
  vector_stdfloat _slider_table;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_AnimBundle(const FactoryParams &params);

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
    register_type(_type_handle, "AnimBundle",
                  TypedWritableReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

inline std::ostream &operator <<(std::ostream &out, const AnimBundle &bundle) {
  bundle.output(out);
  return out;
}


#include "animBundle.I"

#endif
