/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelIndex.h
 * @author brian
 * @date 2021-03-04
 */

#ifndef MODELINDEX_H
#define MODELINDEX_H

#include "pandabase.h"
#include "filename.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "typedWritableReferenceCount.h"

class FactoryParams;
class Datagram;
class DatagramIterator;

/**
 * This is a global class that indexes into all of the assets of one or more
 * model trees.
 */
class EXPCL_PANDA_PUTIL ModelIndex {
PUBLISHED:
  class EXPCL_PANDA_PUTIL Asset : public ReferenceCount {
  public:
    std::string _name;
    Filename _src;
    Filename _built;

    void write_datagram(Datagram &dg);
    void read_datagram(DatagramIterator &dgi);
  };

  class EXPCL_PANDA_PUTIL AssetIndex : public ReferenceCount {
  public:
    std::string _type;

    typedef pmap<std::string, PT(ModelIndex::Asset)> Assets;
    Assets _assets;

    void write_datagram(Datagram &dg);
    void read_datagram(DatagramIterator &dgi);
  };

  class EXPCL_PANDA_PUTIL Tree : public TypedWritableReferenceCount {
  public:
    std::string _name;
    Filename _install_dir;
    Filename _src_dir;

    typedef pmap<std::string, PT(ModelIndex::AssetIndex)> AssetTypes;
    AssetTypes _asset_types;

  public:
    static void register_with_read_factory();
    virtual void write_datagram(BamWriter *manager, Datagram &dg);

  protected:
    static TypedWritable *make_from_bam(const FactoryParams &params);
    void fillin(DatagramIterator &scan, BamReader *manager);

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      TypedWritableReferenceCount::init_type();
      register_type(_type_handle, "ModelIndex::Tree",
                    TypedWritableReferenceCount::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };
  typedef pvector<PT(Tree)> Trees;

private:
  ModelIndex();

PUBLISHED:
  bool read_index(const Filename &filename);

  bool write_boo_index(int n, const Filename &filename);

  void read_config_trees();

  INLINE int get_num_trees() const;
  INLINE Tree *get_tree(int n) const;

  Asset *find_asset(const std::string &type, const std::string &name) const;

  static ModelIndex *get_global_ptr();

private:
  Trees _trees;

  static ModelIndex *_global_ptr;
};

#include "modelIndex.I"

#endif // MODELINDEX_H
