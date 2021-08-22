/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBundle.I
 * @author brian
 * @date 2021-08-05
 */

/**
 *
 */
INLINE AnimChannelBundle::
AnimChannelBundle(const std::string &name) :
  PandaNode(name)
{
}

/**
 * Adds a channel to the bundle.
 */
INLINE void AnimChannelBundle::
add_channel(AnimChannel *channel) {
  _channels.push_back(channel);
}

/**
 * Returns the number of channels in the bundle.
 */
INLINE int AnimChannelBundle::
get_num_channels() const {
  return (int)_channels.size();
}

/**
 * Returns the nth channel in the bundle.
 */
INLINE AnimChannel *AnimChannelBundle::
get_channel(int n) const {
  nassertr(n >= 0 && n < (int)_channels.size(), nullptr);
  return _channels[n];
}
