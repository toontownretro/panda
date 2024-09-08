/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderObject.cxx
 * @author brian
 * @date 2020-12-22
 */

#include "shaderObject.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "virtualFileSystem.h"
#include "virtualFile.h"
#include "config_shader.h"
#include "string_utils.h"
#include "internalName.h"
#include "shaderCompiler.h"
#include "shaderCompilerRegistry.h"

TypeHandle ShaderObject::_type_handle;

#define VARIABLE_PREFIX '$'
#define VARIABLE_OPEN_BRACE '['
#define VARIABLE_CLOSE_BRACE ']'
#define FUNCTION_PARAMETER_SEPARATOR ','

/**
 * Calculates the total number of possible combinations for combo variable
 * values.
 */
void ShaderObject::
calc_total_combos() {
  _total_combos = 1;
  for (int i = (int)_combos.size() - 1; i >= 0; --i) {
    Combo &combo = _combos[i];
    // The scale is used to calculate a permutation index from all the combo
    // values.
    combo.scale = _total_combos;
    _total_combos *= combo.max_val - combo.min_val + 1;
    _combos_by_name[combo.name.p()] = (int)i;
  }
}

/**
 * Tells the BamReader how to create objects of type ShaderObject.
 */
void ShaderObject::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderObject::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  // Write the combos.
  dg.add_uint32(_combos.size());
  for (size_t i = 0; i < _combos.size(); i++) {
    const Combo &combo = _combos[i];
    dg.add_string(combo.name->get_name());
    dg.add_int8(combo.min_val);
    dg.add_int8(combo.max_val);
  }

  // Write the permutations.
  dg.add_uint32(_permutations.size());
  for (size_t i = 0; i < _permutations.size(); i++) {
    manager->write_pointer(dg, _permutations[i].p());
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int ShaderObject::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _permutations.size(); i++) {
    if (p_list[index] != nullptr) {
      ShaderModule *mod;
      DCAST_INTO_R(mod, p_list[index], index);
      _permutations[i] = mod;
    }
    index++;
  }

  return index;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderObject is encountered in the Bam file.  It should create the
 * ShaderObject and extract its information from the file.
 */
TypedWritable *ShaderObject::
make_from_bam(const FactoryParams &params) {
  ShaderObject *sho = new ShaderObject;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  sho->fillin(scan, manager);

  return sho;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderObject.
 */
void ShaderObject::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  size_t num_combos = scan.get_uint32();
  for (size_t i = 0; i < num_combos; i++) {
    Combo combo;
    combo.name = InternalName::make(scan.get_string());
    combo.min_val = scan.get_int8();
    combo.max_val = scan.get_int8();
    _combos.push_back(combo);
  }

  size_t num_permutations = scan.get_uint32();
  _permutations.resize(num_permutations);
  for (size_t i = 0; i < num_permutations; i++) {
    manager->read_pointer(scan);
  }

  calc_total_combos();
}

static ShaderObject::SkipCommand r_expand_command(const std::string &str, size_t &vp);

/**
 *
 */
static ShaderObject::SkipCommand
r_expand_expression(const std::string &str, size_t p) {
  ShaderObject::SkipCommand cmd;

  std::string literal;
  // Search for the beginning of a command.
  while (p < str.length()) {
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      // Found a command.  Expand it.
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "command: " << str << "\n";
      }
      cmd = r_expand_command(str, p);

    } else {
      // Must just be a literal value.
      cmd.cmd = ShaderObject::SkipCommand::C_literal;
      literal += str[p];
      p++;
    }
  }

  if (cmd.cmd == ShaderObject::SkipCommand::C_literal) {
    bool ret = string_to_int(literal, cmd.value);
    if (!ret) {
      shadermgr_cat.error()
        << "Invalid literal integer: " << literal << "\n";
      assert(0);
    }
  }

  return cmd;
}

/**
 *
 */
