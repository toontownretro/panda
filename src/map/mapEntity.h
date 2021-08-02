/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapEntity.h
 * @author brian
 * @date 2021-07-08
 */

#ifndef MAPENTITY_H
#define MAPENTITY_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pdxElement.h"

class FactoryParams;

/**
 * An entity associated with a particular map.
 */
class EXPCL_PANDA_MAP MapEntity : public TypedWritableReferenceCount {
  DECLARE_CLASS(MapEntity, TypedWritableReferenceCount);

PUBLISHED:
  /**
   *
   */
  //class Connection {
  //PUBLISHED:
  //  const std::string &get_output_name() const;

  //};

  MapEntity();

  INLINE void set_model_index(int index);
  INLINE int get_model_index() const;

  INLINE void set_class_name(const std::string &name);
  INLINE const std::string &get_class_name() const;

  INLINE void set_properties(PDXElement *properties);
  INLINE PDXElement *get_properties() const;

public:
  static void register_with_read_factory();

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;

private:
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  // The model/mesh that is associated with the entity.
  int _model_index;

  std::string _class_name;

  // Name/value entity properties.
  PT(PDXElement) _properties;
};

#include "mapEntity.I"

#endif // MAPENTITY_H
