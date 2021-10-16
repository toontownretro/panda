/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRender.h
 * @author brian
 * @date 2021-10-15
 */

#ifndef MAPRENDER_H
#define MAPRENDER_H

#include "pandabase.h"
#include "pandaNode.h"

class MapData;

/**
 * This node is intended to be used as the root of the 3-D scene graph when
 * maps are being used.  It implements a custom cull traverser that culls
 * dynamic nodes against the map's potentially visible set, and computes the
 * lights from the map that should affect models.
 */
class EXPCL_PANDA_MAP MapRender : public PandaNode {
  DECLARE_CLASS(MapRender, PandaNode);

PUBLISHED:
  MapRender(const std::string &name);

  INLINE void set_map_data(MapData *data);
  INLINE MapData *get_map_data() const;

public:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;
  virtual bool is_renderable() const override;

private:
  MapData *_map_data;
};

#include "mapRender.I"

#endif // MAPRENDER_H
