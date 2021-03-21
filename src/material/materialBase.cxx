/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialBase.cxx
 * @author lachbr
 * @date 2021-03-06
 */

#include "materialBase.h"
#include "config_material.h"
#include "virtualFileSystem.h"
#include "keyValues.h"
#include "bam.h"
#include "bamWriter.h"
#include "datagramOutputFile.h"

TypeHandle MaterialBase::_type_handle;

/**
 *
 */
MaterialBase::
MaterialBase(const std::string &name) :
  Namable(name),
  _num_params(0)
{
}

/**
 *
 */
void MaterialBase::
read_keyvalues(KeyValues *kv, const DSearchPath &search_path) {
  // Left up to derived materials.
}

/**
 *
 */
void MaterialBase::
write_keyvalues(KeyValues *kv, const Filename &filename) {
  Params::const_iterator pi;
  for (pi = _params.begin(); pi != _params.end(); ++pi) {
    const std::string &name = (*pi).first->get_name();
    std::string value;
    (*pi).second->to_string(value, filename);

    kv->set_key_value(name, value);
  }
}

/**
 * Writes this material to the indicated text-format .pmat file.
 */
void MaterialBase::
write_pmat(const Filename &filename) {
  PT(KeyValues) kv = new KeyValues;

  PT(KeyValues) mat_block = new KeyValues(get_type().get_name(), kv);
  write_keyvalues(mat_block, filename);

  kv->write(filename);
}

/**
 * Writes this material to the indicated Bam-format .mto file.
 */
bool MaterialBase::
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
void MaterialBase::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());

  me.add_uint8(_params.size());
  for (Params::const_iterator pi = _params.begin();
       pi != _params.end(); ++pi) {
    manager->write_pointer(me, (*pi).second);
  }
}

/**
 *
 */
int MaterialBase::
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
void MaterialBase::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  _num_params = scan.get_uint8();
  manager->read_pointers(scan, _num_params);
}
