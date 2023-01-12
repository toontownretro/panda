/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullLights.h
 * @author brian
 * @date 2020-12-10
 */

#ifndef CULLLIGHTS_H
#define CULLLIGHTS_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "luse.h"
#include "renderState.h"
#include "geometricBoundingVolume.h"

class LightAttrib;
class CullTraverser;
class CullTraverserData;

/**
 * This represents the set of lights that are definitely in effect for the
 * current node of the CullTraverserData, as well as on all child nodes.  Any
 * light in this list may be safely culled against.
 *
 * This does not include the lights that are in effect now, but might later be
 * turned off by a child node, since we can't safely cull against such lights.
 *
 * The bounding volume in this object are transformed for each level of the
 * scene graph.
 */
class EXPCL_PANDA_PGRAPH CullLights : public ReferenceCount {
protected:
  INLINE CullLights();
  INLINE CullLights(const CullLights &copy);
  INLINE void operator = (const CullLights &copy);

public:
  INLINE ~CullLights();
  ALLOC_DELETED_CHAIN(CullLights);

  INLINE bool is_empty() const;

  static CPT(CullLights) make_empty();
  CPT(CullLights) xform(const LMatrix4 &mat) const;
  CPT(CullLights) apply_state(const CullTraverser *trav,
                              const CullTraverserData *data,
                              const LightAttrib *net_attrib) const;
  CPT(CullLights) do_cull(int &result, CPT(RenderState) &state,
                          const GeometricBoundingVolume *node_gbv) const;

  CPT(CullLights) remove_light(const NodePath &light) const;

  void write(std::ostream &out) const;

private:
  typedef pmap<NodePath, PT(GeometricBoundingVolume) > Lights;
  Lights _lights;
};

// We can safely redefine this as a no-op.
template<>
INLINE void PointerToBase<CullLights>::update_type(To *ptr) {}

#include "cullLights.I"

#endif // CULLLIGHTS_H
