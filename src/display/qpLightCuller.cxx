/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLightCuller.cxx
 * @author brian
 * @date 2022-12-26
 */

#include "qpLightCuller.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector bin_lights_pcollector("LightCuller:BinLights");

/**
 *
 */
qpLightCuller::
qpLightCuller(qpLightManager *light_mgr) :
  _x_div(16),
  _y_div(8),
  _z_div(24),
  _num_sectors(16 * 8 * 24),
  _last_lens_seq(UpdateSeq::initial()),
  _lens(nullptr),
  _light_mgr(light_mgr),
  _buffer_index(0)
{
}

/**
 *
 */
void qpLightCuller::
initialize() {
  _num_sectors = _x_div * _y_div * _z_div;

  _sectors.resize(_num_sectors);
  int index = 0;
  for (int z = 0; z < _z_div; ++z) {
    for (int y = 0; y < _y_div; ++y) {
      for (int x = 0; x < _x_div; ++x) {
        _sectors[index]._coord.set(x, y, z);
        ++index;
      }
    }
  }

  // Build the sector tree.
  _sector_tree = new TreeNode;
  _sector_tree->_div_mins.set(0, 0, 0);
  _sector_tree->_div_maxs.set(_x_div, _y_div, _z_div);
  tree_static_subdiv(_sector_tree);

  for (int i = 0; i < num_buffers; ++i) {
    _light_list_buffers[i] = new Texture("light-list-buffer");
    _light_list_buffers[i]->setup_buffer_texture(_num_sectors * 64, Texture::T_int, Texture::F_r32i, GeomEnums::UH_dynamic);
    _light_list_buffers[i]->set_compression(Texture::CM_off);
    _light_list_buffers[i]->set_keep_ram_image(true);
  }

  _last_lens_seq = UpdateSeq::initial();
}

/**
 *
 */
void qpLightCuller::
bin_lights(const NodePath &camera, const Lens *lens) {
  PStatTimer timer(bin_lights_pcollector);

  if (_light_mgr == nullptr) {
    return;
  }

  _lens = lens;

  if (_sector_tree == nullptr) {
    initialize();
  }
  if (_last_lens_seq != _lens->get_last_change()) {
    recompute_sector_bounds();
    _last_lens_seq = _lens->get_last_change();
  }

  _lens = nullptr;

  for (Sector &s : _sectors) {
    s._num_lights = 0;
  }

  const LMatrix4 *world_to_view = camera.get_net_transform()->get_inverse_mat();
  if (world_to_view == nullptr) {
    return;
  }

  Texture *light_list_buffer = _light_list_buffers[_buffer_index];
  {
    CDWriter cdata(_cycler);
    cdata->_light_list_buffer = light_list_buffer;
  }

  PTA_uchar light_list_img = light_list_buffer->modify_ram_image();
  int *light_list_data = (int *)light_list_img.p();
  memset(light_list_data, 0, light_list_img.size());

  // Cull each static light.
  for (int i = 0; i < _light_mgr->get_num_static_lights(); ++i) {
    qpLight *light = _light_mgr->get_static_light(i);
    PN_stdfloat radius = light->get_cull_radius();
    LPoint3 pos = world_to_view->xform_point(light->get_pos());
    r_bin_light(_sector_tree, pos, radius * radius, i, false, light_list_data);
  }

  // Cull each dynamic light.
  for (int i = 0; i < _light_mgr->get_num_dynamic_lights(); ++i) {
    qpLight *light = _light_mgr->get_dynamic_light(i);
    PN_stdfloat radius = light->get_cull_radius();
    LPoint3 pos = world_to_view->xform_point(light->get_pos());
    r_bin_light(_sector_tree, pos, radius * radius, i, true, light_list_data);
  }

  ++_buffer_index;
  _buffer_index %= num_buffers;
}

/**
 *
 */
