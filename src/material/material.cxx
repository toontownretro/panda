/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.cxx
 * @author brian
 * @date 2021-03-06
 */

#include "material.h"
#include "config_material.h"
#include "virtualFileSystem.h"
#include "bam.h"
#include "bamWriter.h"
#include "datagramOutputFile.h"
#include "pdxElement.h"
#include "pdxList.h"
#include "pdxValue.h"
#include "materialPool.h"

TypeHandle Material::_type_handle;

/**
 *
 */
Material::
Material(const std::string &name) :
  Namable(name),
  _num_params(0),
  _read_rawdata(false),
  _attrib_flags(F_none),
  _color(1),
  _color_scale(1),
  _transparency_mode(0), // TransparencyAttrib::M_none
  _alpha_test_mode(8), // PandaCompareFunc::M_always
  _alpha_test_ref(0.6f),
  _bin_sort(0),
  _depth_write(true),
  _depth_test_mode(2), // PandaCompareFunc::M_less
  _light_off(false),
  _fog_off(false),
  _render_mode(0), // RenderModeAttrib::M_unchanged
  _cull_face_mode(0), // CullFaceAttrib::M_unchanged
  _color_blend_mode(0) // No color blending
{
}

/**
 *
 */
void Material::
read_pdx(PDXElement *data, const DSearchPath &search_path) {
  // Reading parameters is up to derived materials.

  if (data->has_attribute("tags")) {
    PDXElement *tags = data->get_attribute_value("tags").get_element();
    nassertv(tags != nullptr);
    for (size_t i = 0; i < tags->get_num_attributes(); i++) {
      _tags[tags->get_attribute_name(i)] = tags->get_attribute_value(i).get_string();
    }
  }

  if (data->has_attribute("transparency")) {
    _attrib_flags |= F_transparency;

    std::string mode = data->get_attribute_value("transparency").get_string();
    if (mode == "off" || mode == "none" || mode == "no" || mode == "0") {
      _transparency_mode = 0;

    } else if (mode == "on" || mode == "yes" || mode == "alpha" || mode == "1") {
      _transparency_mode = 1;

    } else if (mode == "premultiplied_alpha" || mode == "premult_alpha" ||
               mode == "premult" || mode == "pm" || mode == "2") {
      _transparency_mode = 2;

    } else if (mode == "multisample" || mode == "ms" || mode == "3") {
      _transparency_mode = 3;

    } else if (mode == "multisample_mask" || mode == "ms_mask" || mode == "4") {
      _transparency_mode = 4;

    } else if (mode == "binary" || mode == "5") {
      _transparency_mode = 5;

    } else if (mode == "dual" || mode == "6") {
      _transparency_mode = 6;

    } else {
      material_cat.error()
        << "Unknown material transparency mode: " << mode << "\n";
    }
  }

  if (data->has_attribute("color")) {
    _attrib_flags |= F_color;
    _color.set(1, 1, 1, 1);
    data->get_attribute_value("color").to_vec4(_color);
  }

  if (data->has_attribute("color_scale")) {
    _attrib_flags |= F_color_scale;
    _color_scale.set(1, 1, 1, 1);
    data->get_attribute_value("color_scale").to_vec4(_color_scale);
  }

  if (data->has_attribute("alpha_test")) {
    _attrib_flags |= F_alpha_test;

    std::string mode = data->get_attribute_value("alpha_test").get_string();
    if (mode == "off" || mode == "no" || mode == "none" || mode == "0") {
      _alpha_test_mode = 0;

    } else if (mode == "never_draw" || mode == "never" || mode == "1") {
      _alpha_test_mode = 1;

    } else if (mode == "less" || mode == "l" || mode == "2") {
      _alpha_test_mode = 2;

    } else if (mode == "equal" || mode == "eq" || mode == "3") {
      _alpha_test_mode = 3;

    } else if (mode == "less_equal" || mode == "le" || mode == "leq" || mode == "4") {
      _alpha_test_mode = 4;

    } else if (mode == "greater" || mode == "gr" || mode == "5") {
      _alpha_test_mode = 5;

    } else if (mode == "not_equal" || mode == "ne" || mode == "neq" || mode == "6") {
      _alpha_test_mode = 6;

    } else if (mode == "greater_equal" || mode == "ge" || mode == "geq" || mode == "7") {
      _alpha_test_mode = 7;

    } else if (mode == "always" || mode == "8") {
      _alpha_test_mode = 8;

    } else {
      material_cat.error()
        << "Unknown material alpha test mode: " << mode << "\n";
    }
  }

  if (data->has_attribute("alpha_test_ref")) {
    _attrib_flags |= F_alpha_test;
    _alpha_test_ref = data->get_attribute_value("alpha_test_ref").get_float();
  }

  if (data->has_attribute("cull")) {

    _attrib_flags |= F_cull_face;
    std::string mode = data->get_attribute_value("cull").get_string();
    if (mode == "off" || mode == "none" || mode == "no" || mode == "double_sided" ||
        mode == "two_sided" || mode == "0") {
      _cull_face_mode = 0;

    } else if (mode == "on" || mode == "cw" || mode == "clockwise" || mode == "back" ||
               mode == "back_faces" || mode == "yes" || mode == "1") {
      _cull_face_mode = 1;

    } else if (mode == "ccw" || mode == "counter_clockwise" || mode == "front" ||
               mode == "front_faces" || mode == "2") {
      _cull_face_mode = 2;

    } else if (mode == "unchanged" || mode == "3") {
      _cull_face_mode = 3;

    } else {
      material_cat.error()
        << "Unknown material cull mode: " << mode << "\n";
    }
  }

  if (data->has_attribute("depth_write")) {
    _attrib_flags |= F_depth_write;
    std::string mode = data->get_attribute_value("depth_write").get_string();
    if (mode == "off" || mode == "no" || mode == "0") {
      _depth_write = false;
    } else {
      _depth_write = true;
    }
  }

  if (data->has_attribute("depth_test")) {
    _attrib_flags |= F_depth_test;

    std::string mode = data->get_attribute_value("depth_test").get_string();
    if (mode == "off" || mode == "no" || mode == "none" || mode == "0") {
      _depth_test_mode = 0;

    } else if (mode == "never_draw" || mode == "never" || mode == "1") {
      _depth_test_mode = 1;

    } else if (mode == "less" || mode == "l" || mode == "2") {
      _depth_test_mode = 2;

    } else if (mode == "equal" || mode == "eq" || mode == "3") {
      _depth_test_mode = 3;

    } else if (mode == "less_equal" || mode == "le" || mode == "leq" || mode == "4") {
      _depth_test_mode = 4;

    } else if (mode == "greater" || mode == "gr" || mode == "5") {
      _depth_test_mode = 5;

    } else if (mode == "not_equal" || mode == "ne" || mode == "neq" || mode == "6") {
      _depth_test_mode = 6;

    } else if (mode == "greater_equal" || mode == "ge" || mode == "geq" || mode == "7") {
      _depth_test_mode = 7;

    } else if (mode == "always" || mode == "8") {
      _depth_test_mode = 8;

    } else {
      material_cat.error()
        << "Unknown material depth test mode: " << mode << "\n";
    }
  }

  if (data->has_attribute("light")) {
    _attrib_flags |= F_light;

    std::string mode = data->get_attribute_value("light").get_string();
    if (mode == "off" || mode == "no" || mode == "0") {
      _light_off = true;

    } else {
      _light_off = false;
    }
  }

  if (data->has_attribute("fog")) {
    _attrib_flags |= F_fog;

    std::string mode = data->get_attribute_value("fog").get_string();
    if (mode == "off" || mode == "no" || mode == "0") {
      _fog_off = true;

    } else {
      _fog_off = false;
    }
  }

  if (data->has_attribute("bin")) {
    _attrib_flags |= F_bin;
    _bin_name = data->get_attribute_value("bin").get_string();
  }

  if (data->has_attribute("bin_sort")) {
    _attrib_flags |= F_bin;
    _bin_sort = data->get_attribute_value("bin_sort").get_int();
  }

  if (data->has_attribute("render_mode")) {
    _attrib_flags |= F_render_mode;

    std::string mode = data->get_attribute_value("render_mode").get_string();
    if (mode == "unchanged" || mode == "0") {
      _render_mode = 0;

    } else if (mode == "filled" || mode == "1") {
      _render_mode = 1;

    } else if (mode == "wireframe" || mode == "wire" || mode == "2") {
      _render_mode = 2;

    } else if (mode == "filled_wireframe" || mode == "filled_wire" || mode == "5") {
      _render_mode = 5;

    } else {
      material_cat.error()
        << "Unknown material render mode: " << mode << "\n";
    }
  }

  if (data->has_attribute("color_blend")) {
    _attrib_flags |= F_color_blend;

    std::string mode = data->get_attribute_value("color_blend").get_string();
    if (mode == "off" || mode == "none" || mode == "no" || mode == "0") {
      _color_blend_mode = 0;

    } else if (mode == "modulate" || mode == "1") {
      _color_blend_mode = 1;

    } else if (mode == "additive" || mode == "2") {
      _color_blend_mode = 2;

    } else {
      material_cat.error()
        << "Unknown material color blend mode: " << mode << "\n";
    }
  }
}

