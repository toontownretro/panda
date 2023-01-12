/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LQuaternion_ext.h
 * @author brian
 * @date 2021-02-26
 */

#ifndef PTA_LQUATERNION_EXT_H
#define PTA_LQUATERNION_EXT_H

#include "pointerToArray_ext.h"
#include "pta_LQuaternion.h"

#if defined(_MSC_VER) && !defined(CPPPARSER)
template class EXPORT_THIS Extension<PTA_LQuaternionf>;
template class EXPORT_THIS Extension<PTA_LQuaterniond>;
template class EXPORT_THIS Extension<CPTA_LQuaternionf>;
template class EXPORT_THIS Extension<CPTA_LQuaterniond>;
#endif

#endif // PTA_LQUATERNION_EXT_H