INLINE bool
qp_box_overlap(const LPoint3 &min_a, const LPoint3 &max_a, const LPoint3 &min_b, const LPoint3 &max_b) {
  return (max_a[0] >= min_b[0] && max_b[0] >= min_a[0] &&
          max_a[1] >= min_b[1] && max_b[1] >= min_a[1] &&
          max_a[2] >= min_b[2] && max_b[2] >= min_a[2]);
}

/**
 *
 */
INLINE bool
qp_aabb_sphere_overlap(const LPoint3 &mins, const LPoint3 &maxs, const LPoint3 &center, PN_stdfloat radius_sqr) {
  PN_stdfloat dmin = 0.0f;
  for (int i = 0; i < 3; ++i) {
    if (center[i] < mins[i]) {
      dmin += (center[i] - mins[i]) * (center[i] - mins[i]);
    } else if (center[i] > maxs[i]) {
      dmin += (center[i] - maxs[i]) * (center[i] - maxs[i]);
    }
  }
  return dmin <= radius_sqr;
}

/**
 *
 */
void qpLightCuller::
r_bin_light(TreeNode *node, const LPoint3 &center, PN_stdfloat radius_sqr, int light_index, bool is_dynamic, int *light_list) {
  if (!qp_aabb_sphere_overlap(node->_mins, node->_maxs, center, radius_sqr)) {
    return;
  }

  if (node->_children[0].is_null()) {
    // Leaf node.  Test and mark each sector.
    for (int i : node->_sectors) {
      Sector *s = &_sectors[i];
      if (s->_num_lights < 64 && qp_aabb_sphere_overlap(s->_mins, s->_maxs, center, radius_sqr)) {
        int index;
        if (is_dynamic) {
          index = ~light_index;
        } else {
          index = light_index + 1;
        }
        light_list[i * 64 + s->_num_lights] = index;
        ++(s->_num_lights);
      }
    }
    return;
  }

  for (int i = 0; i < 8; ++i) {
    r_bin_light(node->_children[i], center, radius_sqr, light_index, is_dynamic, light_list);
  }
}


/**
 *
 */
void qpLightCuller::
r_calc_tree_bounds(TreeNode *node) {
  calc_sector_bounds(node->_div_mins, node->_div_maxs, node->_mins, node->_maxs);

  for (int i = 0; i < 8; ++i) {
    if (node->_children[i] == nullptr) {
      continue;
    }
    r_calc_tree_bounds(node->_children[i]);
  }
}

/**
 * Recomputes the AABBs of each tree node and frustum sector.
 *
 * This should be called when the properties of the view frustum
 * change, such as the FOV or near/far planes.
 */
void qpLightCuller::
recompute_sector_bounds() {
  // Recompute the AABBs of the sector tree nodes.
  r_calc_tree_bounds(_sector_tree);

  // Now recompute the AABB of each individual sector.
  int index = 0;
  for (int z = 0; z < _z_div; ++z) {
    for (int y = 0; y < _y_div; ++y) {
      for (int x = 0; x < _x_div; ++x) {
        LPoint3 mins, maxs;
        calc_sector_bounds(LVecBase3i(x, y, z), LVecBase3i(x + 1, y + 1, z + 1), mins, maxs);
        _sectors[index]._mins = mins;
        _sectors[index]._maxs = maxs;
        ++index;
      }
    }
  }
}

/**
 * Returns a lens-space 2-D point for the given X/Y sector coordinate.
 */
LPoint2 qpLightCuller::
get_div_lens_point(int x, int y) {
  LPoint2 ret((float)x / (float)_x_div, (float)y / (float)_y_div);
  ret *= 2.0f;
  ret -= LPoint2(1.0f);
  return ret;
}

/**
 * Returns a lens-space depth value for the given sector Z coordinate.
 */
PN_stdfloat qpLightCuller::
get_div_lens_depth(int z) {
  PN_stdfloat ret = _lens->get_near() * cpow(_lens->get_far() / _lens->get_near(),
                                            (float)z / (float)_z_div);
  ret -= _lens->get_near();
  ret /= _lens->get_far() - _lens->get_near();
  ret *= 2.0f;
  ret -= 1.0f;
  return ret;
}