static std::string
r_scan_variable(const std::string &str, size_t &vp) {

  // Search for the end of the variable name: an unmatched square
  // bracket.
  size_t start = vp;
  size_t p = vp + 2;
  while (p < str.length() && str[p] != VARIABLE_CLOSE_BRACE) {
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      // Here's a nested variable!  Scan past it, matching braces
      // properly.
      r_scan_variable(str, p);
    } else {
      p++;
    }
  }

  if (p < str.length()) {
    assert(str[p] == VARIABLE_CLOSE_BRACE);
    p++;
  } else {
    shadermgr_cat.warning()
      << "Unclosed variable reference:\n"
      << str.substr(vp) << "\n";
  }

  vp = p;
  return str.substr(start, vp - start);
}

/**
 * Tokenizes the function parameters, skipping nested variables/functions.
 */
static void
tokenize_params(const std::string &str, vector_string &tokens,
                bool expand) {
  size_t p = 0;
  while (p < str.length()) {
    // Skip initial whitespace.
    while (p < str.length() && isspace(str[p])) {
      p++;
    }

    std::string token;
    while (p < str.length() && str[p] != FUNCTION_PARAMETER_SEPARATOR) {
      if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
          str[p + 1] == VARIABLE_OPEN_BRACE) {
        // Skip a nested variable reference.
        token += r_scan_variable(str, p);
      } else {
        token += str[p];
        p++;
      }
    }

    // Back up past trailing whitespace.
    size_t q = token.length();
    while (q > 0 && isspace(token[q - 1])) {
      q--;
    }

    tokens.push_back(token.substr(0, q));
    p++;

    if (p == str.length()) {
      // In this case, we have just read past a trailing comma symbol
      // at the end of the string, so we have one more empty token.
      tokens.push_back(std::string());
    }
  }
}

/**
 *
 */
static ShaderObject::SkipCommand
r_expand_command(const std::string &str, size_t &vp) {
  ShaderObject::SkipCommand cmd;

  std::string varname;
  size_t whitespace_at = 0;

  size_t p = vp + 2;
  while (p < str.length() && str[p] != VARIABLE_CLOSE_BRACE) {
    if (p + 1 < str.length() && str[p] == VARIABLE_PREFIX &&
        str[p + 1] == VARIABLE_OPEN_BRACE) {
      if (whitespace_at == 0) {
        shadermgr_cat.error()
          << "Nested skip commands can only be function arguments.\n";
        assert(0);
      }

      varname += r_scan_variable(str, p);
    } else {
      if (whitespace_at == 0 && isspace(str[p])) {
        whitespace_at = p - (vp + 2);
      }
      varname += str[p];
      p++;
    }
  }

  if (p < str.length()) {
    assert(str[p] == VARIABLE_CLOSE_BRACE);
    p++;
  } else {
    shadermgr_cat.warning()
      << "Warning!  Unclosed variable reference:\n"
      << str.substr(vp) << "\n";
  }

  vp = p;

  // Check for a function expansion.
  if (whitespace_at != 0) {
    std::string funcname = varname.substr(0, whitespace_at);
    p = whitespace_at;
    while (p < varname.length() && isspace(varname[p])) {
      p++;
    }

    vector_string params;
    tokenize_params(varname.substr(p), params, false);

    if (funcname == "and") {
      cmd.cmd = ShaderObject::SkipCommand::C_and;
    } else if (funcname == "or") {
      cmd.cmd = ShaderObject::SkipCommand::C_or;
    } else if (funcname == "not") {
      cmd.cmd = ShaderObject::SkipCommand::C_not;
    } else if (funcname == "eq") {
      cmd.cmd = ShaderObject::SkipCommand::C_eq;
    } else if (funcname == "neq") {
      cmd.cmd = ShaderObject::SkipCommand::C_neq;
    } else {
      shadermgr_cat.error()
        << "Unknown skip function: " << funcname << "\n";
      assert(0);
    }

    for (size_t i = 0; i < params.size(); i++) {
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "param " << i << ": " << params[i] << "\n";
      }
      cmd.arguments.push_back(r_expand_expression(params[i], 0));
    }
  } else {
    // Not a function, must be a combo variable reference.
    cmd.cmd = ShaderObject::SkipCommand::C_ref;
    cmd.name = varname;
  }

  return cmd;
}

