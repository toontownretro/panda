/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderModule.h"

TypeHandle ShaderModule::_type_handle;

/**
 * Writes the contents of the Variable for shipping out to a Bam file.
 */
void ShaderModule::Variable::
write_datagram(Datagram &dg, BamWriter *manager) {
  manager->write_pointer(dg, type);
  dg.add_string(name->get_name());
  dg.add_int32(_location);
}

/**
 * Reads the contents from the Datagram to re-create the Variable from a Bam
 * file.
 */
void ShaderModule::Variable::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);
  name = InternalName::make(scan.get_string());
  _location = scan.get_int32();
}

/**
 * Writes the contents of the SpecializationConstant for shipping out to a Bam
 * file.
 */
void ShaderModule::SpecializationConstant::
write_datagram(Datagram &dg, BamWriter *manager) {
  manager->write_pointer(dg, type);
  dg.add_string(name->get_name());
  dg.add_uint32(id);
}

/**
 * Reads the contents from the Datagram to re-create the SpecializationConstant
 * from a Bam file.
 */
void ShaderModule::SpecializationConstant::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);
  name = InternalName::make(scan.get_string());
  id = scan.get_uint32();
}

/**
 *
 */
ShaderModule::
ShaderModule(Stage stage) : _stage(stage), _used_caps(C_basic_shader) {
  switch (stage) {
  case Stage::tess_control:
  case Stage::tess_evaluation:
    _used_caps |= C_tessellation_shader;
    break;

  case Stage::geometry:
    _used_caps |= C_geometry_shader;
    break;

  case Stage::compute:
    _used_caps |= C_compute_shader;
    break;

  default:
    break;
  }
}

/**
 *
 */
ShaderModule::
~ShaderModule() {
}

/**
 * Adjusts any input bindings necessary to be able to link up with the given
 * previous stage.  Should return false to indicate that the link is not
 * possible.
 */
bool ShaderModule::
link_inputs(const ShaderModule *previous) {
  // By default we need to do nothing special to link it up, as long as the
  // modules have the same type.
  return get_stage() > previous->get_stage()
      && get_type() == previous->get_type();
}

/**
 * Remaps parameters with a given location to a given other location.  Locations
 * not included in the map remain untouched.
 */
void ShaderModule::
remap_parameter_locations(pmap<int, int> &locations) {
}

/**
 *
 */
