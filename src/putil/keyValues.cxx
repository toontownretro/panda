/**
 * PANDA3D BSP LIBRARY
 *
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file keyValues.cxx
 * @author Brian Lach
 * @date 2020-04-15
 *
 * @desc The new and much cleaner interface for dealing with Valve's KeyValue
 *	     format. KeyValues can either be created procedurally or loaded from
 *	     disk.
 *
 * Parsing code is based on the implementation from
 * https://github.com/JulioC/keyvalues-python.
 *
 */

#include "keyValues.h"

#include "virtualFileSystem.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "tokenFile.h"

TypeHandle KeyValues::_type_handle;

static const std::string kv_symbols = "{}";

NotifyCategoryDeclNoExport(keyvalues);
NotifyCategoryDef(keyvalues, "");

char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A') {
    return in - ('Z' - 'z');
  }

  return in;
}

//------------------------------------------------------------------------------------------------

KeyValues::Pair *KeyValues::
find_pair(const std::string &key) {
  size_t count = _keyvalues.size();
  for (size_t i = 0; i < count; i++) {
    Pair *pair = &_keyvalues[i];
    if (pair->key == key) {
      return pair;
    }
  }

  return nullptr;
}

const KeyValues::Pair *KeyValues::
find_pair(const std::string &key) const {
  size_t count = _keyvalues.size();
  for (size_t i = 0; i < count; i++) {
    const Pair *pair = &_keyvalues[i];
    if (pair->key == key) {
      return pair;
    }
  }

  return nullptr;
}

const KeyValues::Pair *KeyValues::
get_pair(size_t n) const {
  return &_keyvalues[n];
}

int KeyValues::find_child(const std::string &name) const
{
  size_t count = _children.size();
  for (size_t i = 0; i < count; i++)
  {
    if (_children[i]->get_name() == name)
    {
      return (int)i;
    }
  }

  return -1;
}

pvector<KeyValues *>
KeyValues::get_children_with_name(const std::string &name) const
{
  pvector<KeyValues *> result;

  size_t count = _children.size();
  for (size_t i = 0; i < count; i++)
  {
    KeyValues *child = _children[i];
    if (child->get_name() == name)
    {
      result.push_back(child);
    }
  }

  return result;
}

void KeyValues::parse(TokenFile *tokens)
{
  bool has_key = false;
  std::string key;

  while (tokens->token_available(true))
  {
    tokens->next_token(true);

    std::string token = tokens->get_token();

    if (token == "}") {
      break;

    } else if (token == "{") {
      PT(KeyValues) child = new KeyValues(key, this);
      child->_filename = _filename;
      child->parse(tokens);
      has_key = false;

    } else {
      if (has_key) {
        add_key_value(key, token);
        key.clear();
        has_key = false;

      } else {
        key = token;
        has_key = true;
      }
    }
  }
}

/**
 * Loads a raw text KeyValues definition from the indicated filename and
 * returns a new KeyValues object representing the root of the KeyValues tree.
 */
