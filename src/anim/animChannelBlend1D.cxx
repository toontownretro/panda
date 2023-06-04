/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBlend1D.cxx
 * @author brian
 * @date 2021-08-04
 */

#include "animChannelBlend1D.h"
#include "character.h"

#include <algorithm>

IMPLEMENT_CLASS(AnimChannelBlend1D);

/**
 *
 */
AnimChannelBlend1D::
AnimChannelBlend1D(const std::string &name) :
  AnimChannel(name),
  _blend_param(-1),
  _sorted(false)
{
}

/**
 *
 */
AnimChannelBlend1D::
AnimChannelBlend1D(const AnimChannelBlend1D &copy) :
  AnimChannel(copy),
  _blend_param(copy._blend_param),
  _channels(copy._channels),
  _sorted(copy._sorted)
{
}

/**
 *
 */
PT(AnimChannel) AnimChannelBlend1D::
make_copy() const {
  return new AnimChannelBlend1D(*this);
}

/**
 * Adds a channel into the multi-channel at the indicated blend coordinate.
 */
void AnimChannelBlend1D::
add_channel(AnimChannel *channel, PN_stdfloat coord) {
  Channel chan;
  chan._channel = channel;
  chan._blend_coord = std::clamp(coord, 0.0f, 1.0f);
  _channels.push_back(std::move(chan));

  // The overall frame rate and number of frames in the multi-channel is the
  // maximum of all channels within the multi-channel.
  if (_channels.size() == 1) {
    _fps = channel->get_frame_rate();
    _num_frames = channel->get_num_frames();

  } else {
    _fps = std::max(_fps, channel->get_frame_rate());
    _num_frames = std::max(_num_frames, channel->get_num_frames());
  }

  _sorted = false;
}

/**
 * Sorts the list of channels by increasing blend coordinate.  This must be
 * called before the channel is used on a Character.
 */
void AnimChannelBlend1D::
sort_channels() {
  if (_sorted) {
    return;
  }
  std::sort(_channels.begin(), _channels.end(),
    [](const Channel &a, const Channel &b) -> bool {
      return a._blend_coord < b._blend_coord;
    }
  );
  _sorted = true;
}

/**
 * Returns the length of the channel in the context of the indicated character.
 */
PN_stdfloat AnimChannelBlend1D::
get_length(Character *character) const {
  if (_channels.empty() || _blend_param < 0) {
    return 0.01f;
  }

  const Channel *from, *to;
  PN_stdfloat frac = ((AnimChannelBlend1D *)this)->get_blend_targets(character, from, to);

  nassertr(from != nullptr || to != nullptr, 0.01f);

  if (to == nullptr) {
    return from->_channel->get_length(character);

  } else if (from == nullptr) {
    return to->_channel->get_length(character);

  } else {
    // Return the weighted length of the two blend targets.

    PN_stdfloat from_length = from->_channel->get_length(character);
    PN_stdfloat to_length = to->_channel->get_length(character);

    return (from_length * (1.0f - frac)) + (to_length * frac);
  }
}

/**
 * Computes the two channels to blend between based on the pose parameter
 * value of the indicated character.  Returns the blend fraction between the
 * two channels.
 */
PN_stdfloat AnimChannelBlend1D::
get_blend_targets(Character *character, const Channel *&from, const Channel *&to) {
  int before, after;
  before = after = -1;

  from = to = nullptr;

  if (_channels.empty() || _blend_param < 0) {
    return 0.0f;
  }

  nassertr(_sorted, 0.0f);

  PN_stdfloat coord = character->get_pose_parameter(_blend_param).get_norm_value();

  for (size_t i = 0; i < _channels.size(); i++) {
    PN_stdfloat chan_coord = _channels[i]._blend_coord;

    if (chan_coord == coord) {
      after = (int)i;
      before = -1;
      break;
    }

    if (chan_coord < coord) {
      before = (int)i;
    } else if (chan_coord > coord) {
      after = (int)i;
      break;
    }
  }

  if (after >= 0) {
    to = &_channels[after];
  }

  if (before >= 0) {
    from = &_channels[before];
  }

  nassertr(from != nullptr || to != nullptr, 0.0f);

  if (from == nullptr || to == nullptr) {
    return 1.0f;

  } else {
    return (coord - from->_blend_coord) / (to->_blend_coord - from->_blend_coord);
  }
}

