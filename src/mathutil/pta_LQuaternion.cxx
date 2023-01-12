/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LQuaternion.cxx
 * @author brian
 * @date 2021-02-26
 */

#include "pta_LQuaternion.h"

template class PointerToBase<ReferenceCountedVector<LQuaternionf> >;
template class PointerToArrayBase<LQuaternionf>;
template class PointerToArray<LQuaternionf>;
template class ConstPointerToArray<LQuaternionf>;

template class PointerToBase<ReferenceCountedVector<LQuaterniond> >;
template class PointerToArrayBase<LQuaterniond>;
template class PointerToArray<LQuaterniond>;
template class ConstPointerToArray<LQuaterniond>;
