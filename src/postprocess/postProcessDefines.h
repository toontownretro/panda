/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessDefines.h
 * @author lachbr
 * @date 2019-07-22
 */

#ifndef POSTPROCESSDEFINES_H
#define POSTPROCESSDEFINES_H

#include "dtoolbase.h"

BEGIN_PUBLISH
enum AuxTextureBits
{
	AUXTEXTUREBITS_NORMAL   = 1,
        AUXTEXTUREBITS_ARME     = 2,
        AUXTEXTUREBITS_BLOOM    = 4,
};

enum AuxTextures
{
        AUXTEXTURE_NORMAL,
        AUXTEXTURE_ARME,
        AUXTEXTURE_BLOOM,

        AUXTEXTURE_COUNT = 4,
};
END_PUBLISH

#endif // POSTPROCESSDEFINES_H
