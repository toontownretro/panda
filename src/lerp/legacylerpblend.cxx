// Filename: lerpblend.cxx
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

#include "legacylerpblend.h"

TypeHandle LegacyLerpBlendType::_type_handle;
TypeHandle LegacyEaseInBlendType::_type_handle;
TypeHandle LegacyEaseOutBlendType::_type_handle;
TypeHandle LegacyEaseInOutBlendType::_type_handle;
TypeHandle LegacyNoBlendType::_type_handle;

LegacyLerpBlendType::LegacyLerpBlendType(const LegacyLerpBlendType&) {}

LegacyLerpBlendType::~LegacyLerpBlendType() {}

LegacyLerpBlendType& LegacyLerpBlendType::operator=(const LegacyLerpBlendType&) {
  return *this;
}

PN_stdfloat LegacyLerpBlendType::operator()(PN_stdfloat t) {
  return t;
}

LegacyEaseInBlendType::LegacyEaseInBlendType(const LegacyEaseInBlendType& c) : LegacyLerpBlendType(c)
{
}

LegacyEaseInBlendType::~LegacyEaseInBlendType() {}

LegacyEaseInBlendType& LegacyEaseInBlendType::operator=(const LegacyEaseInBlendType& c) {
  LegacyLerpBlendType::operator=(c);
  return *this;
}

PN_stdfloat LegacyEaseInBlendType::operator()(PN_stdfloat t) {
  PN_stdfloat x = t*t;
  return ((3.0f * x) - (t * x)) * 0.5f;
}

LegacyEaseOutBlendType::LegacyEaseOutBlendType(const LegacyEaseOutBlendType& c)
  : LegacyLerpBlendType(c) {}

LegacyEaseOutBlendType::~LegacyEaseOutBlendType() {}

LegacyEaseOutBlendType& LegacyEaseOutBlendType::operator=(const LegacyEaseOutBlendType& c) {
  LegacyLerpBlendType::operator=(c);
  return *this;
}

PN_stdfloat LegacyEaseOutBlendType::operator()(PN_stdfloat t) {
  return ((3.0f * t) - (t * t * t)) * 0.5f;
}

LegacyEaseInOutBlendType::LegacyEaseInOutBlendType(const LegacyEaseInOutBlendType& c)
  : LegacyLerpBlendType(c) {}

LegacyEaseInOutBlendType::~LegacyEaseInOutBlendType() {}

LegacyEaseInOutBlendType& LegacyEaseInOutBlendType::operator=(const LegacyEaseInOutBlendType& c)
{
  LegacyLerpBlendType::operator=(c);
  return *this;
}

PN_stdfloat LegacyEaseInOutBlendType::operator()(PN_stdfloat t) {
  PN_stdfloat x = t*t;
  return (3.0f * x) - (2.0f * t * x);
}

LegacyNoBlendType::LegacyNoBlendType(const LegacyNoBlendType& c) : LegacyLerpBlendType(c) {}

LegacyNoBlendType::~LegacyNoBlendType() {}

LegacyNoBlendType& LegacyNoBlendType::operator=(const LegacyNoBlendType& c) {
  LegacyLerpBlendType::operator=(c);
  return *this;
}

PN_stdfloat LegacyNoBlendType::operator()(PN_stdfloat t) {
  return t;
}

