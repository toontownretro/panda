/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceTriangleMesh.cxx
 * @author brian
 */

#include "rayTraceTriangleMesh.h"
#include "rtcore.h"
#include "geomVertexReader.h"
#include "geomVertexData.h"

IMPLEMENT_CLASS( RayTraceTriangleMesh );

RayTraceTriangleMesh::RayTraceTriangleMesh( const std::string &name) :
        RayTraceGeometry( RTC_GEOMETRY_TYPE_TRIANGLE, name )
{
}

void RayTraceTriangleMesh::add_triangle( const LPoint3 &p1, const LPoint3 &p2, const LPoint3 &p3 )
{
        nassertv( _geometry != nullptr );
        int idx = (int)_verts.size();
        _verts.push_back( p1 );
        _verts.push_back( p2 );
        _verts.push_back( p3 );
        Triangle tri;
        tri.v1 = idx;
        tri.v2 = idx + 1;
        tri.v3 = idx + 2;
        _tris.push_back( tri );

        raytrace_cat.debug()
                << "Added triangle [" << p1 << ", " << p2 << ", " << p3 << "]\n";
}

void RayTraceTriangleMesh::add_triangles_from_geom( const Geom *geom, const TransformState *ts )
{
        if ( ts == nullptr )
                ts = TransformState::make_identity();

        const LMatrix4 &mat = ts->get_mat();

        PT( Geom ) dgeom = geom->decompose();

        nassertv( dgeom->get_primitive_type() == Geom::PT_polygons );

        const GeomVertexData *vdata = dgeom->get_vertex_data();
        GeomVertexReader reader( vdata, InternalName::get_vertex() );

        for ( int i = 0; i < dgeom->get_num_primitives(); i++ )
        {
                const GeomPrimitive *prim = dgeom->get_primitive( i );

                nassertv( prim->get_num_vertices_per_primitive() == 3 );

                for ( int j = 0; j < prim->get_num_primitives(); j++ )
                {
                        int start = prim->get_primitive_start( j );

                        reader.set_row( prim->get_vertex( start ) );
                        LPoint3 p1 = reader.get_data3f();
                        reader.set_row( prim->get_vertex( start + 1 ) );
                        LPoint3 p2 = reader.get_data3f();
                        reader.set_row( prim->get_vertex( start + 2 ) );
                        LPoint3 p3 = reader.get_data3f();

                        add_triangle( mat.xform_point( p1 ), mat.xform_point( p2 ), mat.xform_point( p3 ) );
                }
        }
}

void RayTraceTriangleMesh::build()
{
        LPoint4f *vertices = (LPoint4f *)rtcSetNewGeometryBuffer( _geometry, RTC_BUFFER_TYPE_VERTEX, 0,
                                                                  RTC_FORMAT_FLOAT3, sizeof( LPoint4f ),
                                                                  _verts.size() );
        raytrace_cat.debug()
                << "build(): vertex buffer: " << vertices << "\n";
        for ( size_t i = 0; i < _verts.size(); i++ )
        {
                vertices[i] = LPoint4f( _verts[i], 0 );
        }

        Triangle *tris = (Triangle *)rtcSetNewGeometryBuffer( _geometry, RTC_BUFFER_TYPE_INDEX, 0,
                                                              RTC_FORMAT_UINT3, sizeof( Triangle ), _tris.size() );
        raytrace_cat.debug()
                << "build(): triangle buffer: " << tris << "\n";
        for ( size_t i = 0; i < _tris.size(); i++ )
        {
                tris[i] = _tris[i];
        }

        rtcCommitGeometry( _geometry );

        raytrace_cat.debug()
                << "Built triangle mesh to embree\n";
}
