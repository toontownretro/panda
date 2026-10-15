// Filename: lerpblend.h
// Created by:  frang (30May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef __LERPBLEND_H__
#define __LERPBLEND_H__

#include "pandabase.h"
#include "typedReferenceCount.h"

class EXPCL_PANDA_LERP LegacyLerpBlendType : public TypedReferenceCount {
PUBLISHED:
  LegacyLerpBlendType() {}
  virtual ~LegacyLerpBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat) = 0;

public:
  LegacyLerpBlendType(const LegacyLerpBlendType&);
  LegacyLerpBlendType& operator=(const LegacyLerpBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LegacyLerpBlendType",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

class EXPCL_PANDA_LERP LegacyEaseInBlendType : public LegacyLerpBlendType {
PUBLISHED:
  LegacyEaseInBlendType() {}
  virtual ~LegacyEaseInBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);

public:
  LegacyEaseInBlendType(const LegacyEaseInBlendType&);
  LegacyEaseInBlendType& operator=(const LegacyEaseInBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LegacyEaseInBlendType",
                  LegacyLerpBlendType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

class EXPCL_PANDA_LERP LegacyEaseOutBlendType : public LegacyLerpBlendType {
PUBLISHED:
  LegacyEaseOutBlendType() {}
  virtual ~LegacyEaseOutBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);

public:
  LegacyEaseOutBlendType(const LegacyEaseOutBlendType&);
  LegacyEaseOutBlendType& operator=(const LegacyEaseOutBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LegacyEaseOutBlendType",
                  LegacyLerpBlendType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

class EXPCL_PANDA_LERP LegacyEaseInOutBlendType : public LegacyLerpBlendType {
PUBLISHED:
  LegacyEaseInOutBlendType() {}
  virtual ~LegacyEaseInOutBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);
public:
  LegacyEaseInOutBlendType(const LegacyEaseInOutBlendType&);
  LegacyEaseInOutBlendType& operator=(const LegacyEaseInOutBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LegacyEaseInOutBlendType",
                  LegacyLerpBlendType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

class EXPCL_PANDA_LERP LegacyNoBlendType : public LegacyLerpBlendType {
PUBLISHED:
  LegacyNoBlendType() {}
  virtual ~LegacyNoBlendType();
  virtual PN_stdfloat operator()(PN_stdfloat);
public:
  LegacyNoBlendType(const LegacyNoBlendType&);
  LegacyNoBlendType& operator=(const LegacyNoBlendType&);
public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "LegacyNoBlendType",
                  LegacyLerpBlendType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#endif /* __LERPBLEND_H__ */