/**
 *
 */
void Material::
write_pdx(PDXElement *data, const Filename &filename) {
  PT(PDXElement) params = new PDXElement;
  for (size_t i = 0; i < _params.size(); i++) {
    const std::string &name = _params.get_key(i)->get_name();
    PDXValue value;
    _params.get_data(i)->to_pdx(value, filename);

    params->set_attribute(name, value);
  }
  data->set_attribute("parameters", PDXValue(params));

  PT(PDXElement) tags = new PDXElement;
  for (size_t i = 0; i < _tags.size(); i++) {
    tags->set_attribute(_tags.get_key(i), _tags.get_data(i));
  }
  data->set_attribute("tags", PDXValue(tags));
}

/**
 * Writes this material to the indicated text-format .pmat file.
 */
void Material::
write_pmat(const Filename &filename) {

  PT(PDXElement) data = new PDXElement;
  data->set_attribute("material", get_type().get_name());

  write_pdx(data, filename);

  PDXValue(data).write(filename);
}

/**
 * Writes this material to the indicated Bam-format .mto file.
 */
bool Material::
write_mto(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->delete_file(filename);

  DatagramOutputFile dout;
  if (!dout.open(filename)) {
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    return false;
  }

  BamWriter writer(&dout);
  if (!writer.init()) {
    return false;
  }

  // Always write raw data if we're using this method.
  writer.set_file_material_mode(BamWriter::BTM_rawdata);

  if (!writer.write_object(this)) {
    return false;
  }

  return true;
}

