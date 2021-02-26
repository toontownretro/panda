/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterNode.cxx
 * @author lachbr
 * @date 2021-02-23
 */

#include "characterNode.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pStatTimer.h"
#include "config_anim.h"
#include "nodePath.h"

TypeHandle CharacterNode::_type_handle;

PStatCollector CharacterNode::_animation_pcollector("*:Animation");

/**
 * Private constructor used during Bam reading only.
 */
CharacterNode::
CharacterNode() :
  PandaNode(""),
  _char(nullptr),
  _last_auto_update(-1.0f)
{
  set_cull_callback();
}

/**
 *
 */
CharacterNode::
CharacterNode(const CharacterNode &copy) :
  PandaNode(copy),
  _char(copy._char),
  _joints_pcollector(copy._joints_pcollector),
  _skinning_pcollector(copy._skinning_pcollector),
  _last_auto_update(-1.0f)
{
  set_cull_callback();
}

/**
 *
 */
CharacterNode::
CharacterNode(const std::string &name) :
  PandaNode(name),
  _joints_pcollector(PStatCollector(_animation_pcollector, name), "Joints"),
  _skinning_pcollector(PStatCollector(_animation_pcollector, name), "Vertices"),
  _char(new Character(name)),
  _last_auto_update(-1.0f)
{
  set_cull_callback();
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool CharacterNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // For now, we update the character during the cull traversal; this prevents
  // us from needlessly updating characters that aren't in the view frustum.
  // We may need a better way to do this optimization later, to handle
  // characters that might animate themselves in front of the view frustum.

  update();
  return true;
}

/**
 *
 */
bool CharacterNode::
safe_to_flatten() const {
  return false;
}

/**
 *
 */
bool CharacterNode::
safe_to_combine() const {
  return false;
}

/**
 *
 */
PandaNode *CharacterNode::
make_copy() const {
  return new CharacterNode(*this);
}

/**
 * This is used to support NodePath::calc_tight_bounds().  It is not intended
 * to be called directly, and it has nothing to do with the normal Panda
 * bounding-volume computation.
 *
 * If the node contains any geometry, this updates min_point and max_point to
 * enclose its bounding box.  found_any is to be set true if the node has any
 * geometry at all, or left alone if it has none.  This method may be called
 * over several nodes, so it may enter with min_point, max_point, and
 * found_any already set.
 *
 * This function is recursive, and the return value is the transform after it
 * has been modified by this node's transform.
 */
CPT(TransformState) CharacterNode::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  // This method is overridden by Character solely to provide a hook to force
  // the joints to update before computing the bounding volume.
  ((CharacterNode *)this)->update();

  // Unfortunately, calling update_to_now() will invalidate the node's cached
  // bounding volume, which causes a problem when this is called during the
  // traversal, e.g.  due to a ShowBoundsEffect.  As a hacky fix to work
  // around this, we will force-recompute all of the bounding volumes of our
  // parent nodes immediately.
  Parents parents = get_parents();
  for (size_t i = 0; i < parents.get_num_parents(); ++i) {
    PandaNode *parent = parents.get_parent(i);
    parent->get_bounds();
  }

  return PandaNode::calc_tight_bounds(min_point, max_point,
                                      found_any, transform, current_thread);
}

/**
 * Recalculates the Character's joints and vertices for the current frame.
 * Normally this is performed automatically during the render and need not be
 * called explicitly.
 */
void CharacterNode::
update() {
  LightMutexHolder holder(_lock);
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now != _last_auto_update) {
    _last_auto_update = now;

    if (anim_cat.is_spam()) {
      anim_cat.spam()
        << "Animating " << NodePath::any_path(this)
        << " at time " << now << "\n";
    }

    PStatTimer timer(_joints_pcollector);
    do_update();
  }
}

/**
 * Recalculates the character even if we think it doesn't need it.
 */
void CharacterNode::
force_update() {
  LightMutexHolder holder(_lock);

  // Statistics
  PStatTimer timer(_joints_pcollector);

  _char->force_update();
}

/**
 * The actual implementation of update().  Assumes the appropriate
 * PStatCollector has already been started, and that the lock is held.
 */
void CharacterNode::
do_update() {
  // Update all the joints and sliders.
  if (even_animation) {
    _char->force_update();
  } else {
    _char->update();
  }
}

/**
 *
 */
void CharacterNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void CharacterNode::
write_datagram(BamWriter *manager, Datagram &me) {
  PandaNode::write_datagram(manager, me);
  manager->write_pointer(me, _char);
}

/**
 *
 */
int CharacterNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  _char = DCAST(Character, p_list[pi++]);

  return pi;
}

/**
 *
 */
TypedWritable *CharacterNode::
make_from_bam(const FactoryParams &params) {
  CharacterNode *object = new CharacterNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 *
 */
void CharacterNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);
}
