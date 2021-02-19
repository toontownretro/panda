/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelRoot.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef MODELROOT_H
#define MODELROOT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "modelNode.h"
#include "materialGroup.h"

/**
 * A node of this type is created automatically at the root of each model file
 * that is loaded.  It may eventually contain some information about the
 * contents of the model; at the moment, it contains no special information,
 * but can be used as a flag to indicate the presence of a loaded model file.
 */
class EXPCL_PANDA_PGRAPH ModelRoot : public ModelNode {
PUBLISHED:
  INLINE explicit ModelRoot(const std::string &name);
  INLINE explicit ModelRoot(const Filename &fullpath, time_t timestamp);

  INLINE int get_model_ref_count() const;
  MAKE_PROPERTY(model_ref_count, get_model_ref_count);

  INLINE const Filename &get_fullpath() const;
  INLINE void set_fullpath(const Filename &fullpath);
  MAKE_PROPERTY(fullpath, get_fullpath, set_fullpath);

  INLINE time_t get_timestamp() const;
  INLINE void set_timestamp(time_t timestamp);
  MAKE_PROPERTY(timestamp, get_timestamp, set_timestamp);

  // This class is used to unify references to the same model.
  class ModelReference : public ReferenceCount {
  PUBLISHED:
    INLINE ModelReference();
  };

  INLINE ModelReference *get_reference() const;
  void set_reference(ModelReference *ref);
  MAKE_PROPERTY(reference, get_reference, set_reference);

  INLINE void set_active_material_group(size_t n);
  INLINE size_t get_active_material_group() const;
  MAKE_PROPERTY(active_material_group, get_active_material_group, set_active_material_group);

  INLINE void add_material_group(MaterialGroup *group);
  INLINE size_t get_num_material_groups() const;
  INLINE MaterialGroup *get_material_group(size_t n) const;
  MAKE_SEQ(get_material_groups, get_num_material_groups, get_material_group);
  MAKE_SEQ_PROPERTY(material_groups, get_num_material_groups, get_material_group);

private:
  void r_set_active_material_group(PandaNode *node, size_t n);

protected:
  INLINE ModelRoot(const ModelRoot &copy);

public:
  virtual PandaNode *make_copy() const;

private:
  Filename _fullpath;
  time_t _timestamp;
  PT(ModelReference) _reference;

  // This handles skins.
  typedef pvector<PT(MaterialGroup)> MaterialGroups;
  MaterialGroups _material_groups;
  size_t _active_material_group;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ModelNode::init_type();
    register_type(_type_handle, "ModelRoot",
                  ModelNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "modelRoot.I"

#endif