/**
 * Writes the contents of this object to a Datagram for shipping out to a Bam
 * file.
 */
void Material::
write_datagram(BamWriter *manager, Datagram &me) {
  BamWriter::BamTextureMode file_material_mode = manager->get_file_material_mode();
  bool has_rawdata = (file_material_mode == BamWriter::BTM_rawdata ||
                      _filename.empty());
  me.add_bool(has_rawdata);

  if (has_rawdata) {
    // Chuck the material guts into the Bam file.
    me.add_string(get_name());

    me.add_uint8(_params.size());
    for (size_t i = 0; i < _params.size(); i++) {
      manager->write_pointer(me, _params.get_data(i));
    }

    me.add_uint8(_tags.size());
    for (size_t i = 0; i < _tags.size(); i++) {
      me.add_string(_tags.get_key(i));
      me.add_string(_tags.get_data(i));
    }

    me.add_uint32(_attrib_flags);
    if (_attrib_flags & F_color) {
      _color.write_datagram(me);
    }
    if (_attrib_flags & F_color_scale) {
      _color_scale.write_datagram(me);
    }
    if (_attrib_flags & F_transparency) {
      me.add_uint8(_transparency_mode);
    }
    if (_attrib_flags & F_alpha_test) {
      me.add_uint8(_alpha_test_mode);
      me.add_stdfloat(_alpha_test_ref);
    }
    if (_attrib_flags & F_cull_face) {
      me.add_uint8(_cull_face_mode);
    }
    if (_attrib_flags & F_depth_write) {
      me.add_bool(_depth_write);
    }
    if (_attrib_flags & F_depth_test) {
      me.add_uint8(_depth_test_mode);
    }
    if (_attrib_flags & F_light) {
      me.add_bool(_light_off);
    }
    if (_attrib_flags & F_fog) {
      me.add_bool(_fog_off);
    }
    if (_attrib_flags & F_bin) {
      me.add_string(_bin_name);
      me.add_int32(_bin_sort);
    }
    if (_attrib_flags & F_render_mode) {
      me.add_uint8(_render_mode);
    }
    if (_attrib_flags & F_color_blend) {
      me.add_uint8(_color_blend_mode);
    }

  } else {
    // Just reference the filename.
    bool has_bam_dir = !manager->get_filename().empty();
    Filename bam_dir = manager->get_filename().get_dirname();
    Filename filename = get_filename();

    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    switch (file_material_mode) {
    case BamWriter::BTM_unchanged:
    case BamWriter::BTM_rawdata:
      break;

    case BamWriter::BTM_fullpath:
      filename = get_fullpath();
      break;

    case BamWriter::BTM_relative:
      filename = get_fullpath();
      bam_dir.make_absolute(vfs->get_cwd());
      if (!has_bam_dir || !filename.make_relative_to(bam_dir, true)) {
        filename.find_on_searchpath(get_model_path());
      }
      if (material_cat.is_debug()) {
        material_cat.debug()
          << "Material " << get_fullpath()
          << " found as " << filename << "\n";
      }
      break;

    case BamWriter::BTM_basename:
      filename = get_fullpath().get_basename();
      break;

    default:
      material_cat.error()
        << "Unsupported bam-material-mode: " << (int)file_material_mode << "\n";
    }

    me.add_string(filename.get_fullpath());
  }
}

