/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointTransformTable.h
 * @author theclashingfritz
 * @date 2025-03-18
 */
 
#ifndef JOINTTRANSFORMTABLE_H
#define JOINTTRANSFORMTABLE_H

#include "pandabase.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "luse.h"
#include "pipelineCycler.h"
#include "pointerTo.h"
#include "pvector.h"
#include "transformTable.h"
#include "weakPointerTo.h"

class Character;
class FactoryParams;

class EXPCL_PANDA_ANIM JointTransformTable : public TransformTable {
private:
  JointTransformTable();

PUBLISHED:
  JointTransformTable(Character *character);
  JointTransformTable(const TransformTable &copy, Character *character);
  JointTransformTable(const JointTransformTable &copy);
  void operator = (const JointTransformTable &copy);
  virtual ~JointTransformTable();
  
  INLINE static CPT(JointTransformTable) register_table(const JointTransformTable *table);
  
  INLINE const Character *get_character() const;
  MAKE_PROPERTY(character, get_character);
  
  INLINE void set_joint_count(int32_t count);
  INLINE const int32_t get_joint_count() const;
  MAKE_PROPERTY(joint_count, get_joint_count, set_joint_count);
  
  virtual LMatrix4f *get_transform_matrices(size_t num_matrices, Thread *current_thread = Thread::get_current_thread()) const;
  virtual LVecBase4f *get_transform_vectors(size_t num_vectors, Thread *current_thread = Thread::get_current_thread()) const;
  
  virtual void write(std::ostream &out) const;
  
private:
  WPT(Character) _char = nullptr;
  int32_t _joint_count = -1;
  
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
    TransformTable::init_type();
    register_type(_type_handle, "JointTransformTable",
                  TransformTable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Character;
  friend class VertexTransform;
};

#include "jointTransformTable.I"

#endif // JOINTTRANSFORMTABLE_H