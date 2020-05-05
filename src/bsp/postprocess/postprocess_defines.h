/**
 * PANDA3D BSP LIBRARY
 * 
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file postprocess_defines.h
 * @author Brian Lach
 * @date July 22, 2019
 */

#ifndef POSTPROCESS_DEFINES_H
#define POSTPROCESS_DEFINES_H

#include <dtoolbase.h>

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

#endif // POSTPROCESS_DEFINES_H
