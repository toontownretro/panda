/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pvsCullEffect.h
 * @author brian
 * @date 2021-11-29
 */

#ifndef PVSCULLEFFECT_H
#define PVSCULLEFFECT_H

#include "pandabase.h"
#include "renderEffect.h"

/**
 * This is a special RenderEffect that culls an associated node against the
 * static potentially visible set of the scene.
 *
 * The PVS query is cached if the node did not move since the last check.
 */
class EXPCL_PANDA_PGRAPH PVSCullEffect : public RenderEffect {
protected:
  PVSCullEffect();

PUBLISHED:
  static CPT(RenderEffect) make();

public:
  virtual bool has_cull_callback() const override;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const override;


protected:
  virtual int compare_to_impl(const RenderEffect *other) const override;

private:
  bool do_cull_callback(CullTraverser *trav, CullTraverserData &data,
                        CPT(TransformState) &node_transform,
                        CPT(RenderState) &node_state);

private:
  // Cached vis info for the node.
  int _sectors[128];
  int _num_sectors;
  CPT(TransformState) _parent_net_transform;
  const BoundingVolume *_bounds;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "PVSCullEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pvsCullEffect.I"

#endif // PVSCULLEFFECT_H
