/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointLight.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "pointLight.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraphnodes.h"
#include "boundingSphere.h"

TypeHandle PointLight::_type_handle;

/**
 *
 */
CycleData *PointLight::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PointLight::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  _attenuation.write_datagram(dg);
  dg.add_stdfloat(_max_distance);
  _point.write_datagram(dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void PointLight::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _attenuation.read_datagram(scan);
  _max_distance = scan.get_stdfloat();
  _point.read_datagram(scan);
}

/**
 *
 */
PointLight::
PointLight(const std::string &name) :
  LightLensNode(name) {
  _light_type = Light::LT_point;
  PT(Lens) lens;
  lens = new PerspectiveLens(90, 90);
  lens->set_interocular_distance(0);
  lens->set_view_vector(1, 0, 0, 0, -1, 0);
  lens->set_near(0.01);
  //lens->set_far(get_max_distance());
  set_lens(0, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_interocular_distance(0);
  lens->set_view_vector(-1, 0, 0, 0, -1, 0);
  lens->set_near(0.01);
  //lens->set_far(get_max_distance());
  set_lens(1, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_interocular_distance(0);
  lens->set_view_vector(0, 1, 0, 0, 0, 1);
  lens->set_near(0.01);
  //lens->set_far(get_max_distance());
  set_lens(2, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_interocular_distance(0);
  lens->set_view_vector(0, -1, 0, 0, 0, -1);
  lens->set_near(0.01);
  //lens->set_far(get_max_distance());
  set_lens(3, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_interocular_distance(0);
  lens->set_view_vector(0, 0, 1, 0, -1, 0);
  lens->set_near(0.01);
  //lens->set_far(get_max_distance());
  set_lens(4, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_interocular_distance(0);
  lens->set_view_vector(0, 0, -1, 0, -1, 0);
  lens->set_near(0.01);
  //lens->set_far(get_max_distance());
  set_lens(5, lens);
}

/**
 * Do not call the copy constructor directly; instead, use make_copy() or
 * copy_subgraph() to make a copy of a node.
 */
PointLight::
PointLight(const PointLight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *PointLight::
make_copy() const {
  return new PointLight(*this);
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void PointLight::
xform(const LMatrix4 &mat) {
  LightLensNode::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_point = cdata->_point * mat;
  mark_viz_stale();
}

/**
 *
 */
void PointLight::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
  indent(out, indent_level + 2)
    << "attenuation " << get_attenuation() << "\n";

  if (!cinf(get_max_distance())) {
    indent(out, indent_level + 2)
      << "max distance " << get_max_distance() << "\n";
  }
  indent(out, indent_level + 2)
    << "point " << get_point() << "\n";
}

/**
 * Computes the vector from a particular vertex to this light.  The exact
 * vector depends on the type of light (e.g.  point lights return a different
 * result than directional lights).
 *
 * The input parameters are the vertex position in question, expressed in
 * object space, and the matrix which converts from light space to object
 * space.  The result is expressed in object space.
 *
 * The return value is true if the result is successful, or false if it cannot
 * be computed (e.g.  for an ambient light).
 */
bool PointLight::
get_vector_to_light(LVector3 &result, const LPoint3 &from_object_point,
                    const LMatrix4 &to_object_space) {
  CDReader cdata(_cycler);
  LPoint3 point = cdata->_point * to_object_space;

  result = point - from_object_point;
  return true;
}

/**
 * Returns the relative priority associated with all lights of this class.
 * This priority is used to order lights whose instance priority
 * (get_priority()) is the same--the idea is that other things being equal,
 * AmbientLights (for instance) are less important than DirectionalLights.
 */
int PointLight::
get_class_priority() const {
  return (int)CP_point_priority;
}

/**
 * Creates and returns a bounding volume that encloses all of the space this
 * light might illuminate.
 */
PT(GeometricBoundingVolume) PointLight::
make_light_bounds() const {
  return new BoundingSphere(get_point(), get_max_distance());
}

/**
 *
 */
void PointLight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

/**
 * Creates the shadow map texture.  Can be overridden.
 */
void PointLight::
setup_shadow_map() {
  if (_shadow_map != nullptr && _shadow_map->get_x_size() == _sb_size[0]) {
    // Nothing to do.
    return;
  }

  if (_sb_size[0] != _sb_size[1]) {
    pgraphnodes_cat.error()
      << "PointLight shadow buffers must have an equal width and height!\n";
  }

  if (_shadow_map == nullptr) {
    _shadow_map = new Texture(get_name());
  }

  _shadow_map->setup_cube_map(_sb_size[0], Texture::T_unsigned_byte, Texture::F_depth_component);
  _shadow_map->set_clear_color(LColor(1));
  _shadow_map->set_wrap_u(SamplerState::WM_clamp);
  _shadow_map->set_wrap_v(SamplerState::WM_clamp);
  _shadow_map->set_minfilter(SamplerState::FT_shadow);
  _shadow_map->set_magfilter(SamplerState::FT_shadow);
}

/**
 * Tells the BamReader how to create objects of type PointLight.
 */
void PointLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PointLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type PointLight is encountered in the Bam file.  It should create the
 * PointLight and extract its information from the file.
 */
TypedWritable *PointLight::
make_from_bam(const FactoryParams &params) {
  PointLight *node = new PointLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PointLight.
 */
void PointLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}
