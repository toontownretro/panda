/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamMatrix.cxx
 * @author brian
 * @date 2022-03-22
 */

#include "materialParamMatrix.h"
#include "pdxList.h"
#include "pdxElement.h"
#include "compose_matrix.h"

TypeHandle MaterialParamMatrix::_type_handle;

/**
 *
 */
bool MaterialParamMatrix::
from_pdx(const PDXValue &val, const DSearchPath &search_path) {
  if (val.is_list()) {
    // Flat list of numbers for each matrix cell.
    // Not sure why this would be used.

    PDXList *list = val.get_list();
    if (list->size() == 12) {
      // 3x3 matrix.  We assume this is a 2-D rotation-scale-translation
      // matrix.
      LMatrix3 mat3;
      if (!val.to_mat3(mat3)) {
        return false;
      }
      _value = LMatrix4::ident_mat();
      _value.set_row(0, mat3.get_row(0));
      _value.set_row(1, mat3.get_row(1));
      LVecBase2 trans = mat3.get_row2(2);
      _value.set_row(3, LVecBase4(trans[0], trans[1], 0.0f, 1.0f));

    } else if (list->size() == 16) {
      // Full 4x4 transform matrix.
      if (!val.to_mat4(_value)) {
        return false;
      }

    } else {
      // Invalid matrix size.
      return false;
    }

  } else if (val.is_element()) {
    // Transform components specified.

    PDXElement *element = val.get_element();

    bool is_3d = false;
    if (element->has_attribute("is_3d")) {
      is_3d = element->get_attribute_value("is_3d").get_bool();
    }

    LVecBase3 scale(1.0f, 1.0f, 1.0f);
    LVecBase3 shear(0.0f), translate(0.0f), hpr(0.0f);

    if (element->has_attribute("scale")) {
      const PDXValue &scalev = element->get_attribute_value("scale");
      if (scalev.is_float() || scalev.is_int()) {
        scale.fill(scalev.get_float());

      } else {
        if (!scalev.to_vec3(scale)) {
          return false;
        }
      }
    }

    if (element->has_attribute("rotate")) {
      if (!is_3d) {
        hpr[0] = element->get_attribute_value("rotate").get_float();
      } else {
        if (!element->get_attribute_value("rotate").to_vec3(hpr)) {
          return false;
        }
      }
    }

    if (element->has_attribute("translate")) {
      if (!element->get_attribute_value("translate").to_vec3(translate)) {
        return false;
      }
    }

    if (element->has_attribute("shear")) {
      if (!element->get_attribute_value("shear").to_vec3(shear)) {
        return false;
      }
    }

    compose_matrix(_value, scale, shear, hpr, translate);

  } else {
    // Invalid PDXValue type for matrix parameter.
    return false;
  }

  return true;
}

/**
 *
 */
void MaterialParamMatrix::
to_pdx(PDXValue &val, const Filename &filename) {
  PT(PDXElement) element = new PDXElement;

  // We always write the full 4x4 matrix, as that is what we store.
  element->set_attribute("is_3d", true);

  LVecBase3 scale, shear, translate, hpr;
  decompose_matrix(_value, scale, shear, hpr, translate);

  if (!scale.almost_equal(LVecBase3(1.0f))) {
    PDXValue scalev;
    scalev.from_vec3(scale);
    element->set_attribute("scale", scalev);
  }

  if (!shear.almost_equal(LVecBase3(0.0f))) {
    PDXValue shearv;
    shearv.from_vec3(shear);
    element->set_attribute("shear", shearv);
  }

  if (!translate.almost_equal(LVecBase3(0.0f))) {
    PDXValue translatev;
    translatev.from_vec3(translate);
    element->set_attribute("translate", translatev);
  }

  if (!hpr.almost_equal(LVecBase3(0.0f))) {
    PDXValue hprv;
    hprv.from_vec3(hpr);
    element->set_attribute("rotate", hprv);
  }
}

/**
 *
 */
void MaterialParamMatrix::
write_datagram(BamWriter *manager, Datagram &me) {
  MaterialParamBase::write_datagram(manager, me);

  _value.write_datagram(me);
}
/**
 *
 */
void MaterialParamMatrix::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MaterialParamMatrix::
fillin(DatagramIterator &scan, BamReader *manager) {
  MaterialParamBase::fillin(scan, manager);

  _value.read_datagram(scan);
}

/**
 *
 */
TypedWritable *MaterialParamMatrix::
make_from_bam(const FactoryParams &params) {
  MaterialParamMatrix *param = new MaterialParamMatrix("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  param->fillin(scan, manager);
  return param;
}
