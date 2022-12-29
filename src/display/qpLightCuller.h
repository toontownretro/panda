/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLightCuller.h
 * @author brian
 * @date 2022-12-26
 */

#ifndef QPLIGHTCULLER_H
#define QPLIGHTCULLER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "texture.h"
#include "lens.h"
#include "luse.h"
#include "vector_int.h"
#include "pvector.h"
#include "nodePath.h"
#include "updateSeq.h"
#include "qpLightManager.h"
#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"

/**
 * This is an object that sectors the view frustum of a camera into a set of
 * AABBs, and bins lights into the sectors by testing if the culling volume of
 * the light intersects each sector.  The lights to bin come from a separate
 * object called qpLightManager, which maintains all of the lights in a
 * particular scene.
 *
 * The qpLightCuller is assigned to a DisplayRegion, and uses the camera and
 * lens assigned to the DisplayRegion to sector the view frustum and perform
 * view-space culling+binning of lights.
 *
 * The qpLightCuller fills a buffer that indexes into the light buffers created
 * by the associated qpLightManager, for each view frustum sector.  These buffers
 * are uploaded to the GPU and made available to shaders to only compute and
 * apply the necessary set of lights to a pixel.
 */
class EXPCL_PANDA_DISPLAY qpLightCuller : public ReferenceCount {
PUBLISHED:
  class TreeNode : public ReferenceCount {
  PUBLISHED:
    INLINE LPoint3 get_mins() const { return _mins; }
    INLINE LPoint3 get_maxs() const { return _maxs; }
    INLINE TreeNode *get_child(int i) const { return _children[i]; }
    INLINE int get_num_sectors() const { return (int)_sectors.size(); }
    INLINE int get_sector(int i) const { return _sectors[i]; }
    INLINE LVecBase3i get_div_mins() const { return _div_mins; }
    INLINE LVecBase3i get_div_maxs() const { return _div_maxs; }

  public:
    // This stuff is all precomputed once since it doesn't depend on the
    // actual frustum size.
    PT(TreeNode) _children[8];
    vector_int _sectors;
    LVecBase3i _div_mins, _div_maxs;

    // This is the only thing that actually changes, as we resize the
    // window or change the lens properties.
    LPoint3 _mins, _maxs;
  };
  class Sector {
  PUBLISHED:
    INLINE LPoint3 get_mins() const { return _mins; }
    INLINE LPoint3 get_maxs() const { return _maxs; }
    INLINE LVecBase3i get_coord() const { return _coord; }
    INLINE int get_num_lights() const { return _num_lights; }

  public:
    LVecBase3i _coord;
    LPoint3 _mins, _maxs;
    int _num_lights;
  };
  typedef pvector<Sector> Sectors;

  qpLightCuller(qpLightManager *light_mgr);

  void initialize();
  void bin_lights(const NodePath &camera, const Lens *lens);

  void recompute_sector_bounds();

  void tree_static_subdiv(TreeNode *parent);
  void r_calc_tree_bounds(TreeNode *node);

  LPoint2 get_div_lens_point(int x, int y);
  PN_stdfloat get_div_lens_depth(int z);
  LPoint3 lens_extrude_depth_linear(const LPoint3 &point2d);
  void calc_sector_bounds(const LVecBase3i &div_mins, const LVecBase3i &div_maxs,
                          LPoint3 &mins, LPoint3 &maxs);

  void r_bin_light(TreeNode *node, const LPoint3 &center, PN_stdfloat radius_sqr,
                   int light_index, bool is_dynamic, int *light_list);


  INLINE void set_frustum_div(int x, int y, int z);
  INLINE LVecBase3i get_frustum_div() const { return LVecBase3i(_x_div, _y_div, _z_div); }

  INLINE int get_num_sectors() const { return _num_sectors; }
  INLINE const Sector *get_sector(int i) const { return &_sectors[i]; }

  INLINE TreeNode *get_sector_tree() const { return _sector_tree; }

  INLINE Texture *get_light_list_buffer() const;
  INLINE qpLightManager *get_light_mgr() const { return _light_mgr; }

private:
  // Buffer texture containing a list of indices into the qpLightManager's
  // light buffers for each view frustum sector.  Each sector stores a max
  // of 64 light indices, and the list is 0-terminated.  A negative index
  // indicates a light from the dynamic buffer, and a >0 index indicates
  // a light from the static buffer.
  static constexpr int num_buffers = 2;
  PT(Texture) _light_list_buffers[num_buffers];
  unsigned char _buffer_index;

  class CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const override;

    Texture *_light_list_buffer;
  };
  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  // How the lens is divided to create sectors.
  int _x_div, _y_div, _z_div;
  int _num_sectors;
  // We use this to determine if the lens properties changed
  // and we need to recompute the sector bounds.
  UpdateSeq _last_lens_seq;
  const Lens *_lens;

  // We store a flat list of sectors as well as a AABB tree of them
  // to optimize light binning.
  PT(TreeNode) _sector_tree;
  Sectors _sectors;

  PT(qpLightManager) _light_mgr;
};

#include "qpLightCuller.I"

#endif // QPLIGHTCULLER_H