PT(KeyValues)
KeyValues::load(const Filename &filename) {
  if (filename.empty()) {
    return nullptr;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename load_filename;
  if (filename.is_local()) {
    // Look along the model path for the file
    DSearchPath search_path(get_model_path());
    for (int i = 0; i < search_path.get_num_directories(); i++) {
      Filename search(search_path.get_directory(i), filename);
      if (vfs->exists(search)) {
        load_filename = search;
        break;
      }
    }

  } else {
    // This is an absolute filename. Use it as-is
    load_filename = filename;
  }

  if (load_filename.empty()) {
    keyvalues_cat.error() << "Unable to find `" << filename.get_fullpath()
                          << "`\n";
    return nullptr;
  }

  TokenFile tokens;
  tokens.local_object();
  tokens.set_symbols(kv_symbols);

  if (!tokens.read(load_filename)) {
    return nullptr;
  }

  PT(KeyValues) kv = new KeyValues;
  kv->parse(&tokens);
  kv->_filename = filename;

  return kv;
}

/**
 * Parses the indicated string and returns a new KeyValues object
 * representing the root of the KeyValues tree.
 */
PT(KeyValues) KeyValues::
from_string(const std::string &buffer) {
  std::istringstream iss = std::istringstream(buffer, std::ios::in);

  TokenFile tokens;
  tokens.local_object();
  tokens.set_symbols(kv_symbols);
  if (!tokens.tokenize(iss)) {
    return nullptr;
  }

  PT(KeyValues) kv = new KeyValues;
  kv->parse(&tokens);

  return kv;
}

//------------------------------------------------------------------------------------------------
// Helper functions for parsing string values that represent numbers.
//------------------------------------------------------------------------------------------------

vector_float KeyValues::parse_float_list(const std::string &str)
{
  vector_float result;
  std::string curr_num_string;
  int current = 0;
  while (current < str.length())
  {
    char let = str[current];
    if (let == ' ')
    {
      result.push_back(stof(curr_num_string));
      curr_num_string = "";
    }
    else
    {
      curr_num_string += let;
    }
    current++;

    if (current >= str.length())
    {
      result.push_back(stof(curr_num_string));
      curr_num_string = "";
    }
  }

  return result;
}

vector_int KeyValues::parse_int_list(const std::string &str)
{
  vector_int result;
  std::string curr_num_string;
  int current = 0;
  while (current < str.length())
  {
    char let = str[current];
    if (let == ' ')
    {
      result.push_back(stoi(curr_num_string));
      curr_num_string = "";
    }
    else
    {
      curr_num_string += let;
    }
    current++;

    if (current >= str.length())
    {
      result.push_back(stoi(curr_num_string));
      curr_num_string = "";
    }
  }

  return result;
}

pvector<vector_float>
KeyValues::
parse_float_tuple_list(const std::string &str) {
  pvector<vector_float> result;
  int current = 0;
  std::string curr_num_string;
  vector_float tuple_result;

  while (current < str.length()) {
    char let = str[current];
    if (let == '(' || (let == ' ' && str[current - 1] == ')')) {
      tuple_result.clear();

    } else if (let == ' ' || let == ')') {
      tuple_result.push_back(std::stof(curr_num_string));
      curr_num_string = "";
      if (let == ')') {
        result.push_back(tuple_result);
      }

    } else {
      curr_num_string += let;
    }
    current++;
  }

  return result;
}

void KeyValues::
parse_material_axis(const std::string &str, LVector3 &axis, LVector2 &shift_scale) {
  sscanf(str.c_str(), "[%f %f %f %f] %f", &axis[0], &axis[1], &axis[2], &shift_scale[0], &shift_scale[1]);
}

void KeyValues::
parse_plane_points(const std::string &str, LPoint3 &p0, LPoint3 &p1, LPoint3 &p2) {
  sscanf(str.c_str(), "(%f %f %f) (%f %f %f) (%f %f %f)",
    &p0[0], &p0[1], &p0[2],
    &p1[0], &p1[1], &p1[2],
    &p2[0], &p2[1], &p2[2]);
}

LVecBase2f KeyValues::
to_2f(const std::string &str) {
  vector_float vec = KeyValues::parse_float_list(str);
  LVecBase2f lvec;
  for (size_t i = 0; i < vec.size(); i++) {
      lvec[i] = vec[i];
  }
  return lvec;
}

LVecBase3f KeyValues::
to_3f(const std::string &str) {
  vector_float vec = KeyValues::parse_float_list(str);
  LVecBase3f lvec;
  for (size_t i = 0; i < vec.size(); i++) {
      lvec[i] = vec[i];
  }
  return lvec;
}

LVecBase4f KeyValues::
to_4f(const std::string &str) {
  vector_float vec = KeyValues::parse_float_list(str);
  LVecBase4f lvec;
  for (size_t i = 0; i < vec.size(); i++) {
      lvec[i] = vec[i];
  }
  return lvec;
}

std::string KeyValues::
to_string(const LVecBase2f &v) {
  std::ostringstream ss;
  ss << v[0] << " " << v[1];
  return ss.str();
}

std::string KeyValues::
to_string(const LVecBase3f &v) {
  std::ostringstream ss;
  ss << v[0] << " " << v[1] << " " << v[2];
  return ss.str();
}

std::string KeyValues::
to_string(const LVecBase4f &v) {
  std::ostringstream ss;
  ss << v[0] << " " << v[1] << " " << v[2] << " " << v[3];
  return ss.str();
}

void KeyValues::
write(const Filename &filename, int indent) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::ostringstream out;

  int curr_indent = 0;
  do_write(out, indent, curr_indent);

  vfs->write_file(filename, out.str(), false);
}

