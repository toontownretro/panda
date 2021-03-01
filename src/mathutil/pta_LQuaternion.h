/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LQuaternion.h
 * @author lachbr
 * @date 2021-02-26
 */

#ifndef PTA_LQUATERNION_H
#define PTA_LQUATERNION_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

/**
 * A pta of LQuaterionfs.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LQuaternionf> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LQuaternionf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LQuaternionf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LQuaternionf>)

typedef PointerToArray<LQuaternionf> PTA_LQuaternionf;
typedef ConstPointerToArray<LQuaternionf> CPTA_LQuaternionf;

/**
 * A pta of LQuaternionds.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LQuaterniond> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LQuaterniond>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LQuaterniond>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LQuaterniond>)

typedef PointerToArray<LQuaterniond> PTA_LQuaterniond;
typedef ConstPointerToArray<LQuaterniond> CPTA_LQuaterniond;

#ifndef STDFLOAT_DOUBLE
typedef PTA_LQuaternionf PTA_LQuaternion;
typedef CPTA_LQuaternionf CPTA_LQuaternion;
#else
typedef PTA_LQuaterniond PTA_LQuaternion;
typedef CPTA_LQuaterniond CPTA_LQuaternion;
#endif  // STDFLOAT_DOUBLE

#endif // PTA_LQUATERNION_H
