/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartMatrix.cxx
 * @author drose
 * @date 1999-02-23
 */

#include "movingPartMatrix.h"
#include "animChannelMatrixDynamic.h"
#include "animChannelMatrixFixed.h"
#include "compose_matrix.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_chan.h"

template class MovingPart<ACMatrixSwitchType>;

TypeHandle MovingPartMatrix::_type_handle;

/**
 *
 */
MovingPartMatrix::
~MovingPartMatrix() {
}


/**
 * Creates and returns a new AnimChannel that is not part of any hierarchy,
 * but that returns the default value associated with this part.
 */
AnimChannelBase *MovingPartMatrix::
make_default_channel() const {
  LVecBase3 pos, hpr, scale, shear;
  decompose_matrix(_default_value, pos, hpr, scale, shear);
  return new AnimChannelMatrixFixed(get_name(), pos, hpr, scale);
}

/**
 * Attempts to blend the various matrix values indicated, and sets the _value
 * member to the resulting matrix.
 */
void MovingPartMatrix::
get_blend_value(const PartBundle *root) {
  // If a forced channel is set on this particular joint, we always return
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
    // A blend of two or more values.
    AnimGraphNode *graph = cdata->_anim_graph;
    graph->evaluate(this, cdata->_frame_blend_flag);

    _value = LMatrix4::scale_shear_mat(graph->get_scale(), graph->get_shear()) * graph->get_rotation();
    _value.set_row(3, graph->get_position());
  }
}

/**
 * Freezes this particular joint so that it will always hold the specified
 * transform.  Returns true if this is a joint that can be so frozen, false
 * otherwise.  This is called internally by PartBundle::freeze_joint().
 */
bool MovingPartMatrix::
apply_freeze_matrix(const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale) {
  _forced_channel = new AnimChannelMatrixFixed(get_name(), pos, hpr, scale);
  return true;
}

/**
 * Specifies a node to influence this particular joint so that it will always
 * hold the node's transform.  Returns true if this is a joint that can be so
 * controlled, false otherwise.  This is called internally by
 * PartBundle::control_joint().
 */
bool MovingPartMatrix::
apply_control(PandaNode *node) {
  AnimChannelMatrixDynamic *chan = new AnimChannelMatrixDynamic(get_name());
  chan->set_value_node(node);
  _forced_channel = chan;
  return true;
}

/**
 * Factory method to generate a MovingPartMatrix object
 */
TypedWritable* MovingPartMatrix::
make_MovingPartMatrix(const FactoryParams &params) {
  MovingPartMatrix *me = new MovingPartMatrix;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a MovingPartMatrix object
 */
void MovingPartMatrix::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_MovingPartMatrix);
}
