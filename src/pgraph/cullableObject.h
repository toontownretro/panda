/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullableObject.h
 * @author drose
 * @date 2002-03-04
 */

#ifndef CULLABLEOBJECT_H
#define CULLABLEOBJECT_H

#include "pandabase.h"

#include "geom.h"
#include "geomVertexData.h"
#include "renderState.h"
#include "transformState.h"
#include "pointerTo.h"
#include "geomNode.h"
#include "cullTraverserData.h"
#include "pStatCollector.h"
#include "deletedChain.h"
#include "graphicsStateGuardianBase.h"
#include "sceneSetup.h"
#include "lightMutex.h"
#include "callbackObject.h"
#include "geomDrawCallbackData.h"
#include "instanceList.h"

class CullTraverser;

/**
 * The smallest atom of cull.  This is normally just a Geom and its associated
 * state, but it also contain a draw callback.
 */
class EXPCL_PANDA_PGRAPH CullableObject {
public:
  INLINE CullableObject();
  INLINE CullableObject(const Geom *geom, CPT(RenderState) state,
                        CPT(TransformState) internal_transform);

  INLINE CullableObject(const CullableObject &copy);
  INLINE CullableObject(CullableObject &&other);
  INLINE void operator = (const CullableObject &copy);
  INLINE void operator = (CullableObject &&other);

  bool munge_geom(GraphicsStateGuardianBase *gsg,
                  const CullTraverser *traverser, bool force);
  INLINE void draw(GraphicsStateGuardianBase *gsg,
                   bool force, Thread *current_thread);

  INLINE bool request_resident() const;
  INLINE static void flush_level();

  INLINE void set_draw_callback(CallbackObject *draw_callback);

  //INLINE void draw_inline(GraphicsStateGuardianBase *gsg,
  //                        bool force, Thread *current_thread);
  INLINE void draw_callback(GraphicsStateGuardianBase *gsg,
                            bool force, Thread *current_thread);

public:
  //ALLOC_DELETED_CHAIN(CullableObject);

  void output(std::ostream &out) const;

public:
  //CPT(InstanceList) _instances;
  PT(CallbackObject) _draw_callback;

  CPT(RenderState) _state;
  CPT(TransformState) _internal_transform;

  // The Geom to render.  Should weigh storing the pointer with
  // directly storing the information needed to render the Geom.
  CPT(Geom) _geom;
  const GeomVertexData *_munged_data;

  int _num_instances = 1;

  // This union contains the data used by various CullBins to sort their list
  // of CullableObjects.  Each bin type will only use one of the three
  // parameters to sort the objects, so a union is used to reduce the memory
  // requirements.
  union {
    PN_stdfloat _dist;
    int _draw_order;
  } _sort_data;

private:
  //void munge_instances(Thread *current_thread);
  static CPT(RenderState) get_flash_cpu_state();
  static CPT(RenderState) get_flash_hardware_state();

private:
  static PStatCollector _munge_pcollector;
  static PStatCollector _munge_geom_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "CullableObject");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const CullableObject &object) {
  object.output(out);
  return out;
}

#include "cullableObject.I"

#endif
