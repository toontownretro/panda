/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceGeometry.h
 * @author lachbr
 */

#ifndef RAYTRACEGEOMETRY_H
#define RAYTRACEGEOMETRY_H

#include "config_raytrace.h"
#include "pandaNode.h"

class RayTraceScene;

class EXPCL_PANDA_RAYTRACE RayTraceGeometry : public PandaNode
{
        DECLARE_CLASS( RayTraceGeometry, PandaNode );

PUBLISHED:
        INLINE RayTraceGeometry( const std::string &name = "" ) :
                PandaNode( name ),
                _geometry( nullptr ),
                _geom_id( 0 ),
                _rtscene( nullptr ),
                _last_trans( nullptr )
        {
        }
        virtual ~RayTraceGeometry();

        INLINE void set_mask( const BitMask32 &mask )
        {
                set_mask( mask.get_word() );
        }
        void set_mask( unsigned int mask );

        INLINE BitMask32 get_mask() const
        {
                return _mask;
        }

        INLINE unsigned int get_geom_id() const
        {
                return _geom_id;
        }

        void set_build_quality( int quality );

        virtual void build() = 0;

public:
        RayTraceGeometry( int type, const std::string &name = "" );

        INLINE RTCGeometry get_geometry() const
        {
                return _geometry;
        }

        void update_rtc_transform( const TransformState *ts );

protected:
        RTCGeometry _geometry;
        unsigned int _geom_id;
        unsigned int _mask;
        RayTraceScene *_rtscene;

        CPT(TransformState) _last_trans;

        friend class RayTraceScene;
};

#endif // RAYTRACEGEOMETRY_H
