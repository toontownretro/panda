/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelIndex.cxx
 * @author lachbr
 * @date 2021-03-04
 */

#include "modelIndex.h"
#include "keyValues.h"
#include "config_putil.h"
#include "executionEnvironment.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "virtualFileSystem.h"
#include "bam.h"

ModelIndex *ModelIndex::_global_ptr = nullptr;

TypeHandle ModelIndex::Tree::_type_handle;

/**
 *
 */
void ModelIndex::Asset::
write_datagram(Datagram &dg) {
  dg.add_string(_name);
  dg.add_string(_src);
  dg.add_string(_built);
}

/**
 *
 */
void ModelIndex::Asset::
read_datagram(DatagramIterator &dgi) {
  _name = dgi.get_string();
  _src = dgi.get_string();
  _built = dgi.get_string();
}

/**
 *
 */
void ModelIndex::AssetIndex::
write_datagram(Datagram &dg) {
  dg.add_string(_type);
  dg.add_uint32(_assets.size());
  for (auto it = _assets.begin(); it != _assets.end(); ++it) {
    (*it).second->write_datagram(dg);
  }
}

/**
 *
 */
void ModelIndex::AssetIndex::
read_datagram(DatagramIterator &dgi) {
  _type = dgi.get_string();
  size_t num_assets = dgi.get_uint32();
  for (size_t i = 0; i < num_assets; i++) {
    PT(ModelIndex::Asset) asset = new ModelIndex::Asset;
    asset->read_datagram(dgi);
    _assets[asset->_name] = asset;
  }
}

/**
 *
 */
void ModelIndex::Tree::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void ModelIndex::Tree::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  dg.add_string(_name);
  dg.add_string(_src_dir);
  dg.add_string(_install_dir);

  dg.add_uint8(_asset_types.size());
  for (auto it = _asset_types.begin(); it != _asset_types.end(); ++it) {
    (*it).second->write_datagram(dg);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type KeyValues is encountered in the Bam file.  It should create the
 * KeyValues and extract its information from the file.
 */
TypedWritable *ModelIndex::Tree::
make_from_bam(const FactoryParams &params) {
  Tree *tree = new Tree;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  tree->fillin(scan, manager);

  return tree;
}

/**
 *
 */
void ModelIndex::Tree::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  _name = scan.get_string();
  _src_dir = scan.get_string();
  _install_dir = scan.get_string();

  uint8_t num_asset_types = scan.get_uint8();
  for (uint8_t i = 0; i < num_asset_types; i++) {
    PT(ModelIndex::AssetIndex) indx = new ModelIndex::AssetIndex;
    indx->read_datagram(scan);

    _asset_types[indx->_type] = indx;
  }
}

/**
 *
 */
ModelIndex::
ModelIndex() {
}

/**
 * Reads in the indicated index file for a single model tree.
 */
bool ModelIndex::
read_index(const Filename &filename) {
  if (filename.get_extension() == "boo") {
    // Compiled version.

    DatagramInputFile din;
    if (!din.open(filename)) {
      return false;
    }

    std::string head;
    if (!din.read_header(head, _bam_header.size())) {
      return false;
    }

    if (head != _bam_header) {
      return false;
    }

    BamReader reader(&din);
    if (!reader.init()) {
      din.close();
      return false;
    }

    TypedWritable *obj = reader.read_object();

    if (obj == nullptr || !reader.resolve()) {
      din.close();
      return false;
    }

    if (!obj->is_of_type(Tree::get_class_type())) {
      return false;
      din.close();
    }

    PT(Tree) tree = DCAST(Tree, obj);
    _trees.push_back(tree);

    din.close();

    return true;
  }

  PT(KeyValues) kv = KeyValues::load(filename);
  if (kv == nullptr) {
    return false;
  }

  PT(Tree) tree = new Tree;
  tree->_name = kv->get_value("tree");
  tree->_src_dir = kv->get_value("src_dir");
  tree->_install_dir = kv->get_value("install_dir");

  // Each child is an asset type.
  for (size_t i = 0; i < kv->get_num_children(); i++) {
    KeyValues *child = kv->get_child(i);

    PT(AssetIndex) index = new AssetIndex;
    index->_type = child->get_name();

    // Each child is an asset entry of this type.
    for (size_t j = 0; j < child->get_num_children(); j++) {
      KeyValues *child2 = child->get_child(j);

      PT(Asset) asset = new Asset;
      asset->_name = child2->get_name();
      asset->_src = child2->get_value("src");
      asset->_built = child2->get_value("built");

      index->_assets[asset->_name] = asset;
    }

    tree->_asset_types[index->_type] = index;
  }

  _trees.push_back(tree);

  return true;
}

/**
 * Writes the indicated tree to a .boo file.
 */
bool ModelIndex::
write_boo_index(int n, const Filename &filename) {
  Tree *tree = _trees[n];

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

  if (!writer.write_object(tree)) {
    return false;
  }

  dout.close();

  return true;
}

/**
 * Reads in all model tree index files requested through the config variable.
 */
void ModelIndex::
read_config_trees() {
  for (size_t i = 0; i < model_index.get_num_unique_values(); i++) {
    Filename filename = ExecutionEnvironment::expand_string(model_index.get_unique_value(i));
    read_index(filename);
  }
}

/**
 * Searches all of the model trees for the indicated asset of the indicated
 * type.
 */
ModelIndex::Asset *ModelIndex::
find_asset(const std::string &type, const std::string &name) const {
  if (_trees.empty()) {
    return nullptr;
  }

  // Search the trees in reverse order so we favor trees lower in the
  // hierarchy, eg $TTMODELS over $DMODELS.
  for (auto it = _trees.rbegin(); it != _trees.rend(); ++it) {
    Tree *tree = *it;

    auto iit = tree->_asset_types.find(type);
    if (iit == tree->_asset_types.end()) {
      continue;
    }

    AssetIndex *index = (*iit).second;

    auto ait = index->_assets.find(name);
    if (ait == index->_assets.end()) {
      continue;
    }

    // Found it!
    return (*ait).second;
  }

  return nullptr;
}

/**
 *
 */
ModelIndex *ModelIndex::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new ModelIndex;
    _global_ptr->read_config_trees();
  }

  return _global_ptr;
}