void ShaderModule::
output(std::ostream &out) const {
  out << get_type() << " " << get_stage();
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderModule::
write_datagram(BamWriter *manager, Datagram &dg) {
  CopyOnWriteObject::write_datagram(manager, dg);

  dg.add_uint8((int)_stage);
  dg.add_string(_source_filename);
  dg.add_uint32(_used_caps);

  dg.add_uint32(_inputs.size());
  for (size_t i = 0; i < _inputs.size(); i++) {
    _inputs[i].write_datagram(dg, manager);
  }

  dg.add_uint32(_outputs.size());
  for (size_t i = 0; i < _outputs.size(); i++) {
    _outputs[i].write_datagram(dg, manager);
  }

  dg.add_uint32(_parameters.size());
  for (size_t i = 0; i < _parameters.size(); i++) {
    _parameters[i].write_datagram(dg, manager);
  }

  dg.add_uint32(_spec_constants.size());
  for (size_t i = 0; i < _spec_constants.size(); i++) {
    _spec_constants[i].write_datagram(dg, manager);
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int ShaderModule::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = CopyOnWriteObject::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _inputs.size(); i++) {
    if (p_list[index] != nullptr) {
      _inputs[i].type = (const ShaderType *)p_list[index];
    }
    index++;
  }

  for (size_t i = 0; i < _outputs.size(); i++) {
    if (p_list[index] != nullptr) {
      _outputs[i].type = (const ShaderType *)p_list[index];
    }
    index++;
  }

  for (size_t i = 0; i < _parameters.size(); i++) {
    if (p_list[index] != nullptr) {
      _parameters[i].type = (const ShaderType *)p_list[index];
    }
    index++;
  }

  for (size_t i = 0; i < _spec_constants.size(); i++) {
    if (p_list[index] != nullptr) {
      _spec_constants[i].type = (const ShaderType *)p_list[index];
    }
    index++;
  }

  return index;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderModule.
 */
void ShaderModule::
fillin(DatagramIterator &scan, BamReader *manager) {
  CopyOnWriteObject::fillin(scan, manager);

  _stage = (Stage)scan.get_uint8();
  _source_filename = scan.get_string();
  _used_caps = scan.get_uint32();

  size_t num_inputs = scan.get_uint32();
  for (size_t i = 0; i < num_inputs; i++) {
    Variable var;
    var.fillin(scan, manager);
    _inputs.push_back(var);
  }

  size_t num_outputs = scan.get_uint32();
  for (size_t i = 0; i < num_outputs; i++) {
    Variable var;
    var.fillin(scan, manager);
    _outputs.push_back(var);
  }

  size_t num_params = scan.get_uint32();
  for (size_t i = 0; i < num_params; i++) {
    Variable var;
    var.fillin(scan, manager);
    _parameters.push_back(var);
  }

  size_t num_spec_consts = scan.get_uint32();
  for (size_t i = 0; i < num_spec_consts; i++) {
    SpecializationConstant spec_const;
    spec_const.fillin(scan, manager);
    _spec_constants.push_back(spec_const);
  }
}

/**
 * Returns the stage as a string.
 */
std::string ShaderModule::
format_stage(Stage stage) {
  switch (stage) {
  case Stage::vertex:
    return "vertex";
  case Stage::tess_control:
    return "tess_control";
  case Stage::tess_evaluation:
    return "tess_evaluation";
  case Stage::geometry:
    return "geometry";
  case Stage::fragment:
    return "fragment";
  case Stage::compute:
    return "compute";
  }

  return "**invalid**";
}

/**
 * Outputs the given capabilities mask.
 */
void ShaderModule::
output_capabilities(std::ostream &out, int caps) {
  if (caps & C_basic_shader) {
    out << "basic_shader ";
  }
  if (caps & C_vertex_texture) {
    out << "vertex_texture ";
  }
  if (caps & C_sampler_shadow) {
    out << "sampler_shadow ";
  }
  if (caps & C_invariant) {
    out << "invariant ";
  }
  if (caps & C_matrix_non_square) {
    out << "matrix_non_square ";
  }
  if (caps & C_integer) {
    out << "integer ";
  }
  if (caps & C_texture_lod) {
    out << "texture_lod ";
  }
  if (caps & C_texture_fetch) {
    out << "texture_fetch ";
  }
  if (caps & C_sampler_cube_shadow) {
    out << "sampler_cube_shadow ";
  }
  if (caps & C_vertex_id) {
    out << "vertex_id ";
  }
  if (caps & C_round_even) {
    out << "round_even ";
  }
  if (caps & C_instance_id) {
    out << "instance_id ";
  }
  if (caps & C_buffer_texture) {
    out << "buffer_texture ";
  }
  if (caps & C_geometry_shader) {
    out << "geometry_shader ";
  }
  if (caps & C_primitive_id) {
    out << "primitive_id ";
  }
  if (caps & C_bit_encoding) {
    out << "bit_encoding ";
  }
  if (caps & C_texture_gather) {
    out << "texture_gather ";
  }
  if (caps & C_double) {
    out << "double ";
  }
  if (caps & C_cube_map_array) {
    out << "cube_map_array ";
  }
  if (caps & C_tessellation_shader) {
    out << "tessellation_shader ";
  }
  if (caps & C_sample_variables) {
    out << "sample_variables ";
  }
  if (caps & C_extended_arithmetic) {
    out << "extended_arithmetic ";
  }
  if (caps & C_texture_query_lod) {
    out << "texture_query_lod ";
  }
  if (caps & C_image_load_store) {
    out << "image_load_store ";
  }
  if (caps & C_compute_shader) {
    out << "compute_shader ";
  }
  if (caps & C_texture_query_levels) {
    out << "texture_query_levels ";
  }
  if (caps & C_enhanced_layouts) {
    out << "enhanced_layouts ";
  }
  if (caps & C_derivative_control) {
    out << "derivative_control ";
  }
  if (caps & C_texture_query_samples) {
    out << "texture_query_samples ";
  }
}
