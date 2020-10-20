/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_misc.h
 * @author lachbr
 * @date 2020-10-19
 *
 * Miscellaneous math utility functions.
 */

#ifndef MATHUTIL_MISC_H
#define MATHUTIL_MISC_H

#include "config_mathutil.h"
#include "deg_2_rad.h"
#include "luse.h"

#define NORMAL_EPSILON 0.00001
#define ON_EPSILON 0.1 // we should ensure that (float)BOGUS_RANGE < (float)(BOGUA_RANGE + 0.2 * ON_EPSILON)
#define EQUAL_EPSILON 0.001
#define ON_LIGHTMAP_EPSILON ON_EPSILON / 16.0

#define OO_SQRT_2 0.70710676908493042f
#define OO_SQRT_3 0.57735025882720947f
#define OO_SQRT_6 0.40824821591377258f
// sqrt( 2 / 3 )
#define OO_SQRT_2_OVER_3 0.81649661064147949f

#define NUM_BUMP_VECTS 3

const LVector3 g_localbumpbasis[NUM_BUMP_VECTS] = {
  LVector3(OO_SQRT_2_OVER_3, 0.0f, OO_SQRT_3),
  LVector3(-OO_SQRT_6, OO_SQRT_2, OO_SQRT_3),
  LVector3(-OO_SQRT_6, -OO_SQRT_2, OO_SQRT_3)
};

//
// Vector Math
//

#define DotProduct(x, y) ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
#define CrossProduct(a, b, dest)                               \
        {                                                      \
                (dest)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
                (dest)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
                (dest)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]; \
        }

#define VectorMidpoint(a, b, c)                 \
        {                                       \
                (c)[0] = ((a)[0] + (b)[0]) / 2; \
                (c)[1] = ((a)[1] + (b)[1]) / 2; \
                (c)[2] = ((a)[2] + (b)[2]) / 2; \
        }

#define VectorFill(a, b)      \
        {                     \
                (a)[0] = (b); \
                (a)[1] = (b); \
                (a)[2] = (b); \
        }
#define VectorAvg(a) (((a)[0] + (a)[1] + (a)[2]) / 3)

#define VectorSubtract(a, b, c)           \
        {                                 \
                (c)[0] = (a)[0] - (b)[0]; \
                (c)[1] = (a)[1] - (b)[1]; \
                (c)[2] = (a)[2] - (b)[2]; \
        }
#define VectorAdd(a, b, c)                \
        {                                 \
                (c)[0] = (a)[0] + (b)[0]; \
                (c)[1] = (a)[1] + (b)[1]; \
                (c)[2] = (a)[2] + (b)[2]; \
        }
#define VectorMultiply(a, b, c)           \
        {                                 \
                (c)[0] = (a)[0] * (b)[0]; \
                (c)[1] = (a)[1] * (b)[1]; \
                (c)[2] = (a)[2] * (b)[2]; \
        }
#define VectorDivide(a, b, c)             \
        {                                 \
                (c)[0] = (a)[0] / (b)[0]; \
                (c)[1] = (a)[1] / (b)[1]; \
                (c)[2] = (a)[2] / (b)[2]; \
        }

#define VectorSubtractVec(a, b, c)     \
        {                              \
                (c)[0] = (a)[0] - (b); \
                (c)[1] = (a)[1] - (b); \
                (c)[2] = (a)[2] - (b); \
        }
#define VectorAddVec(a, b, c)          \
        {                              \
                (c)[0] = (a)[0] + (b); \
                (c)[1] = (a)[1] + (b); \
                (c)[2] = (a)[2] + (b); \
        }
#define VecSubtractVector(a, b, c)     \
        {                              \
                (c)[0] = (a) - (b)[0]; \
                (c)[1] = (a) - (b)[1]; \
                (c)[2] = (a) - (b)[2]; \
        }
