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
#include "lightAttrib.h"

#include <algorithm>

//#ifdef HAVE_TBB
//#include <oneapi/tbb.h>
//#endif

TypeHandle CullBinStateSorted::_type_handle;

/**
 *
 */
CullBinStateSorted::
~CullBinStateSorted() {
  for (CullableObject *object : _objects) {
    delete object;
  }
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
add_object(CullableObject *object, Thread *current_thread) {
  if (object->_munged_data != nullptr) {
    object->_sort_data._format = object->_munged_data->get_format(current_thread);
  } else {
    object->_sort_data._format = nullptr;
  }
  _objects.push_back(object);
}

auto compare_objects_state = [](const CullableObject *a, const CullableObject *b) -> bool {
  // Group by state changes, in approximate order from heaviest change to
  // lightest change.
  const RenderState *sa = a->_state;
  const RenderState *sb = b->_state;

#if 1

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

#if 0
      if (sha != nullptr && shb != nullptr) {
        //if (!sha->_has_texture_inputs) {
        //  ((ShaderAttrib *)sha)->build_texture_inputs();
        //}
        //if (!shb->_has_texture_inputs) {
        //  ((ShaderAttrib *)shb)->build_texture_inputs();
        //}

        //std::cout << "num inputs: " << sha->_texture_inputs.size() << " vs " << shb->_texture_inputs.size() << "\n";

        // Compare texture inputs.
        if (sha->_texture_inputs.size() != shb->_texture_inputs.size()) {
          return sha->_texture_inputs.size() < shb->_texture_inputs.size();
        }

        ShaderAttrib::TextureInputs::const_iterator it = sha->_texture_inputs.begin();
        ShaderAttrib::TextureInputs::const_iterator it2 = shb->_texture_inputs.begin();
        for (; it != sha->_texture_inputs.end() && it2 != shb->_texture_inputs.end();) {
          //std::cout << (*it).first->get_name() << " vs " << (*it2).first->get_name() << "\n";
          //if ((*it).first != (*it2).first) {
          //  return (*it).first < (*it2).first;
          //}
          //std::cout << "tex " << (*it).second << " vs " << (*it2).second << "\n";
          if ((*it).second != (*it2).second) {
            return (*it).second < (*it2).second;
          }
          ++it;
          ++it2;
        }
      }
#endif
    }
  }

  if (sa != sb) {

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

  //std::cout << "format " << a._sort_data._format << " vs " << b._sort_data._format << "\n";

  // Vertex format changes are also fairly slow.
  if (a->_sort_data._format != b->_sort_data._format) {
    return a->_sort_data._format < b->_sort_data._format;
  }

  //std::cout << "vdata " << a._munged_data << " vs " << b._munged_data << "\n";

  // Prevent unnecessary vertex buffer rebinds.
  if (a->_munged_data != b->_munged_data) {
    return a->_munged_data < b->_munged_data;
  }

  if (sa != sb) {
    const RenderAttrib *ra = sa->get_attrib(ColorAttrib::get_class_slot());
    const RenderAttrib *rb = sb->get_attrib(ColorAttrib::get_class_slot());
    // Color attribs are a vertex attribute change.
    if (ra != rb) {
      return ra < rb;
    }

    ra = sa->get_attrib(LightAttrib::get_class_slot());
    rb = sb->get_attrib(LightAttrib::get_class_slot());
    // Lights require lots of glUniform calls.
    if (ra != rb) {
      return ra < rb;
    }
  }

  if (sha != shb) {
    return sha < shb;
  }

  // Uniform updates are actually pretty fast.
  if (a->_internal_transform != b->_internal_transform) {
    return a->_internal_transform < b->_internal_transform;
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

#else

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
  }

  int compare = sa->compare_sort(*sb);
  if (compare != 0) {
    return compare < 0;
  }

  // Vertex format changes are also fairly slow.
  if (a->_sort_data._format != b->_sort_data._format) {
    return a->_sort_data._format < b->_sort_data._format;
  }

  // Prevent unnecessary vertex buffer rebinds.
  if (a->_munged_data != b->_munged_data) {
    return a->_munged_data < b->_munged_data;
  }

  // Uniform updates are actually pretty fast.
  if (a->_internal_transform != b->_internal_transform) {
    return a->_internal_transform < b->_internal_transform;
  }
#endif

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
//#ifdef HAVE_TBB
//  oneapi::tbb::parallel_sort(_objects.begin(), _objects.end(), compare_objects_state);
//#else
  std::sort(_objects.begin(), _objects.end(), compare_objects_state);
//#endif
}


/**
 * Draws all the geoms in the bin, in the appropriate order.
 */
void CullBinStateSorted::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);
  _gsg->draw_objects(_objects, force, current_thread);
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
