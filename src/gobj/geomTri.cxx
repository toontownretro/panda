// Filename: geomTri.cxx
// Created by:  charles (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#include <ioPtaDatagramShort.h>
#include <ioPtaDatagramInt.h>
#include <ioPtaDatagramLinMath.h>
#include <graphicsStateGuardianBase.h>

#include "geomTri.h"

TypeHandle GeomTri::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomTri::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomTri::
make_copy() const {
  return new GeomTri(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTri::print_draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomTri::
print_draw_immediate(void) const
{
  /*
    int i;
    int j;
    int nprims = _numprims;
    Vertexf* tcoords = _coords;
    Normalf* tnorms = _norms;
    Colorf* tcolors = _colors;
    TexCoordf* ttexcoords = _texcoords;
    ushort* tvindex = _vindex;
    ushort* tnindex = _nindex;
    ushort* tcindex = _cindex;
    ushort* ttindex = _tindex;

    // Draw overall
    if ( _color_command[G_OVERALL] != _issue_color_noop )
    {
	nout << "Color (Overall): "; 	
	if ( tcindex ) 
	{
	    nout << "idx: " << *tcindex << " ";
	    nout << tcolors[*(tcindex++)];
	}
	else
	    nout << *(tcolors++);
	nout << endl;
    }
    if ( _normal_command[G_OVERALL] != _issue_normal_noop )
    {   
        nout << "Normal (Overall): ";    
        if ( tnindex )                    
        {
            nout << "idx: " << *tnindex << " ";
	    nout << tnorms[*(tnindex++)];
        }
        else
	    nout << *(tnorms++);
        nout << endl;
    }

    nout << "BeginGfx()" << endl;

    for ( i = nprims; i > 0; i-- )
    {
        // Draw per primitive
    	if ( _color_command[G_PER_PRIM] != _issue_color_noop )
    	{
            nout << "Color (Per Prim): ";
            if ( tcindex )
            {
            	nout << "idx: " << *tcindex << " ";
	        nout << tcolors[*(tcindex++)];
            }
            else
            	nout << *(tcolors++);
            nout << endl;
    	}
    	if ( _normal_command[G_PER_PRIM] != _issue_normal_noop )
    	{
            nout << "Normal (Per Prim): ";
            if ( tnindex )
            {
            	nout << "idx: " << *tnindex << " ";
            	nout << tnorms[*(tnindex++)];
            }
            else
            	nout << *(tnorms++);
            nout << endl;
    	}

        for ( j = 0; j < 3; j++ )
        {
            // Draw per vertex
            if ( _color_command[G_PER_VERTEX] != _issue_color_noop )
            {
            	nout << "Color (Per Vertex): ";
            	if ( tcindex )
            	{
                    nout << "idx: " << *tcindex << " ";
                    nout << tcolors[*(tcindex++)];
                }
            	else
                    nout << *(tcolors++);
            	nout << endl;
            }
            if ( _normal_command[G_PER_VERTEX] != _issue_normal_noop )
            {
                nout << "Normal (Per Vertex): ";
                if ( tnindex )
                {
                    nout << "idx: " << *tnindex << " ";
                    nout << tnorms[*(tnindex++)];
                }
                else
                    nout << *(tnorms++);
                nout << endl;
            }
            if ( _texcoord_command[G_PER_VERTEX] != _issue_texcoord_noop )
            {
                nout << "TexCoord (Per Vertex): ";
                if ( ttindex )
                {
                    nout << "idx: " << *ttindex << " ";
                    nout << ttexcoords[*(ttindex++)];
                }
                else
                    nout << *(ttexcoords++);
                nout << endl;
            }
            if ( _vertex_command[G_PER_VERTEX] != _issue_vertex_noop )
            {
                nout << "Vertex (Per Vertex): ";
                if ( tvindex )
                {
                    nout << "idx: " << *tvindex << " ";
                    nout << tcoords[*(tvindex++)];
                }
                else
                    nout << *(tcoords++);
                nout << endl;
            }
        }
    }

    nout << "EndGfx()" << endl;
    */
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTri::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomTri::
draw_immediate(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_tri(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTri::get_tris
//       Access: Public, Virtual
//  Description: This is similar in principle to explode(), except it
//               returns only a list of triangle vertex indices, with
//               no information about color or whatever.  The array
//               returned is a set of indices into the geom's _coords
//               array, as retrieve by get_coords(); there will be 3*n
//               elements in the array, where n is the number of
//               triangles described by the geometry.  This is useful
//               when it's important to determine the physical
//               structure of the geometry, without necessarily
//               worrying about its rendering properties, and when
//               performance considerations are not overwhelming.
////////////////////////////////////////////////////////////////////
PTA_ushort GeomTri::
get_tris() const {
  int num_tris = _numprims;
  PTA_ushort tris;
  tris.reserve(num_tris * 3);

  int k = 0;

  for (int i = 0; i < _numprims; i++) {
    ushort indices[3];
    if (_vindex.empty()) {
      for (int j = 0; j < 3; j++) {
	tris.push_back(k++);
      }
    } else {
      for (int j = 0; j < 3; j++) {
	tris.push_back(_vindex[k++]);
      }
    }
  }

  nassertr(tris.size() == num_tris * 3, PTA_ushort());
  return tris;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTri::make_GeomTri
//       Access: Protected
//  Description: Factory method to generate a GeomTri object
////////////////////////////////////////////////////////////////////
TypedWriteable* GeomTri::
make_GeomTri(const FactoryParams &params) {
  GeomTri *me = new GeomTri;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTri::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomTri object
////////////////////////////////////////////////////////////////////
void GeomTri::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomTri);
}
