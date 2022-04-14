/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_simd.cxx
 * @author brian
 * @date 2022-04-12
 */

#include "mathutil_simd.h"

#if defined(HAVE_SSE2)
#include "mathutil_sse_src.cxx"
#endif

#if defined(HAVE_AVX)
#include "mathutil_avx_src.cxx"
#endif

#if defined(HAVE_AVX512)
#include "mathutil_avx512_src.cxx"
#endif
