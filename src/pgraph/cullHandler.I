/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullHandler.I
 * @author drose
 * @date 2002-03-04
 */

/**
 *
 */
INLINE CullHandler::
CullHandler(HandleType type, CullResult *result, GraphicsStateGuardianBase *gsg) :
  _type(type),
  _result(result),
  _gsg(gsg)
{
}

/**
 * This is called as each Geom is discovered by the CullTraverser.
 */
INLINE void CullHandler::
record_object(CullableObject &object, const CullTraverser *traverser) {
  if (_type == HT_bin && _result != nullptr) {
    // Place the object into the appropriate bin for later sorting.
    _result->add_object(object, traverser);

  } else if (_type == HT_draw) {
    // Draw the object immediately, without binning or sorting.

    // Munge vertices as needed for the GSG's requirements, and the object's
    // current state.
    bool force = !_gsg->get_effective_incomplete_render();
    Thread *current_thread = traverser->get_current_thread();

    if (object.munge_geom(_gsg, /*_gsg->get_geom_munger(object->_state, current_thread)*/nullptr, traverser, force)) {
      // Now we can immediately draw the object.
      _gsg->draw_object(&object, force);
    }
  }
}

/**
 * This is called at the end of the traversal.
 */
INLINE void CullHandler::
end_traverse() {
}

/**
 * Draws the indicated CullableObject, with full support for decals if they
 * are attached to the object.  The appropriate state is set on the GSG before
 * drawing the object.
 */
INLINE void CullHandler::
draw(CullableObject &object, GraphicsStateGuardianBase *gsg,
     bool force, Thread *current_thread) {
  gsg->draw_object(&object, force);
}