#define VecAddVector(a, b, c)                                                \
        {                                                                    \
                (c)[0] = (a) + (b)[0]; (c)[1]=(a)[(b)[1]; (c)[2]=(a)+(b)[2]; \
        }

#define VectorMultiplyVec(a, b, c)     \
        {                              \
                (c)[0] = (a)[0] * (b); \
                (c)[1] = (a)[1] * (b); \
                (c)[2] = (a)[2] * (b); \
        }
#define VectorDivideVec(a, b, c)       \
        {                              \
                (c)[0] = (a)[0] / (b); \
                (c)[1] = (a)[1] / (b); \
                (c)[2] = (a)[2] / (b); \
        }

#define VectorScale(a, b, c)           \
        {                              \
                (c)[0] = (a)[0] * (b); \
                (c)[1] = (a)[1] * (b); \
                (c)[2] = (a)[2] * (b); \
        }

#define VectorCopy(a, b)         \
        {                        \
                (b)[0] = (a)[0]; \
                (b)[1] = (a)[1]; \
                (b)[2] = (a)[2]; \
        }
#define VectorClear(a)                          \
        {                                       \
                (a)[0] = (a)[1] = (a)[2] = 0.0; \
        }

#define VectorMaximum(a) (qmax((a)[0], qmax((a)[1], (a)[2])))
#define VectorMinimum(a) (qmin((a)[0], qmin((a)[1], (a)[2])))

#define VectorInverse(a)            \
        {                           \
                (a)[0] = -((a)[0]); \
                (a)[1] = -((a)[1]); \
                (a)[2] = -((a)[2]); \
        }
#define VectorRound(a) floor((a) + 0.5)
#define VectorMA(a, scale, b, dest)                    \
        {                                              \
                (dest)[0] = (a)[0] + (scale) * (b)[0]; \
                (dest)[1] = (a)[1] + (scale) * (b)[1]; \
                (dest)[2] = (a)[2] + (scale) * (b)[2]; \
        }
#define VectorLength(a) sqrt((double)((double)((a)[0] * (a)[0]) + (double)((a)[1] * (a)[1]) + (double)((a)[2] * (a)[2])))
#define VectorCompareMinimum(a, b, c)          \
        {                                      \
                (c)[0] = qmin((a)[0], (b)[0]); \
                (c)[1] = qmin((a)[1], (b)[1]); \
                (c)[2] = qmin((a)[2], (b)[2]); \
        }
#define VectorCompareMaximum(a, b, c)          \
        {                                      \
                (c)[0] = qmax((a)[0], (b)[0]); \
                (c)[1] = qmax((a)[1], (b)[1]); \
                (c)[2] = qmax((a)[2], (b)[2]); \
        }

INLINE vec_t VectorNormalize(vec3_t v);

INLINE bool VectorCompare(const float *v1, const float *v2);
INLINE bool VectorCompareD(const double *v1, const double *v2);

INLINE bool isPointFinite(const vec_t *p)
{
        if (finite(p[0]) && finite(p[1]) && finite(p[2]))
        {
                return true;
        }
        return false;
}


INLINE void VectorIRotate(const LVector3 &in1, const LMatrix3 &in2, LVector3 &out)
{
        out[0] = in1[0] * in2[0][0] + in1[1] * in2[1][0] + in1[2] * in2[2][0];
        out[1] = in1[0] * in2[0][1] + in1[1] * in2[1][1] + in1[2] * in2[2][1];
        out[2] = in1[0] * in2[0][2] + in1[1] * in2[1][2] + in1[2] * in2[2][2];
}

INLINE void VectorLerp(const LVector3 &src1, const LVector3 &src2, float t, LVector3 &dest)
{
        dest[0] = src1[0] + (src2[0] - src1[0]) * t;
        dest[1] = src1[1] + (src2[1] - src1[1]) * t;
        dest[2] = src1[2] + (src2[2] - src1[2]) * t;
}

INLINE void Vector2DLerp(const LVector2 &src1, const LVector2 &src2, float t, LVector2 &dest)
{
        dest[0] = src1[0] + (src2[0] - src1[0]) * t;
        dest[1] = src1[1] + (src2[1] - src1[1]) * t;
}

INLINE float DotProductAbs(const LVector3 &v0, const LVector3 &v1)
{
        return fabsf(v0[0] * v1[0]) +
               fabsf(v0[1] * v1[1]) +
               fabsf(v0[2] * v1[2]);
}

INLINE double DotProductAbsD(const LVector3 &v0, const double *v1)
{
        return fabs(v0[0] * v1[0]) +
               fabs(v0[1] * v1[1]) +
               fabs(v0[2] * v1[2]);
}

INLINE float DotProductAbs(const LVector3 &v0, const float *v1)
{
        return fabsf(v0[0] * v1[0]) +
               fabsf(v0[1] * v1[1]) +
               fabsf(v0[2] * v1[2]);
}

EXPCL_PANDA_MATHUTIL void GetBumpNormals(const LVector3 &svec, const LVector3 &tvec, const LVector3 &face_normal,
                               const LVector3 &phong_normal, LVector3 *bump_vecs);

INLINE LVector3 GetLVector3(const vec3_t &vec)
{
        return LVector3(vec[0], vec[1], vec[2]);
}

INLINE LVector3 GetLVector3_2(const float *vec)
{
        return LVector3(vec[0], vec[1], vec[2]);
}

INLINE void swap_floats(float &a, float &b);

template <class T>
INLINE T clamp(T const &val, T const &minVal, T const &maxVal);

INLINE float remap_val_clamped(float val, float A, float B, float C, float D);

// convert texture to linear 0..1 value
INLINE float tex_light_to_linear(int c, int exponent);

// maps a float to a byte fraction between min & max
INLINE unsigned char fixed_8_fraction(float t, float tMin, float tMax);

INLINE PN_stdfloat inv_r_squared(const LVector3 &v);

extern EXPCL_PANDA_MATHUTIL bool
solve_inverse_quadratic(float x1, float y1, float x2, float y2,
                        float x3, float y3, float &a, float &b, float &c);

extern EXPCL_PANDA_MATHUTIL bool
solve_inverse_quadratic_monotonic(float x1, float y1, float x2, float y2,
                                  float x3, float y3, float &a, float &b, float &c);

// Returns A + (B-A)*flPercent.
// float Lerp( float flPercent, float A, float B );
template <class T>
INLINE T tlerp(float percent, T const &A, T const &B);

INLINE float flerp(float f1, float f2, float t);
INLINE float flerp(float f1, float f2, float i1, float i2, float x);

// assuming the matrix is orthonormal, transform in1 by the transpose (also the inverse in this case) of in2.
INLINE void VectorITransform(const LVector3 &in1, const LMatrix4f &in2, LVector3 &out)
{
        float in1t[3];

        in1t[0] = in1[0] - in2[0][3];
        in1t[1] = in1[1] - in2[1][3];
        in1t[2] = in1[2] - in2[2][3];

        out[0] = in1t[0] * in2[0][0] + in1t[1] * in2[1][0] + in1t[2] * in2[2][0];
        out[1] = in1t[0] * in2[0][1] + in1t[1] * in2[1][1] + in1t[2] * in2[2][1];
        out[2] = in1t[0] * in2[0][2] + in1t[1] * in2[1][2] + in1t[2] * in2[2][2];
}

INLINE void AngleMatrix(const LVector3 &angles, LMatrix4f &matrix)
{
        float sr, sp, sy, cr, cp, cy;

        float rady = deg_2_rad(angles[YAW]);
        sy = std::sin(rady);
        cy = std::cos(rady);

        float radp = deg_2_rad(angles[PITCH]);
        sp = std::sin(radp);
        cp = std::cos(radp);

        float radr = deg_2_rad(angles[ROLL]);
        sr = std::sin(radr);
        cr = std::cos(radr);

        // matrix = (YAW * PITCH) * ROLL
        matrix[0][0] = cp * cy;
        matrix[1][0] = cp * sy;
        matrix[2][0] = -sp;

        float crcy = cr * cy;
        float crsy = cr * sy;
        float srcy = sr * cy;
        float srsy = sr * sy;
        matrix[0][1] = sp * srcy - crsy;
        matrix[1][1] = sp * srsy + crcy;
        matrix[2][1] = sr * cp;

        matrix[0][2] = (sp * crcy + srsy);
        matrix[1][2] = (sp * crsy - srcy);
        matrix[2][2] = cr * cp;

        matrix[0][3] = 0.0f;
        matrix[1][3] = 0.0f;
        matrix[2][3] = 0.0f;
}

INLINE void AngleMatrix(const LVector3 &angles, const LVector3 &position, LMatrix4f &matrix)
{
        AngleMatrix(angles, matrix);
        matrix.set_col(3, position);
}

INLINE int ceil_pow_2(int in);
INLINE int floor_pow_2(int in);

//
// Planetype Math
//

typedef enum
{
        plane_x = 0,
        plane_y,
        plane_z,
        plane_anyx,
        plane_anyy,
        plane_anyz
} planetypes;

#define last_axial plane_z
#define DIR_EPSILON 0.0001

#define MAX_COORD_INTEGER (16384)
#define COORD_EXTENT (2 * MAX_COORD_INTEGER)
#define MAX_TRACE_LENGTH (1.732050807569 * COORD_EXTENT)

inline planetypes PlaneTypeForNormal(vec3_t normal)
{
        vec_t ax, ay, az;

        ax = fabs(normal[0]);
        ay = fabs(normal[1]);
        az = fabs(normal[2]);
        if (ax > 1.0 - DIR_EPSILON && ay < DIR_EPSILON && az < DIR_EPSILON)
        {
                return plane_x;
        }

        if (ay > 1.0 - DIR_EPSILON && az < DIR_EPSILON && ax < DIR_EPSILON)
        {
                return plane_y;
        }

        if (az > 1.0 - DIR_EPSILON && ax < DIR_EPSILON && ay < DIR_EPSILON)
        {
                return plane_z;
        }

        if ((ax >= ay) && (ax >= az))
        {
                return plane_anyx;
        }
        if ((ay >= ax) && (ay >= az))
        {
                return plane_anyy;
        }
        return plane_anyz;
}

#include "mathutil_misc.I"
#include "mathutil_misc.T"

#endif // MATHUTIL_MISC_H
