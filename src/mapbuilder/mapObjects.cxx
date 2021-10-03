/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapObjects.cxx
 * @author brian
 * @date 2021-07-06
 */

#include "config_mapbuilder.h"
#include "mapObjects.h"
#include "keyValues.h"
#include "string_utils.h"

#define OPEN_ITERATE_KEYS(kv) \
  for (size_t i = 0; i < kv->get_num_keys(); i++) { \
    const std::string &key = kv->get_key(i); \
    const std::string &value = kv->get_value(i); \

#define CLOSE_ITERATE_KEYS() }

#define OPEN_ITERATE_CHILDREN(kv) \
  for (size_t i = 0; i < kv->get_num_children(); i++) {\
    KeyValues *child = kv->get_child(i); \

#define CLOSE_ITERATE_CHILDREN() }


/**
 *
 */
MapFile::
MapFile() {
  _world = nullptr;
}

/**
 * Reads the map file at the indicated filename.  Returns true on success,
 * or false if the file could not be read.
 */
bool MapFile::
read(const Filename &fullpath) {
  PT(KeyValues) kv = KeyValues::load(fullpath);
  if (kv == nullptr) {
    return false;
  }

  OPEN_ITERATE_CHILDREN(kv)
    if (child->get_name() == "entity" || child->get_name() == "world") {
      if (!read_entity(child)) {
        return false;
      }
    }
  CLOSE_ITERATE_CHILDREN()

  return true;
}

/**
 * Reads the indicated map entity.
 */
bool MapFile::
read_entity(KeyValues *data) {
  PT(MapEntitySrc) ent = new MapEntitySrc;

  OPEN_ITERATE_KEYS(data)
    if (key == "id") {
      ent->_editor_id = atoi(value.c_str());

    } else if (key == "classname") {
      ent->_class_name = value;

    } else {
      ent->_properties[key] = value;
    }
  CLOSE_ITERATE_KEYS()

  OPEN_ITERATE_CHILDREN(data)
    if (child->get_name() == "solid") {
      if (!read_solid(ent, child)) {
        return false;
      }

    } else if (child->get_name() == "connections") {
      if (!read_connection(ent, child)) {
        return false;
      }
    }
  CLOSE_ITERATE_CHILDREN()

  _entities.push_back(ent);

  if (ent->_class_name == "worldspawn") {
    _world = ent;
  }

  return true;
}

/**
 * Reads the indicated map solid.
 */
bool MapFile::
read_solid(MapEntitySrc *entity, KeyValues *data) {
  PT(MapSolid) solid = new MapSolid;
  solid->_editor_id = atoi(data->get_value("id").c_str());

  OPEN_ITERATE_CHILDREN(data)
    if (child->get_name() == "side") {
      if (!read_side(solid, child)) {
        return false;
      }
    }
  CLOSE_ITERATE_CHILDREN()

  entity->_solids.push_back(solid);

  return true;
}

/**
 *
 */
bool MapFile::
read_side(MapSolid *solid, KeyValues *data) {
  PT(MapSide) side = new MapSide;
  side->_editor_id = atoi(data->get_value("id").c_str());

  OPEN_ITERATE_KEYS(data)
    if (key == "plane") {
      LPoint3 p0, p1, p2;
      KeyValues::parse_plane_points(value, p0, p1, p2);
      side->_plane = LPlane(p1, p0, p2);

    } else if (key == "material") {
      side->_material_filename = Filename::from_os_specific(value);

    } else if (key == "uaxis") {
      LVector2 shift_scale;
      KeyValues::parse_material_axis(value, side->_u_axis, shift_scale);
      side->_uv_shift[0] = shift_scale[0];
      side->_uv_scale[0] = shift_scale[1];
      if (side->_uv_scale[0] == 0.0f) {
        side->_uv_scale[0] = 1.0f;
      }

    } else if (key == "vaxis") {
      LVector2 shift_scale;
      KeyValues::parse_material_axis(value, side->_v_axis, shift_scale);
      side->_uv_shift[1] = shift_scale[0];
      side->_uv_scale[1] = shift_scale[1];
      if (side->_uv_scale[1] == 0.0f) {
        side->_uv_scale[1] = 1.0f;
      }

    } else if (key == "rotation") {
      side->_uv_rotation = atof(value.c_str());

    } else if (key == "lightmapscale") {
      side->_lightmap_scale = atof(value.c_str());

    } else if (key == "smoothing_groups") {
      side->_smoothing_groups = atoi(value.c_str());
    }
  CLOSE_ITERATE_KEYS()

  OPEN_ITERATE_CHILDREN(data)
    if (child->get_name() == "dispinfo") {
      if (!read_displacement(side, child)) {
        return false;
      }
    }
  CLOSE_ITERATE_CHILDREN()

  solid->_sides.push_back(side);

  return true;
}

/**
 *
 */
bool MapFile::
read_displacement(MapSide *side, KeyValues *data) {
  PT(MapDisplacement) disp = new MapDisplacement;

  OPEN_ITERATE_KEYS(data)
    if (key == "power") {
      disp->_power = atoi(value.c_str());

    } else if (key == "startposition") {
      LPoint3 &start_pos = disp->_start_position;
      sscanf(value.c_str(), "[%f %f %f]", &start_pos[0], &start_pos[1], &start_pos[2]);

    } else if (key == "elevation") {
      disp->_elevation = atof(value.c_str());

    } else if (key == "subdiv") {
      disp->_subdivide = (bool)atoi(value.c_str());
    }
  CLOSE_ITERATE_KEYS()

  int num_disp_sides = std::pow(2, disp->_power) + 1;

  // Populate the rows and columns of the displacement.
  disp->_rows.resize(num_disp_sides);
  for (size_t i = 0; i < disp->_rows.size(); i++) {
    disp->_rows[i]._vertices.resize(num_disp_sides);
  }

  OPEN_ITERATE_CHILDREN(data)
    if (child->get_name() == "normals") {
      for (size_t j = 0; j < child->get_num_keys(); j++) {
        vector_float normals = KeyValues::parse_float_list(child->get_value(j));
        for (size_t k = 0; k < num_disp_sides; k++) {
          size_t v = k * 3;
          disp->_rows[j]._vertices[k]._normal.set(
            normals[v], normals[v + 1], normals[v + 2]);
        }
      }

    } else if (child->get_name() == "distances") {
      for (size_t j = 0; j < child->get_num_keys(); j++) {
        vector_float distances = KeyValues::parse_float_list(child->get_value(j));
        assert(distances.size() == num_disp_sides);
        for (size_t k = 0; k < num_disp_sides; k++) {
          disp->_rows[j]._vertices[k]._distance = distances[k];
        }
      }

    } else if (child->get_name() == "offsets") {
      for (size_t j = 0; j < child->get_num_keys(); j++) {
        vector_float offsets = KeyValues::parse_float_list(child->get_value(j));
        for (size_t k = 0; k < num_disp_sides; k++) {
          size_t v = k * 3;
          disp->_rows[j]._vertices[k]._offset.set(
            offsets[v], offsets[v + 1], offsets[v + 2]);
        }
      }

    } else if (child->get_name() == "offset_normals") {
      for (size_t j = 0; j < child->get_num_keys(); j++) {
        vector_float offset_normals = KeyValues::parse_float_list(child->get_value(j));
        for (size_t k = 0; k < num_disp_sides; k++) {
          size_t v = k * 3;
          disp->_rows[j]._vertices[k]._offset_normal.set(
            offset_normals[v], offset_normals[v + 1], offset_normals[v + 2]);
        }
      }

    } else if (child->get_name() == "alphas") {
      for (size_t j = 0; j < child->get_num_keys(); j++) {
        vector_float alphas = KeyValues::parse_float_list(child->get_value(j));
        for (size_t k = 0; k < num_disp_sides; k++) {
          disp->_rows[j]._vertices[k]._alpha = alphas[k];
        }
      }
    }
  CLOSE_ITERATE_CHILDREN()

  side->_displacement = disp;

  return true;
}

/**
 *
 */
bool MapFile::
read_connection(MapEntitySrc *entity, KeyValues *data) {
  OPEN_ITERATE_KEYS(data)
    MapEntityConnection conn;
    conn._output_name = key;
    vector_string words;
    tokenize(value, words, ",");
    conn._entity_target_name = words[0];
    conn._input_name = words[1];
    conn._parameters = words[2];
    conn._delay = atof(words[3].c_str());
    conn._repeat = atoi(words[4].c_str());
    entity->_connections.push_back(std::move(conn));
  CLOSE_ITERATE_KEYS()

  return true;
}
