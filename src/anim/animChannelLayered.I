/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelLayered.I
 * @author brian
 * @date 2021-08-05
 */

/**
 * Returns the number of layers in the channel.
 */
INLINE int AnimChannelLayered::
get_num_channels() const {
  return (int)_channels.size();
}

/**
 * Returns the AnimChannel associated with the nth layer of this channel.
 */
INLINE AnimChannel *AnimChannelLayered::
get_channel(int n) const {
  nassertr(n >= 0 && n < (int)_channels.size(), nullptr);
  return _channels[n]._channel;
}

