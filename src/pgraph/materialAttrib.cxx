/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.cxx
 * @author lachbr
 * @date 2021-03-21
 */

#include "materialAttrib.h"
#include "virtualFileSystem.h"
#include "materialPool.h"
#include "config_pgraph.h"

TypeHandle MaterialAttrib::_type_handle;
int MaterialAttrib::_attrib_slot;

/**
 * Returns a new MaterialAttrib that contains no material.
 */
CPT(RenderAttrib) MaterialAttrib::
make_off() {
  return RenderAttribRegistry::get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 * Returns a new MaterialAttrib that references the indicated Material.
 */
CPT(RenderAttrib) MaterialAttrib::
make(Material *material) {
  MaterialAttrib *attr = new MaterialAttrib;
  attr->_material = material;

  return return_new(attr);
}

/**
 * Returns the default MaterialAttrib.
 */
CPT(RenderAttrib) MaterialAttrib::
make_default() {
  return RenderAttribRegistry::get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 *
 */
int MaterialAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const MaterialAttrib *that = (const MaterialAttrib *)other;

  if (this->_material != that->_material) {
    return this->_material < that->_material ? -1 : 1;
  }

  return 0;
}

/**
 *
 */
size_t MaterialAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = pointer_hash::add_hash(hash, _material.p());
  return hash;
}

/**
 *
 */
void MaterialAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MaterialAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  BamWriter::BamTextureMode file_material_mode = manager->get_file_material_mode();

  bool raw_data = file_material_mode == BamEnums::BTM_rawdata ||
                  _material == nullptr ||
                  _material->get_filename().empty();

  dg.add_bool(raw_data);

  if (raw_data) {
    // We're putting the material directly in the Bam file.
    manager->write_pointer(dg, _material);

  } else {
    // We're just putting a filename reference to the material.

    bool has_bam_dir = !manager->get_filename().empty();
    Filename bam_dir = manager->get_filename().get_dirname();
    Filename filename = _material->get_filename();

    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    switch (file_material_mode) {
    case BamWriter::BTM_unchanged:
    case BamWriter::BTM_rawdata:
      break;

    case BamWriter::BTM_fullpath:
      filename = _material->get_fullpath();
      break;

    case BamWriter::BTM_relative:
      filename = _material->get_fullpath();
      bam_dir.make_absolute(vfs->get_cwd());
      if (!has_bam_dir || !filename.make_relative_to(bam_dir, true)) {
        filename.find_on_searchpath(get_model_path());
      }
      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << "Material " << _material->get_fullpath()
          << " found as " << filename << "\n";
      }
      break;

    case BamWriter::BTM_basename:
      filename = _material->get_fullpath().get_basename();
      break;

    default:
      pgraph_cat.error()
        << "Unsupported bam-material-mode: " << (int)file_material_mode << "\n";
    }

    dg.add_string(filename.get_fullpath());
  }
}

/**
 *
 */
TypedWritable *MaterialAttrib::
make_from_bam(const FactoryParams &params) {
  MaterialAttrib *attr = new MaterialAttrib;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  attr->fillin(scan, manager);
  return attr;
}

/**
 *
 */
void MaterialAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _has_raw_data = scan.get_bool();

  if (_has_raw_data) {
    manager->read_pointer(scan);

  } else {
    Filename filename = scan.get_string();
    _material = MaterialPool::load_material(filename);
  }
}

/**
 *
 */
int MaterialAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  if (_has_raw_data) {
    Material *mat;
    DCAST_INTO_R(mat, p_list[pi++], pi);
    _material = mat;
  }

  return pi;
}
