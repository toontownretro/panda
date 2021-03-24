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
#include "keyValues.h"
#include "bam.h"
#include "bamWriter.h"
#include "datagramOutputFile.h"

TypeHandle Material::_type_handle;

/**
 *
 */
Material::
Material(const std::string &name) :
  Namable(name),
  _num_params(0)
{
}

/**
 *
 */
void Material::
read_keyvalues(KeyValues *kv, const DSearchPath &search_path) {
  // Left up to derived materials.
}

/**
 *
 */
void Material::
write_keyvalues(KeyValues *kv, const Filename &filename) {
  for (size_t i = 0; i < _params.size(); i++) {
    const std::string &name = _params.get_key(i)->get_name();
    std::string value;
    _params.get_data(i)->to_string(value, filename);

    kv->set_key_value(name, value);
  }
}

/**
 * Writes this material to the indicated text-format .pmat file.
 */
void Material::
write_pmat(const Filename &filename) {
  PT(KeyValues) kv = new KeyValues;

  PT(KeyValues) mat_block = new KeyValues(get_type().get_name(), kv);
  write_keyvalues(mat_block, filename);

  kv->write(filename);
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
  me.add_string(get_name());

  me.add_uint8(_params.size());
  for (size_t i = 0; i < _params.size(); i++) {
    manager->write_pointer(me, _params.get_data(i));
  }
}

/**
 *
 */
int Material::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
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
  set_name(scan.get_string());

  _num_params = scan.get_uint8();
  manager->read_pointers(scan, _num_params);
}
