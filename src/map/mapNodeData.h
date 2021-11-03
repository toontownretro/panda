/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapNodeData.h
 * @author brian
 * @date 2021-10-18
 */

#ifndef MAPNODEDATA_H
#define MAPNODEDATA_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "transformState.h"
#include "boundingVolume.h"
#include "geometricBoundingVolume.h"
#include "bitArray.h"
#include "texture.h"
#include "renderAttrib.h"
#include "pta_LVecBase3.h"

class MapData;
class MapAmbientProbe;

/**
 * Per-ModelNode data that contains lighting information.
 */
class MapLightData : public ReferenceCount {
public:
  PT(Texture) _cube_map;

  CPT(RenderState) _lighting_state;

  CPT(TransformState) _net_transform;

  PTA_LVecBase3 _probe_color;
  const MapAmbientProbe *_probe;

  // The light attrib at the last time we updated.  If this, or the net
  // transform of the node changes, we have to recompute the closest set of
  // lights.
  CPT(RenderAttrib) _light_attrib;
  CPT(RenderAttrib) _modified_light_attrib;
};

/**
 * Contains per-node data needed by the MapCullTraverser.  It is cached on
 * the node.
 */
class MapNodeData : public TypedReferenceCount {
  DECLARE_CLASS(MapNodeData, TypedReferenceCount);
public:
  // The last recorded world-space transform of the node's *parent*.
  // If this or the node's bounding volume (which contains the node's
  // local transform) changes, we have to recompute the node clusters.
  CPT(TransformState) _net_transform;

  // The last recorded bounding volume of the node.  If this differs from
  // the current, we have to recompute the node's occupied area clusters.
  CPT(BoundingVolume) _bounds;
  // World-space bounding volume, used to test against the area cluster tree.
  CPT(GeometricBoundingVolume) _net_bounds;

  // The map data pointer at the time we last computed the node's area
  // clusters.  We have to recompute it when the map changes because the
  // cluster set from the old map is irrelevant to the new map.
  MapData *_map_data;

  // The set of area clusters the node occupies.  This is AND'd against the
  // current camera PVS to determine if the node should be traversed further.
  // Since we query the cluster tree using the node's external bounds, this
  // also includes the clusters that children of this node occupy, so we can
  // early-out testing children nodes if a parent node is completely within
  // the PVS.
  BitArray _clusters;

  // Non-nullptr if it's a ModelNode.  Contains per-model lighting
  // information, such as the active ambient probe, active local lights,
  // and active cube map.
  PT(MapLightData) _light_data;
};

#include "mapNodeData.I"

#endif // MAPNODEDATA_H