/**
 *
 */
static bool
collect_combos(ShaderObject *obj, const std::string &shader_source, const Filename &input_filename) {
  vector_string lines;
  tokenize(shader_source, lines, "\n");

  for (size_t i = 0; i < lines.size(); i++) {
    const std::string &line = lines[i];

    if (line.size() < 7) {
      // Can't contain combo.
      continue;
    }

    if (line.substr(0, 7) != "#pragma") {
      // Line doesn't start with #pragma, can't possibly be a combo definition.
      continue;
    }

    // Get everything after the #pragma.
    std::string combo_line = line.substr(7);
    vector_string combo_words;
    extract_words(combo_line, combo_words);

    if (combo_words.size() == 0) {
      // It's not a #pragma combo.
      continue;
    }

    if (combo_words[0] == "combo") {
      // It's a combo command.

      // Must contain four words: combo, name, min val, max val.
      if (combo_words.size() != 4) {
        shader_cat.error()
            << "Invalid combo definition at line " << i + 1 << " of "
            << input_filename.get_fullpath() << "\n";
        return false;
      }

      std::string name = combo_words[1];
      int min_val, max_val;
      if (!string_to_int(combo_words[2], min_val)) {
        shadermgr_cat.error()
          << "Invalid min combo value at line " << i + 1 << " of "
          << input_filename.get_fullpath() << "\n";
        return false;
      }
      if (!string_to_int(combo_words[3], max_val)) {
        shadermgr_cat.error()
          << "Invalid max combo value at line " << i + 1 << " of "
          << input_filename.get_fullpath() << "\n";
        return false;
      }

      ShaderObject::Combo combo;
      combo.name = InternalName::make(name);
      combo.min_val = min_val;
      combo.max_val = max_val;
      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "Found combo " << name << " with min value " << min_val
          << " and max value " << max_val << "\n";
      }
      obj->add_combo(std::move(combo));

    } else if (combo_words[0] == "skip") {
      // It's a skip command.  Everything after the skip is the expression.

      std::string expression;
      for (size_t i = 1; i < combo_words.size(); i++) {
        expression += combo_words[i];
        if (i < combo_words.size() - 1) {
          expression += " ";
        }
      }

      if (shadermgr_cat.is_debug()) {
        shadermgr_cat.debug()
          << "Skip expression: " << expression << "\n";
      }

      // Parse the expression to build up an actual skip command.
      obj->add_skip_command(r_expand_expression(expression, 0));
    }
  }

  return true;
}

/**
 * Returns a shader object by reading the indicated source file and populating the combo
 * definitions.  The variations will remain uncompiled until the user requests a
 * particular variation.
 *
 * Returns nullptr if the shader source file could not be read or parsed properly.
 */
ShaderObject *ShaderObject::
read_source(Shader::ShaderLanguage lang, ShaderModule::Stage stage, Filename filename, const DSearchPath &search_path) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vf = vfs->find_file(filename, search_path);
  if (vf == nullptr) {
    shadermgr_cat.error()
      << "Could not find shader source file: " << filename
      << " on search path " << search_path << "\n";
    return nullptr;
  }

  shadermgr_cat.info()
    << "Reading from-source shader object " << filename << "\n";

  std::string shader_source = vf->read_file(true);
  ShaderObject *obj = new ShaderObject;
  obj->_vfile = vf;
  obj->_lang = lang;
  obj->_stage = stage;
  if (!collect_combos(obj, shader_source, vf->get_filename())) {
    delete obj;
    return nullptr;
  }
  obj->calc_total_combos();
  obj->resize_permutations(obj->get_total_combos());

  return obj;
}

