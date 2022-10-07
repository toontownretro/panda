/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullableObject.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Creates an empty CullableObject whose pointers can be filled in later.
 */
INLINE CullableObject::
CullableObject() {
  _num_instances = 1;
#ifdef RENDER_TRACK_GEOM_NODES
  _geom_node = nullptr;
#endif
  memset(&_sort_data, 0, sizeof(_sort_data));
}

/**
 * Creates a CullableObject based the indicated geom, with the indicated
 * render state and transform.
 */
INLINE CullableObject::
CullableObject(CPT(Geom) geom, CPT(RenderState) state,
               CPT(TransformState) internal_transform,
               Thread *current_thread) :
  _state(std::move(state)),
  _internal_transform(std::move(internal_transform)),
  _geom(std::move(geom)),
#ifdef RENDER_TRACK_GEOM_NODES
  _geom_node(nullptr),
#endif
  _num_instances(1)
{
  _primitive = nullptr;
  if (_geom != nullptr) {
    _munged_data = _geom->get_vertex_data_noref(current_thread);
    if (_geom->get_num_primitives(current_thread) == 1) {
      _primitive = _geom->get_fast_primitive(0, current_thread);
    }
  } else {
    _munged_data = nullptr;
  }
  memset(&_sort_data, 0, sizeof(_sort_data));
}

/**
 * Copies the CullableObject.
 */
INLINE CullableObject::
CullableObject(const CullableObject &copy) :
  _state(copy._state),
  _internal_transform(copy._internal_transform),
  _geom(copy._geom),
  _munged_data(copy._munged_data),
  _primitive(copy._primitive),
#ifdef RENDER_TRACK_GEOM_NODES
  _geom_node(copy._geom_node),
#endif
  _num_instances(copy._num_instances)
{
  memcpy(&_sort_data, &copy._sort_data, sizeof(_sort_data));
}

/**
 * Copies the CullableObject.
 */
INLINE void CullableObject::
operator = (const CullableObject &copy) {
  _draw_callback = copy._draw_callback;
  _state = copy._state;
  _internal_transform = copy._internal_transform;
  _geom = copy._geom;
  _munged_data = copy._munged_data;
  _primitive = copy._primitive;
#ifdef RENDER_TRACK_GEOM_NODES
  _geom_node = copy._geom_node;
#endif
  _num_instances = copy._num_instances;
  memcpy(&_sort_data, &copy._sort_data, sizeof(_sort_data));
}

/**
 *
 */
INLINE CullableObject::
CullableObject(CullableObject &&other) :
  _draw_callback(std::move(other._draw_callback)),
  _state(std::move(other._state)),
  _internal_transform(std::move(other._internal_transform)),
  _geom(std::move(other._geom)),
  _munged_data(std::move(other._munged_data)),
  _primitive(std::move(other._primitive)),
  _num_instances(std::move(other._num_instances))
{
  memmove(&_sort_data, &other._sort_data, sizeof(_sort_data));
}

/**
 *
 */
INLINE void CullableObject::
operator = (CullableObject &&other) {
  _draw_callback = std::move(other._draw_callback);
  _state = std::move(other._state);
  _internal_transform = std::move(other._internal_transform);
  _geom = std::move(other._geom);
  _munged_data = std::move(other._munged_data);
  _primitive = std::move(other._primitive);
  memmove(&_sort_data, &other._sort_data, sizeof(_sort_data));
  _num_instances = std::move(other._num_instances);
}

/**
 * Draws the cullable object on the GSG immediately, in the GSG's current
 * state.  This should only be called from the draw thread.
 */
INLINE void CullableObject::
draw(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  gsg->draw_object(this, force, current_thread);

/*
  if (UNLIKELY(_draw_callback != nullptr)) {
    // It has a callback associated.
    gsg->clear_before_callback();
    gsg->set_state_and_transform(_state, _internal_transform);
    GeomDrawCallbackData cbdata(this, gsg, force);
    _draw_callback->do_callback(&cbdata);
    if (cbdata.get_lost_state()) {
      // Tell the GSG to forget its state.
      gsg->clear_state_and_transform();
    }
    // Now the callback has taken care of drawing.
  }
  else if (LIKELY(_instances == nullptr)) {
    nassertv(_geom != nullptr);
    gsg->set_state_and_transform(_state, _internal_transform);
    draw_inline(gsg, force, current_thread);
  }
  else {
    // It has an instance list left over (not munged into vertex data), which
    // means the shader doesn't implement instancing.  Just render the object
    // more than once.
    nassertv(_geom != nullptr);
    GeomPipelineReader geom_reader(_geom, current_thread);
    GeomVertexDataPipelineReader data_reader(_munged_data, current_thread);
    data_reader.check_array_readers();

    for (const InstanceList::Instance &instance : *_instances) {
      gsg->set_state_and_transform(_state, _internal_transform->compose(instance.get_transform()));
      geom_reader.draw(gsg, &data_reader, _num_instances, force);
    }
  }
  */
}

/**
 * Returns true if all the data necessary to render this object is currently
 * resident in memory.  If this returns false, the data will be brought back
 * into memory shortly; try again later.
 */
INLINE bool CullableObject::
request_resident() const {
  bool resident = true;
  if (!_geom->request_resident()) {
    resident = false;
  }
  if (!_munged_data->request_resident()) {
    resident = false;
  }
  return resident;
}


/**
 * Specifies a CallbackObject that will be responsible for drawing this
 * object.
 */
INLINE void CullableObject::
set_draw_callback(CallbackObject *draw_callback) {
  _draw_callback = draw_callback;
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CullableObject::
flush_level() {
  //_sw_sprites_pcollector.flush_level();
}

/**
 * Draws the cullable object on the GSG immediately, in the GSG's current
 * state.  This should only be called from the draw thread.  Assumes the GSG
 * has already been set to the appropriate state.
 */
//INLINE void CullableObject::
//draw_inline(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  /*
  _geom->draw(gsg, _munged_data, _num_instances, force, current_thread);
  */
//}

/**
 * Invokes the draw callback, assuming one is set.  Crashes if not.
 */
INLINE void CullableObject::
draw_callback(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  gsg->clear_before_callback();
  gsg->set_state_and_transform(_state, _internal_transform);
  GeomDrawCallbackData cbdata(this, gsg, force);
  //_draw_callback->do_callback(&cbdata);
  if (cbdata.get_lost_state()) {
    // Tell the GSG to forget its state.
    gsg->clear_state_and_transform();
  }
}
