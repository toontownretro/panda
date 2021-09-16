/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.cxx
 * @author lachbr
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
  _read_rawdata(false)
{
}

/**
 *
 */
void Material::
read_pdx(PDXElement *data, const DSearchPath &search_path) {
  // Reading parameters is up to derived materials.

  if (data->has_attribute("tags")) {
    PDXList *tags = data->get_attribute_value("tags").get_list();
    nassertv(tags != nullptr);
    for (size_t i = 0; i < tags->size(); i++) {
      _tags.push_back(tags->get(i).get_string());
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

  PT(PDXList) tags = new PDXList;
  for (size_t i = 0; i < _tags.size(); i++) {
    tags->append(_tags[i]);
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
      me.add_string(_tags[i]);
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

    _tags.resize(scan.get_uint8());
    for (size_t i = 0; i < _tags.size(); i++) {
      _tags[i] = scan.get_string();
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
