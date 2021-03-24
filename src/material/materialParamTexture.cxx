/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamTexture.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamTexture.h"
#include "texturePool.h"
#include "virtualFileSystem.h"

TypeHandle MaterialParamTexture::_type_handle;

/**
 *
 */
bool MaterialParamTexture::
from_string(const std::string &str, const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename filename = str;
  if (!vfs->resolve_filename(filename, search_path)) {
    return false;
  }

  _value = TexturePool::load_texture(filename);
  return _value != nullptr;
}

/**
 *
 */
void MaterialParamTexture::
to_string(std::string &str, const Filename &filename) {
  if (_value == nullptr) {
    str = "__null__";

  } else {
    Filename out_filename = filename;
    out_filename.make_canonical();
    Filename tex_filename = _value->get_fullpath();
    tex_filename.make_canonical();
    tex_filename.make_relative_to(out_filename.get_dirname());
    str = tex_filename.get_fullpath();
  }
}

/**
 *
 */
void MaterialParamTexture::
write_datagram(BamWriter *manager, Datagram &me) {
  MaterialParamBase::write_datagram(manager, me);

  manager->write_pointer(me, _value);
}

/**
 *
 */
int MaterialParamTexture::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = MaterialParamBase::complete_pointers(p_list, manager);

  Texture *tex;
  DCAST_INTO_R(tex, p_list[pi++], pi);
  _value = tex;

  return pi;
}

/**
 *
 */
void MaterialParamTexture::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MaterialParamTexture::
fillin(DatagramIterator &scan, BamReader *manager) {
  MaterialParamBase::fillin(scan, manager);

  manager->read_pointer(scan);
}

/**
 *
 */
TypedWritable *MaterialParamTexture::
make_from_bam(const FactoryParams &params) {
  MaterialParamTexture *param = new MaterialParamTexture("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  param->fillin(scan, manager);
  return param;
}
