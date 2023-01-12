/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rayTraceTriangleMesh.h
 * @author brian
 */

#ifndef RAYTRACETRIANGLEMESH_H
#define RAYTRACETRIANGLEMESH_H

#include "config_raytrace.h"
#include "rayTraceGeometry.h"
#include "pvector.h"

class EXPCL_PANDA_RAYTRACE RayTraceTriangleMesh : public RayTraceGeometry
{
        DECLARE_CLASS( RayTraceTriangleMesh, RayTraceGeometry );

PUBLISHED:
        RayTraceTriangleMesh( const std::string &name = "" );

        void add_triangle( const LPoint3 &p1, const LPoint3 &p2, const LPoint3 &p3 );
        void add_triangles_from_geom( const Geom *geom, const TransformState *ts = nullptr );

        virtual void build();

private:
        struct Triangle
        {
                int v1, v2, v3;
        };
        pvector<LPoint3> _verts;
        pvector<Triangle> _tris;
};

#endif // RAYTRACETRIANGLEMESH_H
