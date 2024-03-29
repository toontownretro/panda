/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file light.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "light.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

UpdateSeq Light::_sort_seq;

TypeHandle Light::_type_handle;


/**
 *
 */
CycleData *Light::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Light::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _color.write_datagram(dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void Light::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _color.read_datagram(scan);
}

/**
 *
 */
Light::
~Light() {
}

/**
 * Returns true if this is an AmbientLight, false if it is some other kind of
 * light.
 */
bool Light::
is_ambient_light() const {
  return false;
}

/**
 * Sets the color temperature of the light in kelvins.  This will recalculate
 * the light's color.
 *
 * The default value is 6500 K, corresponding to a perfectly white light
 * assuming a D65 white point.
 *
 * @since 1.10.0
 */
void Light::
set_color_temperature(PN_stdfloat temperature) {
  if (_has_color_temperature && _color_temperature == temperature) {
    return;
  }

  _has_color_temperature = true;
  _color_temperature = temperature;

  // Recalculate the color.
  PN_stdfloat x, y;

  if (temperature == 6500) {
    // sRGB D65 white point.
    x = 0.31271;
    y = 0.32902;

  } else {
    PN_stdfloat mm = 1000.0 / temperature;
    PN_stdfloat mm2 = mm * mm;
    PN_stdfloat mm3 = mm2 * mm;

    if (temperature < 4000) {
      x = -0.2661239 * mm3 - 0.2343580 * mm2 + 0.8776956 * mm + 0.179910;
    } else {
      x = -3.0258469 * mm3 + 2.1070379 * mm2 + 0.2226347 * mm + 0.240390;
    }

    PN_stdfloat x2 = x * x;
    PN_stdfloat x3 = x2 * x;
    if (temperature < 2222) {
      y = -1.1063814 * x3 - 1.34811020 * x2 + 2.18555832 * x - 0.20219683;
    } else if (temperature < 4000) {
      y = -0.9549476 * x3 - 1.37418593 * x2 + 2.09137015 * x - 0.16748867;
    } else {
      y =  3.0817580 * x3 - 5.87338670 * x2 + 3.75112997 * x - 0.37001483;
    }
  }

  // xyY to XYZ, assuming Y=1.
  LVecBase3 xyz(x / y, 1, (1 - x - y) / y);

  // Convert XYZ to linearized sRGB.
  const static LMatrix3 xyz_to_rgb(
    3.2406255, -0.9689307, 0.0557101,
    -1.537208, 1.8757561, -0.2040211,
    -0.4986286, 0.0415175, 1.0569959);

  LColor color(xyz_to_rgb.xform(xyz), 1);

  CDWriter cdata(_cycler);
  cdata->_color = color;
  cdata->_viz_geom_stale = true;
}

/**
 * For spotlights, returns the exponent that controls the amount of light
 * falloff from the center of the spotlight.  For other kinds of lights,
 * returns 0.
 */
PN_stdfloat Light::
get_exponent(Thread *current_thread) const {
  return 0;
}

/**
 * Returns the terms of the attenuation equation for the light.  These are, in
 * order, the constant, linear, and quadratic terms based on the distance from
 * the point to the vertex.
 */
const LVecBase3 &Light::
get_attenuation(Thread *current_thread) const {
  static const LVecBase3 no_atten(1, 0, 0);
  return no_atten;
}

/**
 * This is called when the light is added to a LightAttrib.
 */
void Light::
attrib_ref() {
}

/**
 * This is called when the light is removed from a LightAttrib.
 */
void Light::
attrib_unref() {
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
bool Light::
get_vector_to_light(LVector3 &, const LPoint3 &, const LMatrix4 &) {
  return false;
}

/**
 * Returns a GeomNode that may be rendered to visualize the Light.  This is
 * used during the cull traversal to render the Lights that have been made
 * visible.
 */
GeomNode *Light::
get_viz() {
  CDLockedReader cdata(_cycler);
  if (cdata->_viz_geom_stale) {
    CDWriter cdata_w(_cycler, cdata);

    cdata_w->_viz_geom = new GeomNode("viz");
    fill_viz_geom(cdata_w->_viz_geom);
    cdata_w->_viz_geom_stale = false;
  }
  return cdata->_viz_geom;
}

/**
 * Fills the indicated GeomNode up with Geoms suitable for rendering this
 * light.
 */
void Light::
fill_viz_geom(GeomNode *) {
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Light::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_bool(_has_color_temperature);
  if (_has_color_temperature) {
    dg.add_stdfloat(_color_temperature);
  } else {
    manager->write_cdata(dg, _cycler);
  }
  dg.add_int32(_priority);
  dg.add_bool(_stationary);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void Light::
fillin(DatagramIterator &scan, BamReader *manager) {
  _has_color_temperature = scan.get_bool();
  if (_has_color_temperature) {
    set_color_temperature(scan.get_stdfloat());
  } else {
    manager->read_cdata(scan, _cycler);
  }
  _priority = scan.get_int32();
  _stationary = scan.get_bool();
}