/**
 * Given the indicated lens-space point and depth value, returns a
 * view-space point by linearly interpolating between the far
 * and near points on the lens based on the depth value.
 */
LPoint3 qpLightCuller::
lens_extrude_depth_linear(const LPoint3 &point2d) {
  LPoint3 np, fp;
  _lens->extrude(point2d, np, fp);
  PN_stdfloat t = point2d[2] * 0.5f + 0.5f;
  return np + (fp - np) * t;
}

/**
 * Calculates a view-space bounding box for the given min/max sector
 * coordinate pair.
 */
void qpLightCuller::
calc_sector_bounds(const LVecBase3i &div_mins, const LVecBase3i &div_maxs,
                   LPoint3 &mins, LPoint3 &maxs) {
  // Get the 2-D points on the lens.
  LPoint2 lens_min = get_div_lens_point(div_mins[0], div_mins[1]);
  LPoint2 lens_max = get_div_lens_point(div_maxs[0], div_maxs[1]);

  // Get the lens-space depth values.
  PN_stdfloat lens_depth_min = get_div_lens_depth(div_mins[2]);
  PN_stdfloat lens_depth_max = get_div_lens_depth(div_maxs[2]);

  // Extrude into view-space.

  LPoint3 lln = lens_extrude_depth_linear(LPoint3(lens_min, lens_depth_min));
  LPoint3 llf = lens_extrude_depth_linear(LPoint3(lens_min, lens_depth_max));
  LPoint3 urn = lens_extrude_depth_linear(LPoint3(lens_max, lens_depth_min));
  LPoint3 urf = lens_extrude_depth_linear(LPoint3(lens_max, lens_depth_max));

  mins = lln.fmin(llf).fmin(urn).fmin(urf);
  maxs = lln.fmax(llf).fmax(urn).fmax(urf);
}

/**
 *
 */
void qpLightCuller::
tree_static_subdiv(TreeNode *parent) {
  LVecBase3i parent_size = parent->_div_maxs - parent->_div_mins;
  if (parent_size[0] % 2 != 0 || parent_size[1] % 2 != 0 || parent_size[2] % 2 != 0) {
    // If we can't cleanly half the sector division bounds, we are done.

    // Determine the sectors within the leaf node.
    for (int z = parent->_div_mins[2]; z < parent->_div_maxs[2]; ++z) {
      for (int y = parent->_div_mins[1]; y < parent->_div_maxs[1]; ++y) {
        for (int x = parent->_div_mins[0]; x < parent->_div_maxs[0]; ++x) {
          // Store index of this sector.
          int index = z * _y_div * _x_div;
          index += y * _x_div;
          index += x;
          parent->_sectors.push_back(index);
        }
      }
    }

    return;
  }

  LVecBase3i child_size = parent_size / 2;

  // Make the children.
  for (int i = 0; i < 8; ++i) {
    LVecBase3i this_mins = parent->_div_mins;
    if (i & 4) {
      this_mins[0] += child_size[0];
    }
    if (i & 2) {
      this_mins[1] += child_size[1];
    }
    if (i & 1) {
      this_mins[2] += child_size[2];
    }
    LVecBase3i this_maxs = this_mins + child_size;

    PT(TreeNode) child = new TreeNode;
    child->_div_maxs = this_maxs;
    child->_div_mins = this_mins;
    tree_static_subdiv(child);
    parent->_children[i] = child;
  }
}

/**
 *
 */
qpLightCuller::CData::
CData() :
  _light_list_buffer(nullptr)
{
}

/**
 *
 */
qpLightCuller::CData::
CData(const CData &copy) :
  _light_list_buffer(copy._light_list_buffer)
{
}

/**
 *
 */
CycleData *qpLightCuller::CData::
make_copy() const {
  return new CData(*this);
}
