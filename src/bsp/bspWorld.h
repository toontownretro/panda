/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspWorld.h
 * @author brian
 * @date 2021-07-04
 */

#ifndef BSPWORLD_H
#define BSPWORLD_H

#include "pandabase.h"
#include "modelNode.h"
#include "pointerTo.h"
#include "geomNode.h"
#include "bspData.h"

/**
 * Node that contains the geometry for the world, draws the faces using BSP
 * structure.
 */
class EXPCL_PANDA_BSP BSPWorld : public ModelNode {
  DECLARE_CLASS(BSPWorld, ModelNode);

PUBLISHED:
  BSPWorld(BSPData *data);

  INLINE int get_cluster_count() const;
  INLINE void set_cluster_geom_node(int cluster, GeomNode *geom_node);
  INLINE GeomNode *get_cluster_geom_node(int cluster) const;

  virtual PandaNode *make_copy() const override;

public:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;

  virtual bool is_renderable() const override;
  virtual bool safe_to_flatten() const override;
  virtual bool safe_to_combine() const override;

protected:
  BSPWorld(const BSPWorld &copy);

private:
  PT(BSPData) _bsp_data;

  pvector<PT(GeomNode)> _cluster_geom_nodes;

};

#include "bspWorld.I"

#endif // BSPWORLD_H