void KeyValues::
do_indent(std::ostringstream &out, int curr_indent) {
  int indents = curr_indent;
  for (int i = 0; i < indents; i++) {
    out << " ";
  }
}

void KeyValues::
do_write(std::ostringstream &out, int indent, int &curr_indent) {
  bool is_root = _name == root_block_name;

  // Don't write a block if we're the root; the root block is implicit.
  if (!is_root) {
    // Open the block
    do_indent(out, curr_indent);
    out << _name << "\n";
    do_indent(out, curr_indent);
    out << "{\n";
    curr_indent += indent;
  }

  // Write out keyvalues
  for (size_t i = 0; i < _keyvalues.size(); i++) {
    do_indent(out, curr_indent);
    out << "\"" << get_key(i) << "\"" << " " << "\"" << get_value(i) << "\"\n";
  }

  // Only put a line break after the keyvalues if we have child blocks.
  if (_children.size() != 0) {
    out << "\n";
  }

  // Now write the child blocks
  for (size_t i = 0; i < _children.size(); i++) {
    KeyValues *child = _children[i];
    child->do_write(out, indent, curr_indent);
    // Add an extra line break in between child blocks, but not
    // after the last child block.
    if (i != _children.size() - 1) {
      out << "\n";
    }
  }

  if (!is_root) {
    // Close the block
    curr_indent -= indent;
    do_indent(out, curr_indent);
    out << "}\n";
  }
}

/**
 * Tells the BamReader how to create objects of type KeyValues.
 */
void KeyValues::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void KeyValues::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  // Only write the parent if the parent has already been written to the file.
  // This allows for subtrees to be written to a binary.
  if (manager->has_object(_parent)) {
    manager->write_pointer(dg, _parent);
  } else {
    manager->write_pointer(dg, nullptr);
  }

  dg.add_string(_name);

  dg.add_uint16(_keyvalues.size());
  for (size_t i = 0; i < _keyvalues.size(); i++) {
    const Pair &pair = _keyvalues[i];
    dg.add_string(pair.key);
    dg.add_string(pair.value);
  }

  dg.add_uint8(_children.size());
  for (size_t i = 0; i < _children.size(); i++) {
    manager->write_pointer(dg, _children[i]);
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int KeyValues::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  if (p_list[index] != nullptr) {
    KeyValues *parent;
    DCAST_INTO_R(parent, p_list[index], index);
    _parent = parent;
  }
  index++;

  for (size_t i = 0; i < _children.size(); i++) {
    if (p_list[index] != nullptr) {
      KeyValues *child;
      DCAST_INTO_R(child, p_list[index], index);
      _children[i] = child;
    }
    index++;
  }

  return index;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type KeyValues is encountered in the Bam file.  It should create the
 * KeyValues and extract its information from the file.
 */
TypedWritable *KeyValues::
make_from_bam(const FactoryParams &params) {
  KeyValues *kv = new KeyValues;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  kv->fillin(scan, manager);

  return kv;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new KeyValues.
 */
void KeyValues::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  manager->read_pointer(scan);

  _name = scan.get_string();

  size_t num_pairs = scan.get_uint16();
  _keyvalues.resize(num_pairs);
  for (size_t i = 0; i < num_pairs; i++) {
    Pair &pair = _keyvalues[i];
    pair.key = scan.get_string();
    pair.value = scan.get_string();
  }

  int num_children = scan.get_uint8();
  _children.resize(num_children);
  manager->read_pointers(scan, num_children);
}
