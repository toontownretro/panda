/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interpolatedVariable.cxx
 * @author brian
 * @date 2021-05-03
 */

#include "interpolatedVariable.h"

ConfigVariableDouble iv_extrapolate_amount
("iv-extrapolate-amount", 0.25,
 PRC_DESC("Set how many seconds the client will extrapolate variables for."));

InterpolationContext *InterpolationContext::_head = nullptr;
bool InterpolationContext::_allow_extrapolation = false;
double InterpolationContext::_last_timestamp = 0;

template class InterpolatedVariable<float>;
template class InterpolatedVariable<LVecBase2f>;
template class InterpolatedVariable<LVecBase3f>;
template class InterpolatedVariable<LVecBase4f>;
template class InterpolatedVariable<LQuaternionf>;

template class InterpolatedVariable<double>;
template class InterpolatedVariable<LVecBase2d>;
template class InterpolatedVariable<LVecBase3d>;
template class InterpolatedVariable<LVecBase4d>;
template class InterpolatedVariable<LQuaterniond>;

template class InterpolatedVariable<int>;
