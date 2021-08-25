/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEventQueue.I
 * @author brian
 * @date 2021-08-23
 */

/**
 *
 */
INLINE AnimEventQueue::
AnimEventQueue() {
}

/**
 * Pushes a channel event onto the back of the queue.
 */
INLINE void AnimEventQueue::
push_event(int channel, int event) {
  _event_queue.push_back(EventInfo(channel, event));
}

/**
 * Returns true if there is an event queued up.
 */
INLINE bool AnimEventQueue::
has_event() const {
  return !_event_queue.empty();
}

/**
 * Pops and returns the event at the front of the queue.
 */
INLINE AnimEventQueue::EventInfo AnimEventQueue::
pop_event() {
  assert(has_event());

  EventInfo event = _event_queue.front();
  _event_queue.pop_front();
  return event;
}

/**
 *
 */
INLINE AnimEventQueue::EventInfo::
EventInfo(int channel, int event) :
  _channel_index(channel),
  _event_index(event)
{
}

/**
 * Returns the index of the channel that the event occurred on.
 */
INLINE int AnimEventQueue::EventInfo::
get_channel() const {
  return _channel_index;
}

/**
 * Returns the index of the event that occurred on the channel.
 */
INLINE int AnimEventQueue::EventInfo::
get_event() const {
  return _event_index;
}
