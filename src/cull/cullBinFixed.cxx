/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinFixed.cxx
 * @author drose
 * @date 2002-05-29
 */

#include "cullBinFixed.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"

#include <algorithm>

TypeHandle CullBinFixed::_type_handle;

/**
 *
 */
CullBinFixed::
~CullBinFixed() {
  //for (CullableObject *object : _objects) {
  //  delete object;
  //}
}

/**
 * Factory constructor for passing to the CullBinManager.
 */
CullBin *CullBinFixed::
make_bin(const std::string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinFixed(name, gsg, draw_region_pcollector);
}

/**
 * Adds a geom, along with its associated state, to the bin for rendering.
 */
void CullBinFixed::
add_object(CullableObject *object, Thread *current_thread) {
  object->_sort_data._draw_order = object->_state->get_draw_order();
  _objects.emplace_back(std::move(*object));
}

INLINE static bool
compare_objects_fixed(const CullableObject &a, const CullableObject &b) {
  return a._sort_data._draw_order < b._sort_data._draw_order;
};

/**
 * Called after all the geoms have been added, this indicates that the cull
 * process is finished for this frame and gives the bins a chance to do any
 * post-processing (like sorting) before moving on to draw.
 */
void CullBinFixed::
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  std::stable_sort(_objects.begin(), _objects.end(), compare_objects_fixed);
}

/**
 * Draws all the geoms in the bin, in the appropriate order.
 */
void CullBinFixed::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);
  _gsg->draw_objects(_objects, force, current_thread);
}

/**
 * Called by CullBin::make_result_graph() to add all the geoms to the special
 * cull result scene graph.
 */
void CullBinFixed::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  for (CullableObject &object : _objects) {
    builder.add_object(&object);
  }
}
