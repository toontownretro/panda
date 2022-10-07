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
#include "character.h"
#include "randomizer.h"

TypeHandle EyeballNode::_type_handle;

/**
 *
 */
EyeballNode::
EyeballNode() :
  PandaNode(""),
  _view_target(NodePath()),
  _view_offset(TransformState::make_identity()),
  _z_offset(0),
  _radius(0),
  _iris_scale(1),
  _eye_size(1),
  _eye_shift(0),
  _last_update_frame(-1),
  _eye_origin(PTA_LVecBase3::empty_array(1, get_class_type())),
  _iris_projection_u(PTA_LVecBase4::empty_array(1, get_class_type())),
  _iris_projection_v(PTA_LVecBase4::empty_array(1, get_class_type())),
  _debug_enabled(false),
  _character(nullptr),
  _parent_joint(-1)
{
  CPT(RenderAttrib) sha = get_attrib(ShaderAttrib::get_class_slot());
  if (sha == nullptr) {
    sha = ShaderAttrib::make();
  }

  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("eyeOrigin", _eye_origin));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionU", _iris_projection_u));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionV", _iris_projection_v));

  set_attrib(sha);

  set_cull_callback();
}

/**
 *
 */
EyeballNode::
EyeballNode(const std::string &name, Character *character, int parent_joint) :
  PandaNode(name),
  _view_target(NodePath())
{
  _view_offset = TransformState::make_identity();
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
  _character = character;
  _parent_joint = parent_joint;

  CPT(RenderAttrib) sha = get_attrib(ShaderAttrib::get_class_slot());
  if (sha == nullptr) {
    sha = ShaderAttrib::make();
  }

  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("eyeOrigin", _eye_origin));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionU", _iris_projection_u));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionV", _iris_projection_v));

  set_attrib(sha);

  set_cull_callback();
}

/**
 *
 */
EyeballNode::
EyeballNode(const EyeballNode &copy) :
  PandaNode(copy),
  _view_target(copy._view_target)
{
  _view_offset = copy._view_offset;
  _parent_joint = copy._parent_joint;
  _character = copy._character;
  _eye_offset = copy._eye_offset;
  _z_offset = copy._z_offset;
  _iris_scale = copy._iris_scale;
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

  CPT(RenderAttrib) sha = get_attrib(ShaderAttrib::get_class_slot());
  if (sha == nullptr) {
    sha = ShaderAttrib::make();
  }

  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("eyeOrigin", _eye_origin));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionU", _iris_projection_u));
  sha = DCAST(ShaderAttrib, sha)->set_shader_input(ShaderInput("irisProjectionV", _iris_projection_v));

  set_attrib(sha);

  set_cull_callback();
}

/**
 *
 */
PandaNode *EyeballNode::
make_copy() const {
  return new EyeballNode(*this);
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

  if (_character.was_deleted()) {
    return true;
  }

  // Bring the parent joint into world coordinates and apply the eye offset to
  // get the current world space transform of the eye.
  CPT(TransformState) net_trans = TransformState::make_mat(_character->get_joint_net_transform(_parent_joint));
  net_trans = data.get_net_transform(trav)->compose(net_trans);
  net_trans = net_trans->compose(_eye_offset);

  LPoint3 origin = net_trans->get_pos();

  // Look directly at target.
  LPoint3 view_target;
  if (_view_target.is_empty()) {
    // Just look forward if we have no view target.
    LVector3 node_fwd = data.get_net_transform(trav)->get_quat().get_forward();
    view_target = origin + (node_fwd * 128);
  } else {
    CPT(TransformState) view_target_net = _view_target.get_node_path().get_net_transform();
    view_target = view_target_net->compose(_view_offset)->get_pos();
  }
  LVector3 look_forward = view_target - origin;
  look_forward.normalize();

  LQuaternion look_quat;
  look_at(look_quat, look_forward, CoordinateSystem::CS_default);

  LVector3 look_right = look_quat.get_right();
  LVector3 look_up = look_quat.get_up();

#if 1
  // Shift N degrees off of the target
  PN_stdfloat dz = _z_offset;

  look_forward += look_right * (_z_offset + dz);
  // Add random jitter
  //Randomizer random;
  //look_forward += look_right * (random.random_real(0.05) - 0.02);
  //look_forward += look_up * (random.random_real(0.05) - 0.02);
  look_forward.normalize();

  // Re-aim eyes
  look_right = look_forward.cross(look_up);
  look_right.normalize();

  look_up = look_right.cross(look_forward);
  look_up.normalize();
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

    CullableObject obj(geom, RenderState::make_empty(), trav->get_scene()->get_cs_world_transform(),
                       trav->get_current_thread());
    trav->get_cull_handler()->record_object(&obj, trav);
  }

  return true;
}

/**
 *
 */
void EyeballNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *EyeballNode::
make_from_bam(const FactoryParams &params) {
  EyeballNode *eye = new EyeballNode;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  eye->fillin(scan, manager);
  return eye;
}

/**
 *
 */
void EyeballNode::
write_datagram(BamWriter *manager, Datagram &me) {
  // Hack... remove the ShaderAttrib that was set on the node in the
  // constructor before the PandaNode writes the RenderState to the bam
  // file.  We don't want this info in the bam file, and writing ShaderAttribs
  // to bam files doesn't seem to even work correctly right now.
  CPT(RenderAttrib) sa = get_attrib(ShaderAttrib::get_class_slot());
  clear_attrib(ShaderAttrib::get_class_slot());

  PandaNode::write_datagram(manager, me);

  manager->write_pointer(me, _character.p());
  me.add_int16(_parent_joint);

  manager->write_pointer(me, _eye_offset);

  _eye_shift.write_datagram(me);

  me.add_stdfloat(_z_offset);
  me.add_stdfloat(_radius);
  me.add_stdfloat(_iris_scale);
  me.add_stdfloat(_eye_size);

  if (sa != nullptr) {
    set_attrib(sa);
  }
}

/**
 *
 */
void EyeballNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  manager->read_pointer(scan);
  _parent_joint = scan.get_int16();

  manager->read_pointer(scan);

  _eye_shift.read_datagram(scan);

  _z_offset = scan.get_stdfloat();
  _radius = scan.get_stdfloat();
  _iris_scale = scan.get_stdfloat();
  _eye_size = scan.get_stdfloat();
}

/**
 *
 */
int EyeballNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  _character = DCAST(Character, p_list[pi++]);
  _eye_offset = DCAST(TransformState, p_list[pi++]);

  return pi;
}
