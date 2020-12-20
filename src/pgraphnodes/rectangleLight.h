/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rectangleLight.h
 * @author rdb
 * @date 2016-12-19
 */

#ifndef RECTANGLELIGHT_H
#define RECTANGLELIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"
#include "pointLight.h"

/**
 * This is a type of area light that is an axis aligned rectangle, pointing
 * along the Y axis in the positive direction.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_PGRAPHNODES RectangleLight : public LightLensNode {
PUBLISHED:
  explicit RectangleLight(const std::string &name);

protected:
  RectangleLight(const RectangleLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void write(std::ostream &out, int indent_level) const;

PUBLISHED:
  INLINE PN_stdfloat get_falloff() const final;
  INLINE void set_falloff(PN_stdfloat falloff);
  MAKE_PROPERTY(falloff, get_falloff, set_falloff);

  INLINE PN_stdfloat get_inner_radius() const final;
  INLINE void set_inner_radius(PN_stdfloat radius);
  MAKE_PROPERTY(inner_radius, get_inner_radius, set_inner_radius);

  INLINE PN_stdfloat get_outer_radius() const final;
  INLINE void set_outer_radius(PN_stdfloat radius);
  MAKE_PROPERTY(outer_radius, get_outer_radius, set_outer_radius);

  virtual int get_class_priority() const;

public:
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id);

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PGRAPHNODES CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return RectangleLight::get_class_type();
    }

    PN_stdfloat _falloff;
    PN_stdfloat _inner_radius;
    PN_stdfloat _outer_radius;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

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
    LightLensNode::init_type();
    register_type(_type_handle, "RectangleLight",
                  LightLensNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "rectangleLight.I"

#endif
