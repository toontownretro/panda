// Filename: config_gobj.cxx
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_util.h"
#include "boundedObject.h"
#include "config_gobj.h"
#include "drawable.h"
#include "geom.h"
#include "geomMunger.h"
#include "geomPrimitive.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomLines.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "geomVertexArrayData.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "material.h"
#include "orthographicLens.h"
#include "matrixLens.h"
#include "perspectiveLens.h"
#include "lens.h"
#include "sliderTable.h"
#include "texture.h"
#include "textureStage.h"
#include "textureContext.h"
#include "shader.h"
#include "shaderContext.h"
#include "transformBlendTable.h"
#include "transformTable.h"
#include "userVertexSlider.h"
#include "userVertexTransform.h"
#include "vertexTransform.h"
#include "vertexSlider.h"
#include "videoTexture.h"
#include "geomContext.h"
#include "vertexBufferContext.h"
#include "indexBufferContext.h"
#include "internalName.h"

#include "dconfig.h"
#include "string_utils.h"

Configure(config_gobj);
NotifyCategoryDef(gobj, "");

ConfigVariableInt max_texture_dimension
("max-texture-dimension", -1,
 PRC_DESC("Set this to the maximum size a texture is allowed to be in either "
          "dimension.  This is generally intended as a simple way to restrict "
          "texture sizes for limited graphics cards.  When this is greater "
          "than zero, each texture image loaded from a file (but only those "
          "loaded from a file) will be automatically scaled down, if "
          "necessary, so that neither dimension is larger than this value."));

ConfigVariableBool keep_texture_ram
("keep-texture-ram", false,
 PRC_DESC("Set this to true to retain the ram image for each texture after it "
          "has been prepared with the GSG.  This will allow the texture to be "
          "prepared with multiple GSG's, or to be re-prepared later after it is "
          "explicitly released from the GSG, without having to reread the "
          "texture image from disk; but it will consume memory somewhat "
          "wastefully."));

ConfigVariableBool vertex_buffers
("vertex-buffers", true,
 PRC_DESC("Set this true to allow the use of vertex buffers (or buffer "
          "objects, as OpenGL dubs them) for rendering vertex data.  This "
          "can greatly improve rendering performance on "
          "higher-end graphics cards, at the cost of some additional "
          "graphics memory (which might otherwise be used for textures "
          "or offscreen buffers).  On lower-end graphics cards this will "
          "make little or no difference."));

ConfigVariableBool vertex_arrays
("vertex-arrays", true,
 PRC_DESC("Set this true to allow the use of vertex arrays for rendering "
          "OpenGL vertex data.  This, or vertex buffers, is the normal "
          "way of issuing vertices ever since OpenGL 1.1, and you "
          "almost always want to have this set to true.  However, some very "
          "buggy graphics drivers may have problems handling vertex arrays "
          "correctly, so if you are experiencing problems you might try "
          "setting this to false.  If this is false, Panda will fall back "
          "to using immediate-mode commands like glVertex3f(), etc., to "
          "issue the vertices, which is potentially much slower than "
          "vertex arrays.  Setting this false also disables vertex buffers, "
	  "effectively ignoring the setting of the vertex-buffers variable "
	  "(since vertex buffers are a special case of vertex arrays in "
	  "OpenGL).  This variable is normally not enabled in a production "
	  "build.  This has no effect on DirectX rendering."));

ConfigVariableBool display_lists
("display-lists", false,
 PRC_DESC("Set this true to allow the use of OpenGL display lists for "
          "rendering static geometry.  On some systems, this can result "
          "in a performance improvement over vertex buffers alone; on "
          "other systems (particularly low-end systems) it makes little to "
          "no difference.  On some systems, using display lists can actually "
          "reduce performance.  This has no effect on DirectX rendering or "
          "on dynamic geometry (e.g. soft-skinned animation)."));

ConfigVariableBool hardware_animated_vertices
("hardware-animated-vertices", false,
 PRC_DESC("Set this true to allow the transforming of soft-skinned "
          "animated vertices via hardware, if supported, or false always "
          "to perform the vertex animation via software within Panda.  "
          "If you have a card that supports this, and your scene does "
          "not contain too many vertices already, this can provide a "
          "performance boost by offloading some work from your CPU onto "
          "your graphics card.  It may also help by reducing the bandwidth "
          "necessary on your computer's bus.  However, in some cases it "
          "may actually reduce performance."));

