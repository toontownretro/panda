/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceGeometry.cxx
 * @author brian
 */

#include "rayTraceGeometry.h"
#include "rayTrace.h"
#include "rayTraceScene.h"
#include "embree3/rtcore.h"

IMPLEMENT_CLASS( RayTraceGeometry );

RayTraceGeometry::RayTraceGeometry( int type, const std::string &name ) :
        PandaNode( name )
{
        nassertv( RayTrace::get_device() != nullptr );
        _geometry = rtcNewGeometry( RayTrace::get_device(), (RTCGeometryType)type );
        // All bits on by default
        rtcSetGeometryMask( _geometry, BitMask32::all_on().get_word() );
        _geom_id = 0;
        _rtscene = nullptr;
        _last_trans = nullptr;

        set_cull_callback();


        raytrace_cat.debug()
                << "Made new RayTraceGeometry, type " << type << "\n";
}

RayTraceGeometry::~RayTraceGeometry()
{
        if ( _rtscene )
                _rtscene->remove_geometry( this );
        if ( _geometry )
                rtcReleaseGeometry( _geometry );
        _geometry = nullptr;
}

void RayTraceGeometry::update_rtc_transform( const TransformState *ts )
{
        if ( ts != _last_trans )
        {
                _last_trans = ts;

                const LMatrix4 &mat = _last_trans->get_mat();

                rtcSetGeometryTransform( _geometry, 0, RTC_FORMAT_FLOAT4X4_ROW_MAJOR, mat.get_data() );
                rtcCommitGeometry( _geometry );
                if ( _rtscene )
                        _rtscene->_scene_needs_rebuild = true;

                raytrace_cat.debug()
                        << "Updated geometry transform\n";
        }
}

void RayTraceGeometry::set_mask( unsigned int mask )
{
        nassertv( _geometry != nullptr );
        rtcSetGeometryMask( _geometry, mask );
        _mask = mask;
}

void RayTraceGeometry::set_build_quality( int quality )
{
        rtcSetGeometryBuildQuality( _geometry, (RTCBuildQuality)quality );
};
