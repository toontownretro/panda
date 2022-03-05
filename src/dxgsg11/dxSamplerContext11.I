/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxSamplerContext11.I
 * @author brian
 * @date 2022-03-05
 */

/**
 *
 */
INLINE ID3D11SamplerState *DXSamplerContext11::
get_sampler_state() const {
  return _sampler_state;
}
