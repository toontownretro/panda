/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartScalar.cxx
 * @author drose
 * @date 1999-02-23
 */

#include "movingPartScalar.h"
#include "animChannelScalarDynamic.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_chan.h"

template class MovingPart<ACScalarSwitchType>;

TypeHandle MovingPartScalar::_type_handle;

/**
 *
 */
MovingPartScalar::
~MovingPartScalar() {
}

/**
 * Attempts to blend the various scalar values indicated, and sets the _value
 * member to the resulting scalar.
 */
void MovingPartScalar::
get_blend_value(const PartBundle *root) {
  // If a forced channel is set on this particular scalar, we always return
  // that value instead of performing the blend.  Furthermore, the frame
  // number is always 0 for the forced channel.
  if (_forced_channel != nullptr) {
    ChannelType *channel = DCAST(ChannelType, _forced_channel);
    channel->get_value(0, _value);
    return;
  }

  PartBundle::CDReader cdata(root->_cycler);

  if (cdata->_active_controls.empty() || !cdata->_anim_graph) {
    // No channel is bound; supply the default value.
    if (restore_initial_pose) {
      _value = _default_value;
    }

  } else if (_effective_control != nullptr &&
             !cdata->_frame_blend_flag) {
    // A single value, the normal case.
    ChannelType *channel = DCAST(ChannelType, _effective_channel);
    channel->get_value(_effective_control->get_frame(), _value);

  } else {
    // TODO: Slider anim graph?
    _value = _default_value;
  }
}

/**
 * Freezes this particular joint so that it will always hold the specified
 * transform.  Returns true if this is a joint that can be so frozen, false
 * otherwise.  This is called internally by PartBundle::freeze_joint().
 */
bool MovingPartScalar::
apply_freeze_scalar(PN_stdfloat value) {
  _forced_channel = new AnimChannelFixed<ACScalarSwitchType>(get_name(), value);
  return true;
}

/**
 * Specifies a node to influence this particular joint so that it will always
 * hold the node's transform.  Returns true if this is a joint that can be so
 * controlled, false otherwise.  This is called internally by
 * PartBundle::control_joint().
 */
bool MovingPartScalar::
apply_control(PandaNode *node) {
  AnimChannelScalarDynamic *chan = new AnimChannelScalarDynamic(get_name());
  chan->set_value_node(node);
  _forced_channel = chan;
  return true;
}

/**
 * Factory method to generate a MovingPartScalar object
 */
TypedWritable* MovingPartScalar::
make_MovingPartScalar(const FactoryParams &params) {
  MovingPartScalar *me = new MovingPartScalar;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a MovingPartScalar object
 */
void MovingPartScalar::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_MovingPartScalar);
}