/**
 * Composites the channels within the multi-channel to compute a pose for each
 * joint.
 */
void AnimChannelBlend1D::
do_calc_pose(const AnimEvalContext &context, AnimEvalData &data) {
  if (_channels.empty() || _blend_param < 0) {
    return;
  }

  const Channel *from, *to;
  PN_stdfloat frac = get_blend_targets(context._character, from, to);

  nassertv(from != nullptr || to != nullptr);

  if (to == nullptr) {
    from->_channel->calc_pose(context, data);

  } else if (from == nullptr) {
    to->_channel->calc_pose(context, data);

  } else {
    // Evaluate both at full weight and blend between them.

    float this_net_weight = data._net_weight;

    float orig_weight = data._weight;
    data._weight = 1.0f;
    data._net_weight = this_net_weight * (1.0f - frac);
    from->_channel->calc_pose(context, data);

    AnimEvalData to_data;
    to_data._cycle = data._cycle;
    to_data._weight = 1.0f;
    to_data._net_weight = this_net_weight * frac;
    to->_channel->calc_pose(context, to_data);

    SIMDFloatVector vfrac = frac;
    SIMDFloatVector ve0 = SIMDFloatVector(1.0f) - vfrac;

    for (int i = 0; i < context._num_joint_groups; i++) {
      data._pose[i].pos *= ve0;
      data._pose[i].pos.madd_in_place(to_data._pose[i].pos, vfrac);

      data._pose[i].scale *= ve0;
      data._pose[i].scale.madd_in_place(to_data._pose[i].scale, vfrac);

      data._pose[i].shear *= ve0;
      data._pose[i].shear.madd_in_place(to_data._pose[i].shear, vfrac);

      data._pose[i].quat = data._pose[i].quat.align_lerp(to_data._pose[i].quat, vfrac);
    }

    data._weight = orig_weight;
    data._net_weight = this_net_weight;
  }
}

/**
 *
 */
LVector3 AnimChannelBlend1D::
get_root_motion_vector(Character *character) const {
  if (_channels.empty() || _blend_param < 0) {
    return LVector3(0.0f);
  }

  const Channel *from, *to;
  PN_stdfloat frac = ((AnimChannelBlend1D *)this)->get_blend_targets(character, from, to);

  if (to == nullptr) {
    return from->_channel->get_root_motion_vector(character);

  } else if (from == nullptr) {
    return to->_channel->get_root_motion_vector(character);

  } else {
    // Return the weighted average of the two motion vectors.
    LVector3 from_vec = from->_channel->get_root_motion_vector(character);
    LVector3 to_vec = to->_channel->get_root_motion_vector(character);
    return (from_vec * (1.0f - frac)) + (to_vec * frac);
  }
}

/**
 *
 */
void AnimChannelBlend1D::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *AnimChannelBlend1D::
make_from_bam(const FactoryParams &params) {
  AnimChannelBlend1D *chan = new AnimChannelBlend1D("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  chan->fillin(scan, manager);
  return chan;
}

/**
 *
 */
void AnimChannelBlend1D::
write_datagram(BamWriter *manager, Datagram &me) {
  AnimChannel::write_datagram(manager, me);

  me.add_int16(_blend_param);
  me.add_bool(_sorted);

  me.add_uint8(_channels.size());
  for (size_t i = 0; i < _channels.size(); i++) {
    const Channel *chan = &_channels[i];
    me.add_stdfloat(chan->_blend_coord);
    manager->write_pointer(me, chan->_channel);
  }
}

/**
 *
 */
int AnimChannelBlend1D::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = AnimChannel::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _channels.size(); i++) {
    _channels[i]._channel = DCAST(AnimChannel, p_list[pi++]);
  }

  return pi;
}

/**
 *
 */
void AnimChannelBlend1D::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel::fillin(scan, manager);

  _blend_param = scan.get_int16();
  _sorted = scan.get_bool();

  _channels.resize(scan.get_uint8());
  for (size_t i = 0; i < _channels.size(); i++) {
    _channels[i]._blend_coord = scan.get_stdfloat();
    manager->read_pointer(scan);
  }
}
