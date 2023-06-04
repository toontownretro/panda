/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBlend2D.I
 * @author brian
 * @date 2021-08-04
 */

/**
 *
 */
INLINE void AnimChannelBlend2D::
set_blend_x(int param) {
  _blend_x = param;
}

/**
 *
 */
INLINE int AnimChannelBlend2D::
get_blend_x() const {
  return _blend_x;
}

/**
 *
 */
INLINE void AnimChannelBlend2D::
set_blend_y(int param) {
  _blend_y = param;
}

/**
 *
 */
INLINE int AnimChannelBlend2D::
get_blend_y() const {
  return _blend_y;
}

/**
 *
 */
INLINE void AnimChannelBlend2D::
add_channel(AnimChannel *channel, const LPoint2 &coord) {
  nassertv(coord[0] >= 0 && coord[0] <= 1 && coord[1] >= 0 && coord[1] <= 1);

  Channel chan;
  chan._channel = channel;
  chan._point = coord;
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

  _has_triangles = false;
}

/**
 *
 */
INLINE int AnimChannelBlend2D::
get_num_channels() const {
  return (int)_channels.size();
}

/**
 *
 */
INLINE AnimChannel *AnimChannelBlend2D::
get_channel(int n) const {
  nassertr(n >= 0 && n < (int)_channels.size(), nullptr);
  return _channels[n]._channel;
}

/**
 *
 */
INLINE LPoint2 AnimChannelBlend2D::
get_channel_coord(int n) const {
  nassertr(n >= 0 && n < (int)_channels.size(), LPoint2());
  return _channels[n]._point;
}
