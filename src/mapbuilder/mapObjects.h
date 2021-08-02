/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapObjects.h
 * @author brian
 * @date 2021-07-06
 */

#ifndef MAPOBJECTS_H
#define MAPOBJECTS_H

#include "pandabase.h"
#include "pvector.h"
#include "pmap.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "luse.h"
#include "vector_stdfloat.h"
#include "plane.h"

class KeyValues;

/**
 * Map objects as read in from the source (editor) map file.
 */

class MapDisplacementVertex {
public:
  LVector3 _normal;
  PN_stdfloat _distance;
  LVector3 _offset;
  LVector3 _offset_normal;
  PN_stdfloat _alpha;
};

class MapDisplacementRow {
public:
  pvector<MapDisplacementVertex> _vertices;
};

/**
 * Tessellated side of a solid.  Converted into a set of polygons during the
 * build process, so they are treated the same as everything else.
 */
class MapDisplacement : public ReferenceCount {
public:
  int _power;
  LPoint3 _start_position;
  PN_stdfloat _elevation;
  bool _subdivide;
  pvector<MapDisplacementRow> _rows;
};

/**
 * Single side of a solid.  Defined by a plane.  Polygons are created by
 * intersecting all planes of a solid.
 */
class MapSide : public ReferenceCount {
public:
  int _editor_id;

  // Plane that the side lies on.
  LPlane _plane;

  PN_stdfloat _lightmap_scale;

  int _smoothing_groups;

  Filename _material_filename;

  // Texture UV properties.
  LVector3 _u_axis;
  LVector3 _v_axis;
  LVector2 _uv_shift;
  LVector2 _uv_scale;
  PN_stdfloat _uv_rotation;

  // Non-null if the side is a displacement.
  PT(MapDisplacement) _displacement;
};

/**
 * Convex solid object.  Composed of a set of planes (sides).
 */
class MapSolid : public ReferenceCount {
public:
  int _editor_id;

  typedef pvector<PT(MapSide)> Sides;
  Sides _sides;
};

/**
 * An entity I/O connection.
 */
class MapEntityConnection {
public:
  std::string _output_name;
  std::string _entity_target_name;
  std::string _input_name;
  std::string _parameters;
  PN_stdfloat _delay;
  int _repeat;
};

/**
 *
 */
class MapEntitySrc : public ReferenceCount {
public:
  int _editor_id;
  std::string _class_name;

  typedef pmap<std::string, std::string> Properties;
  Properties _properties;
  typedef pvector<PT(MapSolid)> Solids;
  Solids _solids;

  typedef pvector<MapEntityConnection> Connections;
  Connections _connections;
};

/**
 *
 */
class MapFile : public ReferenceCount {
public:
  MapFile();

  bool read(const Filename &filename);
  bool read_entity(KeyValues *data);
  bool read_solid(MapEntitySrc *entity, KeyValues *data);
  bool read_side(MapSolid *solid, KeyValues *data);
  bool read_displacement(MapSide *side, KeyValues *data);
  bool read_connection(MapEntitySrc *entity, KeyValues *data);

  Filename _filename;

  typedef pvector<PT(MapEntitySrc)> Entities;
  Entities _entities;

  MapEntitySrc *_world;
};

#include "mapObjects.I"

#endif // MAPOBJECTS_H
