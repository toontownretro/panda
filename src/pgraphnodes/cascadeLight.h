/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cascadeLight.h
 * @author lachbr
 * @date 2020-10-26
 */

#ifndef CASCADELIGHT_H
#define CASCADELIGHT_H

#include "config_pgraphnodes.h"
#include "directionalLight.h"
#include "orthographicLens.h"
#include "pointerTo.h"
#include "weakNodePath.h"

class CullTraverser;
class CullTraverserData;

/**
 * Specialization of DirectionalLight that uses cascaded shadow mapping to
 * render shadows.
 */
class EXPCL_PANDA_PGRAPHNODES CascadeLight : public DirectionalLight {
PUBLISHED:
  explicit CascadeLight(const std::string &name);

  INLINE NodePath get_scene_camera() const;
  INLINE void set_scene_camera(const NodePath &camera);
  MAKE_PROPERTY(scene_camera, get_scene_camera, set_scene_camera);

  INLINE int get_num_cascades() const;
  INLINE void set_num_cascades(int cascades);
  MAKE_PROPERTY(num_cascades, get_num_cascades, set_num_cascades);

  INLINE float get_csm_distance() const;
  INLINE void set_csm_distance(float distance);
  MAKE_PROPERTY(csm_distance, get_csm_distance, set_csm_distance);

  INLINE float get_sun_distance() const;
  INLINE void set_sun_distance(float distance);
  MAKE_PROPERTY(sun_distance, get_sun_distance, set_sun_distance);

  INLINE bool get_use_fixed_film_size() const;
  INLINE void set_use_fixed_film_size(bool flag);
  MAKE_PROPERTY(fixed_film_size, get_use_fixed_film_size, set_use_fixed_film_size);

  INLINE float get_log_factor() const;
  INLINE void set_log_factor(float factor);
  MAKE_PROPERTY(log_factor, get_log_factor, set_log_factor);

  INLINE float get_border_bias() const;
  INLINE void set_border_bias(float bias);
  MAKE_PROPERTY(border_bias, get_border_bias, set_border_bias);

  INLINE const LMatrix4 &get_cascade_mvp(int n) const;
  INLINE const LVecBase2 &get_cascade_near_far(int n) const;

public:
  virtual void setup_shadow_map();

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

private:
  // Used to access the near and far points in the array
  enum CoordinateOrigin {
    CO_ul = 0,
    CO_ur,
    CO_ll,
    CO_lr
  };

  // Data about a single cascade.
  struct Cascade {
    PT(OrthographicLens) lens;
    NodePath node;
  };

  void setup_cascades();
  void compute_pssm_splits(const LMatrix4 &transform, float distance,
                           Camera *scene_cam, CullTraverser *trav,
                           CullTraverserData &data);
  INLINE float get_split_start(int n) const;
  INLINE LPoint3 get_interpolated_point(CoordinateOrigin origin, float depth);
  INLINE void compute_mvp(int n, LMatrix4 &mvp, const TransformState *to_local);
  INLINE void get_film_properties(LVecBase2 &size, LVecBase2 &offset,
                                  const LVecBase3 &mins,
                                  const LVecBase3 &maxs);
  void calc_min_max_extents(LVecBase3 &mins, LVecBase3 &maxs, const LMatrix4 &transform,
                            LVecBase3 *const proj_points, const Cascade &c);

private:

  // We don't cycle these because they aren't expected to change that often, if
  // at all.
  float _csm_distance;
  float _sun_distance;
  float _log_factor;
  float _border_bias;
  int _num_cascades;
  bool _fixed_film_size;
  WeakNodePath _scene_camera;

  typedef pvector<Cascade> Cascades;
  Cascades _cascades;

  // This stuff is modified internally when we compute the cascades, only on
  // the Cull thread, so it doesn't need cycling either.
  pvector<LVecBase2> _max_film_sizes;
  LPoint3 _curr_near_points[4];
  LPoint3 _curr_far_points[4];

  // We may be visited by multiple cameras during the cull traversal, but we
  // only need to update the cascades once per frame.  This is only read from
  // and written to on the Cull thread, so we don't need to worry about
  // cycling it.
  int _last_update_frame;

  // This is the data that must be cycled between pipeline stages.
  class CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);

    virtual CycleData *make_copy() const;

    // The cascade projection matrices and near-far points need to be accessed
    // by the Draw thread when submitting the data to a shader, so they must be
    // cycled.
    pvector<LVecBase2> _cascade_nearfar;
    pvector<LMatrix4> _cascade_mvps;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DirectionalLight::init_type();
    register_type(_type_handle, "CascadeLight",
                  DirectionalLight::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cascadeLight.I"

#endif // CASCADELIGHT_H