ConfigVariableBool hardware_point_sprites
("hardware-point-sprites", true,
 PRC_DESC("Set this true to allow the use of hardware extensions when "
          "rendering perspective-scaled points and point sprites.  When "
          "false, these large points are always simulated via quads "
          "computed in software, even if the hardware claims it can "
          "support them directly."));

ConfigVariableBool matrix_palette
("matrix-palette", false,
 PRC_DESC("Set this true to allow the use of the matrix palette when "
          "animating vertices in hardware.  The matrix palette is "
          "not supported by all devices, but if it is, using "
          "it can allow animation of more sophisticated meshes "
          "in hardware, and it can also improve the "
          "performance of animating some simpler meshes.  Without "
          "this option, certain meshes will have to be animated in "
          "software.  However, this option is not enabled by default, "
          "because its support seems to be buggy in certain drivers "
          "(ATI FireGL T2 8.103 in particular.)"));

ConfigVariableBool display_list_animation
("display-list-animation", false,
 PRC_DESC("Set this true to allow the use of OpenGL display lists for "
          "rendering animated geometry (when the geometry is animated "
          "by the hardware).  This is not on by default because there "
          "appear to be some driver issues with this on my FireGL T2, "
          "but it should be perfectly doable in principle, and might get "
          "you a small performance boost."));

ConfigVariableBool connect_triangle_strips
("connect-triangle-strips", true,
 PRC_DESC("Set this true to send a batch of triangle strips to the graphics "
          "card as one long triangle strip, connected by degenerate "
          "triangles, or false to send them as separate triangle strips "
          "with no degenerate triangles.  On PC hardware, using one long "
          "triangle strip may help performance by reducing the number "
          "of separate graphics calls that have to be made."));

ConfigVariableEnum<BamTextureMode> bam_texture_mode
("bam-texture-mode", BTM_relative,
 PRC_DESC("Set this to specify how textures should be written into Bam files."
          "See the panda source or documentation for available options."));

ConfigVariableEnum<AutoTextureScale> textures_power_2
("textures-power-2", ATS_down,
 PRC_DESC("Specify whether textures should automatically be constrained to "
          "dimensions which are a power of 2 when they are loaded from "
          "disk.  Set this to 'none' to disable this feature, or to "
          "'down' or 'up' to scale down or up to the nearest power of 2, "
          "respectively.  This only has effect on textures which are not "
          "already a power of 2."));

ConfigVariableEnum<AutoTextureScale> textures_square
("textures-square", ATS_none,
 PRC_DESC("Specify whether textures should automatically be constrained to "
          "a square aspect ratio when they are loaded from disk.  Set this "
          "to 'none', 'down', or 'up'.  See textures-power-2."));

ConfigVariableInt geom_cache_size
("geom-cache-size", 5000,
 PRC_DESC("Specifies the maximum number of entries in the cache "
          "for storing pre-processed data for rendering "
          "vertices.  This limit is flexible, and may be "
          "temporarily exceeded if many different Geoms are "
          "pre-processed during the space of a single frame."));

ConfigVariableInt geom_cache_min_frames
("geom-cache-min-frames", 1,
 PRC_DESC("Specifies the minimum number of frames any one particular "
          "object will remain in the geom cache, even if geom-cache-size "
          "is exceeded."));

ConfigVariableDouble default_near
("default-near", 1.0,
 PRC_DESC("The default near clipping distance for all cameras."));

ConfigVariableDouble default_far
("default-far", 1000.0,
 PRC_DESC("The default far clipping distance for all cameras."));

ConfigVariableDouble default_fov
("default-fov", 40.0,
 PRC_DESC("The default field of view in degrees for all cameras."));


ConfigVariableDouble default_keystone
("default-keystone", 0.0f,
 PRC_DESC("The default keystone correction, as an x y pair, for all cameras."));



