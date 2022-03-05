/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxSamplerContext11.h
 * @author brian
 * @date 2022-03-05
 */

#ifndef DXSAMPLERCONTEXT11_H
#define DXSAMPLERCONTEXT11_H

#include "pandabase.h"
#include "samplerContext.h"

class DXGraphicsStateGuardian11;

#include <d3d11.h>

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXSamplerContext11 : public SamplerContext {
public:
  DXSamplerContext11(const SamplerState &state, DXGraphicsStateGuardian11 *gsg);
  virtual ~DXSamplerContext11();

  INLINE ID3D11SamplerState *get_sampler_state() const;

private:
  ID3D11SamplerState *_sampler_state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SamplerContext::init_type();
    register_type(_type_handle, "DXSamplerContext11",
                  SamplerContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxSamplerContext11.I"

#endif // DXSAMPLERCONTEXT11_H
