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
#include "pta_stdfloat.h"
#include "luse.h"

class FactoryParams;

class JointFrameData {
PUBLISHED:
  LVecBase3 pos;
  LQuaternion quat;
  LVecBase3 scale;

  bool operator == (const JointFrameData &other) const {
    return pos == other.pos && quat == other.quat && scale == other.scale;
  }

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "JointFrameData");
  }

private:
  static TypeHandle _type_handle;
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToBase<ReferenceCountedVector<JointFrameData> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArrayBase<JointFrameData>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, PointerToArray<JointFrameData>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_ANIM, EXPTP_PANDA_ANIM, ConstPointerToArray<JointFrameData>)

typedef PointerToArray<JointFrameData> PTA_JointFrameData;
typedef ConstPointerToArray<JointFrameData> CPTA_JointFrameData;

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

  INLINE void set_joint_channel_data(CPTA_JointFrameData data);
  INLINE CPTA_JointFrameData get_joint_channel_data() const;

  INLINE void set_slider_channel_data(CPTA_stdfloat data);
  INLINE CPTA_stdfloat get_slider_channel_data() const;

  int find_joint_channel(const std::string &name) const;
  int find_slider_channel(const std::string &name) const;

  INLINE void record_joint_channel_name(int channel, const std::string &name);
  INLINE void record_slider_channel_name(int channel, const std::string &name);

  INLINE int get_num_joint_channels() const;
  INLINE int get_num_slider_channels() const;

  MAKE_PROPERTY(base_frame_rate, get_base_frame_rate, set_base_frame_rate);
  MAKE_PROPERTY(num_frames, get_num_frames, set_num_frames);

  virtual void output(std::ostream &out) const;

  static INLINE int get_channel_data_index(int num_channels, int frame, int channel);

protected:
  INLINE AnimBundle();

  //virtual AnimGroup *make_copy(AnimGroup *parent) const;

private:
  PN_stdfloat _fps;
  int _num_frames;

  typedef pmap<std::string, int> ChannelNames;
  ChannelNames _joint_channel_names;
  ChannelNames _slider_channel_names;

  // One entry for each joint for each frame.
  CPTA_JointFrameData _joint_frames;

  // One entry for each slider for each frame.
  CPTA_stdfloat _slider_frames;

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
