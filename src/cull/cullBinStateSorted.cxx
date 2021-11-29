/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinStateSorted.cxx
 * @author drose
 * @date 2005-03-22
 */

#include "cullBinStateSorted.h"
#include "graphicsStateGuardianBase.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"
#include "lightMutex.h"
#include "shaderAttrib.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "depthOffsetAttrib.h"
#include "transparencyAttrib.h"
#include "colorScaleAttrib.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "materialAttrib.h"

#include <algorithm>


TypeHandle CullBinStateSorted::_type_handle;

/**
 *
 */
CullBinStateSorted::
~CullBinStateSorted() {
}

/**
 * Factory constructor for passing to the CullBinManager.
 */
CullBin *CullBinStateSorted::
make_bin(const std::string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinStateSorted(name, gsg, draw_region_pcollector);
}

/**
 * Adds a geom, along with its associated state, to the bin for rendering.
 */
void CullBinStateSorted::
add_object(CullableObject &object, Thread *current_thread) {
  if (object._munged_data != nullptr) {
    object._sort_data._format = object._munged_data->get_format();
  } else {
    object._sort_data._format = nullptr;
  }
  _objects.emplace_back(std::move(object));
}

auto compare_objects_state = [](const CullableObject &a, const CullableObject &b) -> bool {
  // Group by state changes, in approximate order from heaviest change to
  // lightest change.
  const RenderState *sa = a._state;
  const RenderState *sb = b._state;

  const ShaderAttrib *sha = nullptr, *shb = nullptr;

  if (sa != sb) {

    if (sa->_generated_shader != nullptr) {
      sha = (const ShaderAttrib *)sa->_generated_shader.p();
    } else {
      sha = (const ShaderAttrib *)sa->get_attrib(ShaderAttrib::get_class_slot());
    }

    if (sb->_generated_shader != nullptr) {
      shb = (const ShaderAttrib *)sb->_generated_shader.p();
    } else {
      shb = (const ShaderAttrib *)sb->get_attrib(ShaderAttrib::get_class_slot());
    }

    if (sha != shb) {
      // Program changes are the heaviest.
      const Shader *shader_a = nullptr;
      const Shader *shader_b = nullptr;
      if (sha != nullptr) {
        shader_a = sha->get_shader();
      }
      if (shb != nullptr) {
        shader_b = shb->get_shader();
      }
      if (shader_a != shader_b) {
        return shader_a < shader_b;
      }
    }

    // TextureAttribs result in different generated ShaderAttribs with the
    // textures from the TextureAttrib.  They come second to programs in terms
    // of state change cost.

    const RenderAttrib *ra, *rb;

    ra = sa->get_attrib(TextureAttrib::get_class_slot());
    rb = sb->get_attrib(TextureAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }

    // Same goes for MaterialAttrib.
    ra = sa->get_attrib(MaterialAttrib::get_class_slot());
    rb = sb->get_attrib(MaterialAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }
  }

  // Vertex format changes are also fairly slow.
  if (a._sort_data._format != b._sort_data._format) {
    return a._sort_data._format < b._sort_data._format;
  }

  // Prevent unnecessary vertex buffer rebinds.
  if (a._munged_data != b._munged_data) {
    return a._munged_data < b._munged_data;
  }

  if (sa != sb) {
    const RenderAttrib *ra = sa->get_attrib(ColorAttrib::get_class_slot());
    const RenderAttrib *rb = sb->get_attrib(ColorAttrib::get_class_slot());
    // Color attribs are a vertex attribute change.
    if (ra != rb) {
      return ra < rb;
    }
  }

  if (sha != shb) {
    return sha < shb;
  }

  // Uniform updates are actually pretty fast.
  if (a._internal_transform != b._internal_transform) {
    return a._internal_transform < b._internal_transform;
  }

  if (sa != sb) {
    const RenderAttrib *ra, *rb;

    // Color scale is a uniform update.
    ra = sa->get_attrib(ColorScaleAttrib::get_class_slot());
    rb = sb->get_attrib(ColorScaleAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }

    // Now handle cheaper fixed-function attribs.
    ra = sa->get_attrib(TransparencyAttrib::get_class_slot());
    rb = sb->get_attrib(TransparencyAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }

    ra = sa->get_attrib(DepthWriteAttrib::get_class_slot());
    rb = sb->get_attrib(DepthWriteAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }

    ra = sa->get_attrib(DepthTestAttrib::get_class_slot());
    rb = sb->get_attrib(DepthTestAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }

    ra = sa->get_attrib(DepthOffsetAttrib::get_class_slot());
    rb = sb->get_attrib(DepthOffsetAttrib::get_class_slot());
    if (ra != rb) {
      return ra < rb;
    }
  }

  return false;
};

/**
 * Called after all the geoms have been added, this indicates that the cull
 * process is finished for this frame and gives the bins a chance to do any
 * post-processing (like sorting) before moving on to draw.
 */
void CullBinStateSorted::
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  std::sort(_objects.begin(), _objects.end(), compare_objects_state);
}


/**
 * Draws all the geoms in the bin, in the appropriate order.
 */
void CullBinStateSorted::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);
  _gsg->draw_objects(_objects, force);
}

/**
 * Called by CullBin::make_result_graph() to add all the geoms to the special
 * cull result scene graph.
 */
void CullBinStateSorted::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    builder.add_object(*oi);
  }
}
