/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapTraceScene.h
 * @author brian
 * @date 2023-06-21
 */

#ifndef MAPTRACESCENE_H
#define MAPTRACESCENE_H

#include "pandabase.h"
#include "pointerTo.h"
#include "rayTraceScene.h"
#include "rayTraceTriangleMesh.h"

class MapData;

/**
 * Helper class to build a ray tracing scene containing level geometry and
 * static prop geometry.
 *
 * During the map build process we have several ray tracing scenes, and
 * this is one of them, currently used only for generating light probe
 * sample positions.
 *
 * The other tracing representations are Steam Audio, lighting, and physics.
 */
class MapTraceScene {
public:
  enum Mask {
    M_world = 1,
    M_static_prop = 2,

    M_all = M_world | M_static_prop,
  };

  MapTraceScene(MapData *data);

  void build(unsigned int mask);

  bool hits_backface(const LPoint3 &start, const LPoint3 &end, unsigned int mask = M_all);

private:
  PT(RayTraceScene) _scene;
  PT(RayTraceTriangleMesh) _mesh_world;
  PT(RayTraceTriangleMesh) _mesh_props;

  MapData *_data;
};

#include "mapTraceScene.I"

#endif // MAPTRACESCENE_H
