/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceScene.h
 * @author lachbr
 */

#ifndef RAYTRACESCENE_H
#define RAYTRACESCENE_H

#include "config_raytrace.h"
#include "referenceCount.h"
#include "luse.h"
#include "bitMask.h"
#include "simpleHashMap.h"
#include "rayTraceHitResult.h"
#include "rayTraceHitResult4.h"

class RayTraceGeometry;

class EXPCL_PANDA_RAYTRACE RayTraceScene : public ReferenceCount
{
PUBLISHED:
        RayTraceScene();
        ~RayTraceScene();

        void add_geometry( RayTraceGeometry *geom );
        void remove_geometry( RayTraceGeometry *geom );
        void remove_all();

        enum
        {
                BUILD_QUALITY_LOW       = 0,
                BUILD_QUALITY_MEDIUM    = 1,
                BUILD_QUALITY_HIGH      = 2,
        };

        INLINE RayTraceHitResult trace_line( const LPoint3 &start, const LPoint3 &end, const BitMask32 &mask )
        {
                LPoint3 delta = end - start;
                return trace_ray( start, delta.normalized(), delta.length(), mask );
        }
        RayTraceHitResult trace_ray( const LPoint3 &origin, const LVector3 &direction,
                float distance, const BitMask32 &mask );

        void set_build_quality( int quality );

        void update();

        INLINE RayTraceGeometry *get_geometry( unsigned int geom_id )
        {
                //if ( _geoms.find( geom_id ) == -1 )
                //{
                //        raytrace_cat.error()
                //                << "Geometry with ID " << geom_id << " does not exist\n";
                //        return nullptr;
                //}
                return _geoms[geom_id];
        }

public:
#if 0
#ifndef CPPPARSER
        INLINE void trace_four_lines( const FourVectors &start, const FourVectors &end,
                const u32x4 &mask, RayTraceHitResult4 *res )
        {
                FourVectors direction = end;
                direction -= start;
                fltx4 length4 = direction.length();
                direction.VectorNormalize();
                return trace_four_rays( start, direction, length4, mask, res );
        }
        void trace_four_rays( const FourVectors &origin, const FourVectors &direction,
                const fltx4 &distance, const u32x4 &mask, RayTraceHitResult4 *res );
#endif
#endif

private:
        RTCScene _scene;
        bool _scene_needs_rebuild;

        SimpleHashMap<unsigned int, RayTraceGeometry *, int_hash> _geoms;

        friend class RayTraceGeometry;
};

#endif // RAYTRACESCENE_H
