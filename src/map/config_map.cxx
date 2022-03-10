/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_map.cxx
 * @author lachbr
 * @date 2020-12-18
 */

#include "config_map.h"
#include "mapData.h"
#include "mapEntity.h"
#include "mapRoot.h"
#include "mapRender.h"
#include "mapCullTraverser.h"
#include "mapNodeData.h"
#include "dynamicVisNode.h"
#include "mapLightingEffect.h"
#include "spatialPartition.h"
#include "bspTree.h"
#include "kdTree.h"
#include "staticPartitionedObjectNode.h"

NotifyCategoryDef(map, "");

ConfigureDef(config_map);
ConfigureFn(config_map) {
  init_libmap();
}

void
init_libmap() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  MapData::init_type();
  MapEntity::init_type();
  MapRoot::init_type();
  MapRender::init_type();
  MapCullTraverser::init_type();
  MapNodeData::init_type();
  DynamicVisNode::init_type();
  MapLightingEffect::init_type();
  SpatialPartition::init_type();
  BSPTree::init_type();
  KDTree::init_type();
  StaticPartitionedObjectNode::init_type();

  MapData::register_with_read_factory();
  MapEntity::register_with_read_factory();
  MapRoot::register_with_read_factory();
  BSPTree::register_with_read_factory();
  KDTree::register_with_read_factory();
}
