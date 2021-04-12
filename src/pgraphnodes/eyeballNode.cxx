/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeballNode.cxx
 * @author lachbr
 * @date 2021-03-24
 */

#include "eyeballNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "renderState.h"
#include "shaderAttrib.h"
#include "clockObject.h"
#include "geomVertexFormat.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomLines.h"
#include "geom.h"
#include "cullHandler.h"
#include "cullableObject.h"
#include "look_at.h"

TypeHandle EyeballNode::_type_handle;

/**
 *
 */
EyeballNode::
EyeballNode(const std::string &name) :
  PandaNode(name)
{
  _view_target = LPoint3(0, 0, 0);
  _z_offset = 0;
  _radius = 0;
  _iris_scale = 1;
  _eye_size = 1;
  _eye_shift = LVector3(0, 0, 0);
  _last_update_frame = -1;
  _eye_origin = PTA_LVecBase3::empty_array(1, get_class_type());
  _iris_projection_u = PTA_LVecBase4::empty_array(1, get_class_type());
  _iris_projection_v = PTA_LVecBase4::empty_array(1, get_class_type());
  _debug_enabled = false;

  CPT(RenderState) state = get_state();
  CPT(RenderAttrib) sha = state->get_attrib(ShaderAttrib::get_class_slot());
  if (sha == nullptr) {
    sha = ShaderAttrib::make();
  }

  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("eyeOrigin", _eye_origin));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionU", _iris_projection_u));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionV", _iris_projection_v));

  set_state(state->set_attrib(sha));

  set_cull_callback();
}

/**
 *
 */
EyeballNode::
EyeballNode(const EyeballNode &copy) :
  PandaNode(copy)
{
  _view_target = copy._view_target;
  _z_offset = copy._z_offset;
  _radius = copy._radius;
  _eye_size = copy._eye_size;
  _eye_shift = copy._eye_shift;
  _last_update_frame = -1;
  _debug_enabled = copy._debug_enabled;

  _eye_origin = PTA_LVecBase3::empty_array(1, get_class_type());
  _eye_origin[0] = copy._eye_origin[0];

  _iris_projection_u = PTA_LVecBase4::empty_array(1, get_class_type());
  _iris_projection_u[0] = copy._iris_projection_u[0];

  _iris_projection_v = PTA_LVecBase4::empty_array(1, get_class_type());
  _iris_projection_v[0] = copy._iris_projection_v[0];

  CPT(RenderState) state = get_state();
  CPT(RenderAttrib) sha = state->get_attrib(ShaderAttrib::get_class_slot());
  if (sha == nullptr) {
    sha = ShaderAttrib::make();
  }

  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("eyeOrigin", _eye_origin));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionU", _iris_projection_u));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionV", _iris_projection_v));

  set_state(state->set_attrib(sha));

  set_cull_callback();
}

/**
 *
 */
bool EyeballNode::
is_renderable() const {
  return true;
}

/**
 *
 */
bool EyeballNode::
safe_to_flatten() const {
  return false;
}

/**
 *
 */
bool EyeballNode::
safe_to_combine() const {
  return false;
}

/**
 *
 */
bool EyeballNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {

  ClockObject *clock = ClockObject::get_global_clock();
  if (clock->get_frame_count() == _last_update_frame) {
    return true;
  }

  _last_update_frame = clock->get_frame_count();

  const TransformState *net_trans = data.get_net_transform(trav);

  LPoint3 origin = net_trans->get_pos();
  LQuaternion quat = net_trans->get_quat();
  LVector3 forward = quat.get_forward();
  LVector3 up = quat.get_up();
  LVector3 right = quat.get_right();

  // Look directly at target.
  LVector3 look_forward = _view_target - origin;
  look_forward.normalize();

  LQuaternion look_quat;
  look_at(look_quat, look_forward, CoordinateSystem::CS_default);

  LVector3 look_right = look_quat.get_right();
  LVector3 look_up = look_quat.get_up();

#if 0
  // Shift N degrees off of the target
  PN_stdfloat dz = _z_offset;

  look_forward += look_right * (_z_offset + dz);
  look_forward.normalize();

  // Re-aim eyes
  look_right = look_forward.cross(up);
  look_right.normalize();
#endif

  PN_stdfloat scale = (1.0f / _iris_scale) + _eye_size;

  if (scale > 0.0f) {
    scale = 1.0f / scale;
  }

  LVector3 u_xyz = look_right * -scale;
  LVector3 v_xyz = look_up * -scale;

  _iris_projection_u[0] = LVecBase4(u_xyz, -(origin.dot(u_xyz)) + 0.5f);
  _iris_projection_v[0] = LVecBase4(v_xyz, -(origin.dot(v_xyz)) + 0.5f);

  _eye_origin[0] = origin;


  if (_debug_enabled) {
    PT(GeomVertexData) vdata = new GeomVertexData("eyeball-debug", GeomVertexFormat::get_v3c4(), GeomEnums::UH_static);
    vdata->set_num_rows(6);

    GeomVertexWriter vw(vdata, "vertex");
    GeomVertexWriter cw(vdata, "color");

    // Forward axis
    vw.add_data3f(origin);
    cw.add_data4f(LColor(0, 1, 0, 1));
    vw.add_data3f(origin + (look_forward * _radius));
    cw.add_data4f(LColor(0, 1, 0, 1));

    // Up axis
    vw.add_data3f(origin);
    cw.add_data4f(LColor(0, 0, 1, 1));
    vw.add_data3f(origin + (look_up * _radius));
    cw.add_data4f(LColor(0, 0, 1, 1));

    // Right axis
    vw.add_data3f(origin);
    cw.add_data4f(LColor(1, 0, 0, 1));
    vw.add_data3f(origin + (look_right * _radius));
    cw.add_data4f(LColor(1, 0, 0, 1));

    PT(GeomLines) lines = new GeomLines(GeomEnums::UH_static);
    lines->add_vertices(0, 1);
    lines->close_primitive();
    lines->add_vertices(2, 3);
    lines->close_primitive();
    lines->add_vertices(4, 5);
    lines->close_primitive();

    PT(Geom) geom = new Geom(vdata);
    geom->add_primitive(lines);

    CullableObject *obj = new CullableObject(geom, RenderState::make_empty(), trav->get_scene()->get_cs_world_transform());
    trav->get_cull_handler()->record_object(obj, trav);
  }

  return true;
}
