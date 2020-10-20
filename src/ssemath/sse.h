//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=====================================================================================//

#ifndef _SSE_H
#define _SSE_H

#include "config_ssemath.h"

#include "luse.h"

extern EXPCL_PANDA_SSEMATH float _SSE_Sqrt( float x );
extern EXPCL_PANDA_SSEMATH float _SSE_RSqrtAccurate( float a );
extern EXPCL_PANDA_SSEMATH float _SSE_RSqrtFast( float x );
extern EXPCL_PANDA_SSEMATH float FASTCALL _SSE_VectorNormalize( LVector3& vec );
extern EXPCL_PANDA_SSEMATH void FASTCALL _SSE_VectorNormalizeFast( LVector3& vec );
extern EXPCL_PANDA_SSEMATH float _SSE_InvRSquared( const float* v );
extern EXPCL_PANDA_SSEMATH void _SSE_SinCos( float x, float* s, float* c );
extern EXPCL_PANDA_SSEMATH float _SSE_cos( float x );
extern EXPCL_PANDA_SSEMATH void _SSE2_SinCos( float x, float* s, float* c );
extern EXPCL_PANDA_SSEMATH float _SSE2_cos( float x );
extern EXPCL_PANDA_SSEMATH void VectorTransformSSE( const float *in1, const LMatrix4& in2, float *out1 );
extern EXPCL_PANDA_SSEMATH void VectorRotateSSE( const float *in1, const LMatrix4& in2, float *out1 );

#endif // _SSE_H
