#pragma once

#include "postprocess/postprocess_effect.h"

class EXPCL_PANDABSP SSAO_Effect : public PostProcessEffect
{
	DECLARE_CLASS( SSAO_Effect, PostProcessEffect )

PUBLISHED:
	enum Mode
	{
		M_SSAO,
		M_HBAO
	};

	SSAO_Effect( PostProcess *pp, Mode mode );

	virtual Texture *get_final_texture();
};
