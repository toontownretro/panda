// Filename: eggPolygon.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGPOLYGON_H
#define EGGPOLYGON_H

#include <pandabase.h>

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggPolygon
// Description : A single polygon.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPolygon : public EggPrimitive {
public:
  INLINE EggPolygon(const string &name = "");
  INLINE EggPolygon(const EggPolygon &copy);
  INLINE EggPolygon &operator = (const EggPolygon &copy);

  virtual void cleanup();

  virtual void write(ostream &out, int indent_level) const;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPolygon",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
 
};

#include "eggPolygon.I"

#endif