/**
 *
 */
int Material::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  if (!_read_rawdata) {
    return 0;
  }

  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (int i = 0; i < _num_params; i++) {
    MaterialParamBase *param;
    DCAST_INTO_R(param, p_list[pi++], pi);
    _params[param->get_name()] = param;
  }

  return pi;
}

/**
 * Reads in the contents of this object from the Datagram.
 */
void Material::
fillin(DatagramIterator &scan, BamReader *manager) {
  _read_rawdata = scan.get_bool();

  if (_read_rawdata) {
    // Guts included.
    set_name(scan.get_string());

    _num_params = scan.get_uint8();
    manager->read_pointers(scan, _num_params);

    size_t num_tags = scan.get_uint8();
    for (size_t i = 0; i < num_tags; i++) {
      std::string key = scan.get_string();
      std::string value = scan.get_string();
      _tags[key] = value;
    }

    _attrib_flags = scan.get_uint32();
    if (_attrib_flags & F_color) {
      _color.read_datagram(scan);
    }
    if (_attrib_flags & F_color_scale) {
      _color_scale.read_datagram(scan);
    }
    if (_attrib_flags & F_transparency) {
      _transparency_mode = scan.get_uint8();
    }
    if (_attrib_flags & F_alpha_test) {
      _alpha_test_mode = scan.get_uint8();
      _alpha_test_ref = scan.get_stdfloat();
    }
    if (_attrib_flags & F_cull_face) {
      _cull_face_mode = scan.get_uint8();
    }
    if (_attrib_flags & F_depth_write) {
      _depth_write = scan.get_bool();
    }
    if (_attrib_flags & F_depth_test) {
      _depth_test_mode = scan.get_uint8();
    }
    if (_attrib_flags & F_light) {
      _light_off = scan.get_bool();
    }
    if (_attrib_flags & F_fog) {
      _fog_off = scan.get_bool();
    }
    if (_attrib_flags & F_bin) {
      _bin_name = scan.get_string();
      _bin_sort = scan.get_int32();
    }
    if (_attrib_flags & F_render_mode) {
      _render_mode = scan.get_uint8();
    }
    if (_attrib_flags & F_color_blend) {
      _color_blend_mode = scan.get_uint8();
    }

  } else {
    // It's just a filename reference to the real thing.
    _filename = scan.get_string();
    manager->register_change_this(change_this, this);
  }
}

/**
 *
 */
TypedWritable *Material::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // This method is called when the material read in just contains a filename
  // reference to the real thing.  We'll change the pointer to the real thing
  // loaded from disk using the MaterialPool.
  Material *old_mat = DCAST(Material, old_ptr);
  Material *new_mat = MaterialPool::load_material(old_mat->_filename);
  return new_mat;
}

/**
 *
 */
void Material::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *Material::
make_from_bam(const FactoryParams &params) {
  Material *mat = new Material("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  mat->fillin(scan, manager);
  return mat;
}