ConfigureFn(config_gobj) {
  BoundedObject::init_type();
  Geom::init_type();
  GeomMunger::init_type();
  GeomPrimitive::init_type();
  GeomTriangles::init_type();
  GeomTristrips::init_type();
  GeomTrifans::init_type();
  GeomLines::init_type();
  GeomLinestrips::init_type();
  GeomPoints::init_type();
  GeomVertexArrayData::init_type();
  GeomVertexArrayFormat::init_type();
  GeomVertexData::init_type();
  GeomVertexFormat::init_type();
  TextureContext::init_type();
  GeomContext::init_type();
  VertexBufferContext::init_type();
  IndexBufferContext::init_type();
  Material::init_type();
  OrthographicLens::init_type();
  MatrixLens::init_type();
  PerspectiveLens::init_type();
  Lens::init_type();
  SliderTable::init_type();
  Texture::init_type();
  dDrawable::init_type();
  TextureStage::init_type();
  Shader::init_type();
  ShaderContext::init_type();
  TransformBlendTable::init_type();
  TransformTable::init_type();
  UserVertexSlider::init_type();
  UserVertexTransform::init_type();
  VertexTransform::init_type();
  VertexSlider::init_type();
  VideoTexture::init_type();
  InternalName::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  Geom::register_with_read_factory();
  GeomTriangles::register_with_read_factory();
  GeomTristrips::register_with_read_factory();
  GeomTrifans::register_with_read_factory();
  GeomLines::register_with_read_factory();
  GeomLinestrips::register_with_read_factory();
  GeomPoints::register_with_read_factory();
  GeomVertexArrayData::register_with_read_factory();
  GeomVertexArrayFormat::register_with_read_factory();
  GeomVertexData::register_with_read_factory();
  GeomVertexFormat::register_with_read_factory();
  Material::register_with_read_factory();
  OrthographicLens::register_with_read_factory();
  MatrixLens::register_with_read_factory();
  PerspectiveLens::register_with_read_factory();
  SliderTable::register_with_read_factory();
  Shader::register_with_read_factory();
  Texture::register_with_read_factory();
  TextureStage::register_with_read_factory();
  TransformBlendTable::register_with_read_factory();
  TransformTable::register_with_read_factory();
  UserVertexSlider::register_with_read_factory();
  UserVertexTransform::register_with_read_factory();
  InternalName::register_with_read_factory();
}

ostream &
operator << (ostream &out, BamTextureMode btm) {
  switch (btm) {
  case BTM_unchanged:
    return out << "unchanged";
   
  case BTM_fullpath:
    return out << "fullpath";
    
  case BTM_relative:
    return out << "relative";
    
  case BTM_basename:
    return out << "basename";

  case BTM_rawdata:
    return out << "rawdata";
  }

  return out << "**invalid BamTextureMode (" << (int)btm << ")**";
}

istream &
operator >> (istream &in, BamTextureMode &btm) {
  string word;
  in >> word;

  if (cmp_nocase(word, "unchanged") == 0) {
    btm = BTM_unchanged;
  } else if (cmp_nocase(word, "fullpath") == 0) {
    btm = BTM_fullpath;
  } else if (cmp_nocase(word, "relative") == 0) {
    btm = BTM_relative;
  } else if (cmp_nocase(word, "basename") == 0) {
    btm = BTM_basename;
  } else if (cmp_nocase(word, "rawdata") == 0) {
    btm = BTM_rawdata;

  } else {
    gobj_cat->error() << "Invalid BamTextureMode value: " << word << "\n";
    btm = BTM_relative;
  }

  return in;
}

ostream &
operator << (ostream &out, AutoTextureScale ats) {
  switch (ats) {
  case ATS_none:
    return out << "none";
   
  case ATS_down:
    return out << "down";
    
  case ATS_up:
    return out << "up";
  }

  return out << "**invalid AutoTextureScale (" << (int)ats << ")**";
}

istream &
operator >> (istream &in, AutoTextureScale &ats) {
  string word;
  in >> word;

  if (cmp_nocase(word, "none") == 0 ||
      cmp_nocase(word, "0") == 0 ||
      cmp_nocase(word, "#f") == 0 ||
      tolower(word[0] == 'f')) {
    ats = ATS_none;

  } else if (cmp_nocase(word, "down") == 0 ||
          cmp_nocase(word, "1") == 0 ||
          cmp_nocase(word, "#t") == 0 ||
          tolower(word[0] == 't')) {
    ats = ATS_down;

  } else if (cmp_nocase(word, "up") == 0) {
    ats = ATS_up;

  } else {
    gobj_cat->error() << "Invalid AutoTextureScale value: " << word << "\n";
    ats = ATS_none;
  }

  return in;
}