/**
 *
 */
int ShaderObject::SkipCommand::
eval(const ShaderObject::VariationBuilder &opts) const {
  switch (cmd) {

  case C_and: {
    // All of the arguments must evaluate to true.
    for (size_t i = 0; i < arguments.size(); i++) {
      if (!arguments[i].eval(opts)) {
        return 0;
      }
    }

    return 1;
  }

  case C_or: {
    // At least one argument must evaluate to true.
    for (size_t i = 0; i < arguments.size(); i++) {
      if (arguments[i].eval(opts)) {
        return 1;
      }
    }

    return 0;
  }

  case C_not: {
    return !arguments[0].eval(opts);
  }

  case C_eq: {
    return arguments[0].eval(opts) == arguments[1].eval(opts);
  }

  case C_neq: {
    return arguments[0].eval(opts) != arguments[1].eval(opts);
  }

  case C_literal: {
    return value;
  }

  case C_ref: {
    // Find the combo with this name and get the current value
    // of the builder.
    ShaderObject *obj = opts.get_object();
    int i = obj->get_combo_index(name);
    nassertr(i != -1, 0);
    return opts._combo_values[i];
  }

  default:
    return 0;
  }
}

/**
 * Returns the index of the shader module from the current set of
 * specified combo values.
 */
size_t ShaderObject::VariationBuilder::
get_module_index() const {
  size_t index = 0u;
  for (size_t i = 0; i < _combo_values.size(); ++i) {
    index += _obj->get_combo(i).scale * _combo_values[i];
  }
  return index;
}

/**
 *
 */
ShaderModule *ShaderObject::VariationBuilder::
get_module(bool compile_if_necessary) const {
  size_t index = get_module_index();
  ShaderModule *mod = _obj->get_permutation(index);
  if (mod == nullptr && compile_if_necessary) {
    // We haven't compiled this module yet, let's do it now.

    VirtualFile *vfile = _obj->get_virtual_file();
    if (vfile == nullptr) {
      return nullptr;
    }

    ShaderCompilerRegistry *reg = ShaderCompilerRegistry::get_global_ptr();
    ShaderCompiler *compiler = reg->get_compiler_from_language(_obj->get_shader_language());
    if (compiler == nullptr) {
      shadermgr_cat.error()
        << "No compiler available for " << vfile->get_filename()
        << " (language " << _obj->get_shader_language() << "\n";
      return nullptr;
    }

    if (shadermgr_cat.is_debug()) {
      shadermgr_cat.debug()
        << "Using compiler " << compiler->get_type() << "\n";
    }

    // Build out the set of #defines for this variation.
    ShaderCompiler::Options opts;
    for (size_t i = 0; i < _obj->get_num_combos(); ++i) {
      const Combo &combo = _obj->get_combo(i);
      opts.set_define(combo.name, _combo_values[i]);
    }

    IStreamWrapper wrap(vfile->open_read_file(true), true);
    // Compile it!
    PT(ShaderModule) cmod = compiler->compile_now(_obj->get_shader_stage(), *wrap.get_istream(), vfile->get_filename(), opts);
    if (cmod == nullptr) {
      shadermgr_cat.error()
        << "Failed to compile variation " << index << " for shader object " << vfile->get_filename() << "!\n";
      shadermgr_cat.error(false)
        << "\tCombo values:\n";
      for (size_t i = 0; i < _obj->get_num_combos(); ++i) {
        shadermgr_cat.error(false)
          << "\t\t" << _obj->get_combo(i).name->get_name() << " = " << _combo_values[i] << "\n";
      }
      return nullptr;
    }

    // Alright, module compiled.  Save it on the object for later
    // or for writing to disk.
    _obj->set_permutation(index, cmod);
    mod = cmod;
  }
  return mod;
}
