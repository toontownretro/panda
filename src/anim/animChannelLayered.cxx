/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelLayered.cxx
 * @author brian
 * @date 2021-08-05
 */

#include "animChannelLayered.h"
#include "mathutil_misc.h"
#include "character.h"
#include "poseParameter.h"

IMPLEMENT_CLASS(AnimChannelLayered);

/**
 *
 */
AnimChannelLayered::
AnimChannelLayered(const std::string &name) :
  AnimChannel(name)
{
}

/**
 *
 */
AnimChannelLayered::
AnimChannelLayered(const AnimChannelLayered &copy) :
  AnimChannel(copy),
  _channels(copy._channels)
{
}

/**
 *
 */
void AnimChannelLayered::
add_channel(AnimChannel *channel, PN_stdfloat start_frame, PN_stdfloat peak_frame,
            PN_stdfloat tail_frame, PN_stdfloat end_frame, bool spline,
            bool no_blend, bool xfade, int pose_param) {
  Channel layer;
  layer._channel = channel;

  if (_channels.empty()) {
    // The first layer is used as the reference point for frame rate and
    // frame count of the overall channel.
    _num_frames = channel->get_num_frames();
    _fps = channel->get_frame_rate();
  }

  if (pose_param == -1) {
    PN_stdfloat num_frames = std::max(1.0f, _num_frames - 1.0f);
    layer._start = start_frame / num_frames;
    layer._peak = peak_frame / num_frames;
    layer._tail = tail_frame / num_frames;
    layer._end = end_frame / num_frames;

  } else {
    layer._start = start_frame;
    layer._peak = peak_frame;
    layer._tail = tail_frame;
    layer._end = end_frame;
  }

  layer._spline = spline;
  layer._no_blend = no_blend;
  layer._xfade = xfade;
  layer._pose_parameter = pose_param;

  _channels.push_back(std::move(layer));
}

/**
 *
 */
PT(AnimChannel) AnimChannelLayered::
make_copy() const {
  return new AnimChannelLayered(*this);
}

/**
 * Returns the duration of the channel in the context of the indicated
 * character.
 */
PN_stdfloat AnimChannelLayered::
get_length(Character *character) const {
  if (_channels.empty()) {
    return 0.0f;
  }

  // Return the length of the base layer.
  return _channels[0]._channel->get_length(character);
}

/**
 * Calculates a pose for the channel for each joint.
 */
void AnimChannelLayered::
do_calc_pose(const AnimEvalContext &context, AnimEvalData &data) {
  if (_channels.empty()) {
    return;
  }

  PN_stdfloat cycle = data._cycle;
  PN_stdfloat weight = 1.0f;

  for (size_t i = 0; i < _channels.size(); i++) {
    const Channel &layer = _channels[i];

    PN_stdfloat layer_cycle = cycle;
    PN_stdfloat layer_weight = 1.0f;

    PN_stdfloat start, peak, tail, end;

    start = layer._start;
    peak = layer._peak;
    end = layer._end;
    tail = layer._tail;

    if (start != end) {
      PN_stdfloat index;

      if (layer._pose_parameter == -1) {
        index = cycle;

      } else {
        // Layer driven by pose parameter.
        const PoseParameter &pp = context._character->get_pose_parameter(layer._pose_parameter);
        index = pp.get_value();
      }

      if (index < start || index >= end) {
        // Not in the frame range.
        continue;
      }

      PN_stdfloat scale = 1.0f;

      if (index < peak && start != peak) {
        // On the way up.
        scale = (index - start) / (peak - start);

      } else if (index > tail && end != tail) {
        // On the way down.
        scale = (end - index) / (end - tail);
      }

      if (layer._spline) {
        // Spline blend.
        scale = simple_spline(scale);
      }

      if (layer._xfade && (index > tail)) {
        layer_weight = (scale * weight) / (1 - weight + scale * weight);

      } else if (layer._no_blend) {
        layer_weight = scale;

      } else {
        layer_weight = weight * scale;
      }

      if (layer._pose_parameter == -1) {
        layer_cycle = (cycle - start) / (end - start);
      }
    }

    if (layer_weight <= 0.001f) {
      // Negligible weight.
      continue;
    }

    data._cycle = layer_cycle;
    data._weight = layer_weight;
    layer._channel->calc_pose(context, data);
  }
}

/**
 *
 */
void AnimChannelLayered::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *AnimChannelLayered::
make_from_bam(const FactoryParams &params) {
  AnimChannelLayered *chan = new AnimChannelLayered("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  chan->fillin(scan, manager);
  return chan;
}

/**
 *
 */
void AnimChannelLayered::
write_datagram(BamWriter *manager, Datagram &me) {
  AnimChannel::write_datagram(manager, me);

  me.add_uint8(_channels.size());
  for (size_t i = 0; i < _channels.size(); i++) {
    const Channel *chan = &_channels[i];
    manager->write_pointer(me, chan->_channel);
    me.add_stdfloat(chan->_start);
    me.add_stdfloat(chan->_peak);
    me.add_stdfloat(chan->_tail);
    me.add_stdfloat(chan->_end);
    me.add_bool(chan->_spline);
    me.add_bool(chan->_no_blend);
    me.add_bool(chan->_xfade);
    me.add_int16(chan->_pose_parameter);
  }
}

/**
 *
 */
int AnimChannelLayered::
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
void AnimChannelLayered::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel::fillin(scan, manager);

  _channels.resize(scan.get_uint8());
  for (size_t i = 0; i < _channels.size(); i++) {
    Channel &chan = _channels[i];
    manager->read_pointer(scan);
    chan._start = scan.get_stdfloat();
    chan._peak = scan.get_stdfloat();
    chan._tail = scan.get_stdfloat();
    chan._end = scan.get_stdfloat();
    chan._spline = scan.get_bool();
    chan._no_blend = scan.get_bool();
    chan._xfade = scan.get_bool();
    chan._pose_parameter = scan.get_int16();
  }
}
