/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEventQueue.h
 * @author brian
 * @date 2021-08-23
 */

#ifndef ANIMEVENTQUEUE_H
#define ANIMEVENTQUEUE_H

#include "pandabase.h"
#include "pdeque.h"

/**
 * Queues up AnimChannel events for processing by application code.
 */
class EXPCL_PANDA_ANIM AnimEventQueue {
PUBLISHED:
  /**
   * A single record of an AnimChannel event.
   */
  class EXPCL_PANDA_ANIM EventInfo {
  PUBLISHED:
    INLINE EventInfo(int channel, int event);

    INLINE int get_event() const;
    INLINE int get_channel() const;
    MAKE_PROPERTY(event, get_event);
    MAKE_PROPERTY(channel, get_channel);

  private:
    int _event_index;
    int _channel_index;
  };

  INLINE AnimEventQueue();

  INLINE void push_event(int channel, int event);
  INLINE bool has_event() const;
  INLINE EventInfo pop_event();

private:
  typedef pdeque<EventInfo> EventQueue;
  EventQueue _event_queue;
};

#include "animEventQueue.I"

#endif // ANIMEVENTQUEUE_H
