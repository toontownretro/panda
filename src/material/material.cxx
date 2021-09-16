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

  me.add_uint8(_tags.size());
  for (size_t i = 0; i < _tags.size(); i++) {
    me.add_string(_tags[i]);
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

  _tags.resize(scan.get_uint8());
  for (size_t i = 0; i < _tags.size(); i++) {
    _tags[i] = scan.get_string();
  }
}
