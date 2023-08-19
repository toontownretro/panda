/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomIndexArrayData.cxx
 * @author brian
 * @date 2023-08-16
 */

#include "geomIndexArrayData.h"
#include "preparedGraphicsObjects.h"
#include "indexBufferContext.h"

TypeHandle GeomIndexArrayData::_type_handle;

/**
 *
 */
GeomIndexArrayData::
GeomIndexArrayData() :
  _context(nullptr)
{
}

/**
 *
 */
GeomIndexArrayData::
GeomIndexArrayData(const GeomVertexArrayFormat *format, UsageHint usage_hint) :
  GeomVertexArrayData(format, usage_hint),
  _context(nullptr)
{
  nassertv(format->get_num_columns() == 1);
}

/**
 *
 */
GeomIndexArrayData::
GeomIndexArrayData(const GeomIndexArrayData &copy) :
  GeomVertexArrayData(copy),
  _context(nullptr)
{
}

/**
 *
 */
GeomIndexArrayData::
~GeomIndexArrayData() {
  release_all();
}

/**
 * Indicates that the data should be enqueued to be prepared in the indicated
 * prepared_objects at the beginning of the next frame.  This will ensure the
 * data is already loaded into the GSG if it is expected to be rendered soon.
 *
 * Use this function instead of prepare_now() to preload datas from a user
 * interface standpoint.
 */
void GeomIndexArrayData::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_vertex_buffer(this);
}

/**
 * Returns true if the data has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool GeomIndexArrayData::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_vertex_buffer_queued(this);
}

/**
 * Creates a context for the data on the particular GSG, if it does not
 * already exist.  Returns the new (or old) VertexBufferContext.  This assumes
 * that the GraphicsStateGuardian is the currently active rendering context
 * and that it is ready to accept new datas.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a data does not need to be explicitly prepared by the user before it may be
 * rendered.
 */
IndexBufferContext *GeomIndexArrayData::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {

  //if (prepared_objects->_gsg_id < _id_contexts.size() &&
  //    _id_contexts[prepared_objects->_gsg_id] != nullptr) {
  //  return _id_contexts[prepared_objects->_gsg_id];
  //}

  //if (!_id_contexts.empty() && _id_contexts[0] != nullptr) {
  //  return _id_contexts[0];
  //}

  if (_context != nullptr) {
    return _context;
  }

  //Contexts::const_iterator ci;
  //ci = _contexts.find(prepared_objects);
  //if (ci != _contexts.end()) {
  //  return (*ci).second;
  //}

  IndexBufferContext *vbc = prepared_objects->prepare_index_buffer_now(this, gsg);
  if (vbc != nullptr) {
    _contexts[prepared_objects] = vbc;

    _context = vbc;

    /*
    if (prepared_objects->_gsg_id >= _id_contexts.size()) {
      size_t orig_size = _id_contexts.size();
      _id_contexts.resize(prepared_objects->_gsg_id + 1);
      for (size_t i = orig_size; i < _id_contexts.size(); ++i) {
        _id_contexts[i] = nullptr;
      }
    }
    _id_contexts[prepared_objects->_gsg_id] = vbc;
    */
  }
  return vbc;
}

/**
 * Frees the data context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
bool GeomIndexArrayData::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    IndexBufferContext *vbc = (*ci).second;
    prepared_objects->release_index_buffer(vbc);
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_index_buffer(this);
}

/**
 * Frees the context allocated on all objects for which the data has been
 * declared.  Returns the number of contexts which have been freed.
 */
int GeomIndexArrayData::
release_all() {
  int num_freed = 0;

  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_vertex_buffer(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    IndexBufferContext *vbc = (*ci).second;
    prepared_objects->release_index_buffer(vbc);
  }

  // Now that we've called release_vertex_buffer() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

/**
 * Removes the indicated PreparedGraphicsObjects table from the data array's
 * table, without actually releasing the data array.  This is intended to be
 * called only from PreparedGraphicsObjects::release_vertex_buffer(); it
 * should never be called by user code.
 */
void GeomIndexArrayData::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {

  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);

    _context = nullptr;

    //if (prepared_objects->_gsg_id < _id_contexts.size()) {
    //  _id_contexts[prepared_objects->_gsg_id] = nullptr;
    //}

  } else {
    // If this assertion fails, clear_prepared() was given a prepared_objects
    // which the data array didn't know about.
    nassert_raise("unknown PreparedGraphicsObjects");
  }
}

/**
 *
 */
void GeomIndexArrayData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type GeomVertexArrayData is encountered in the Bam file.  It should create
 * the GeomVertexArrayData and extract its information from the file.
 */
TypedWritable *GeomIndexArrayData::
make_from_bam(const FactoryParams &params) {
  GeomIndexArrayData *object = new GeomIndexArrayData;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}
