/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinStateSorted.I
 * @author drose
 * @date 2005-03-22
 */

/**
 *
 */
INLINE CullBinStateSorted::
CullBinStateSorted(const std::string &name, GraphicsStateGuardianBase *gsg,
                   const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_state_sorted, gsg, draw_region_pcollector),
  _objects(get_class_type())
{
  _objects.reserve(8192);
  _object_indices.reserve(8192);
}

/**
 *
 */
INLINE CullBinStateSorted::ObjectData::
ObjectData(CullableObject &&object) :
  _object(std::move(object))
{
  if (_object._munged_data == nullptr) {
    _format = nullptr;
  } else {
    _format = _object._munged_data->get_format();
  }
}

/**
 *
 */
INLINE CullBinStateSorted::ObjectData::
ObjectData(ObjectData &&other) :
  _object(std::move(other._object)),
  _format(std::move(other._format))
{
}

/**
 *
 */
INLINE CullBinStateSorted::ObjectData::
ObjectData(const ObjectData &copy) :
  _object(copy._object),
  _format(copy._format)
{
}

/**
 *
 */
INLINE void CullBinStateSorted::ObjectData::
operator = (const ObjectData &copy) {
  _object = copy._object;
  _format = copy._format;
}

/**
 *
 */
INLINE void CullBinStateSorted::ObjectData::
operator = (ObjectData &&other) {
  _object = std::move(other._object);
  _format = std::move(other._format);
}

/**
 * Specifies the correct sort ordering for these objects.
 */
INLINE bool CullBinStateSorted::ObjectData::
operator < (const ObjectData &other) const {
  // Group by state changes, in approximate order from heaviest change to
  // lightest change.
  const RenderState *sa = _object._state;
  const RenderState *sb = other._object._state;

  if (sa == sb) {
    return 0;
  }

  int compare = sa->compare_sort(*sb);
  if (compare != 0) {
    return compare < 0;
  }

  // Vertex format changes are also fairly slow.
  if (_format != other._format) {
    return _format < other._format;
  }

  // Prevent unnecessary vertex buffer rebinds.
  //if (_object._munged_data != other._object._munged_data) {
  //  return _object._munged_data < other._object._munged_data;
  //}

  // Uniform updates are actually pretty fast.
  if (_object._internal_transform != other._object._internal_transform) {
    return _object._internal_transform < other._object._internal_transform;
  }

  return 0;
}
