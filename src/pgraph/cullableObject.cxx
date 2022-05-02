/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullableObject.cxx
 * @author drose
 * @date 2002-03-04
 */

#include "cullableObject.h"
#include "nodePath.h"
#include "colorAttrib.h"
#include "texGenAttrib.h"
#include "shaderAttrib.h"
#include "renderState.h"
#include "clockObject.h"
#include "cullTraverser.h"
#include "sceneSetup.h"
#include "lens.h"
#include "stateMunger.h"
#include "pStatTimer.h"
#include "geomVertexWriter.h"
#include "geomVertexReader.h"
#include "geomTriangles.h"
#include "light.h"
#include "lightMutexHolder.h"
#include "thread.h"

PStatCollector CullableObject::_munge_pcollector("*:Munge");
PStatCollector CullableObject::_munge_geom_pcollector("*:Munge:Geom");

TypeHandle CullableObject::_type_handle;

/**
 * Uses the indicated GeomMunger to transform the geom and/or its vertices.
 *
 * If force is false, this may do nothing and return false if the vertex data
 * is nonresident.  If force is true, this will always return true, but it may
 * have to block while the vertex data is paged in.
 */
bool CullableObject::
munge_geom(GraphicsStateGuardianBase *gsg, GeomMunger *munger,
           const CullTraverser *traverser, bool force) {
  //nassertr(munger != nullptr, false);

  Thread *current_thread;
  if (traverser != nullptr) {
    current_thread = traverser->get_current_thread();
  } else {
    current_thread = Thread::get_current_thread();
  }

  PStatTimer timer(_munge_pcollector, current_thread);

  if (_geom != nullptr) {
    //GraphicsStateGuardianBase *gsg = traverser->get_gsg();

    const GeomVertexAnimationSpec &anim_spec = _munged_data->get_format()->get_animation();

    // If there is any animation left in the vertex data after it has been
    // munged--that is, we couldn't arrange to handle the animation in
    // hardware--then we have to calculate that animation now.
    bool cpu_animated = false;

    if (anim_spec.get_animation_type() != GeomEnums::AT_none) {
      const GeomVertexData *animated_vertices =
        _munged_data->animate_vertices(force, current_thread);
      if (animated_vertices != _munged_data) {
        cpu_animated = true;
        std::swap(_munged_data, animated_vertices);
      }
    }

#ifndef NDEBUG
    if (show_vertex_animation) {
      GeomVertexDataPipelineReader data_reader(_munged_data, current_thread);
      bool hardware_animated = (data_reader.get_format()->get_animation().get_animation_type() == Geom::AT_hardware);
      if (cpu_animated || hardware_animated) {
        // These vertices were animated, so flash them red or blue.
        static const double flash_rate = 1.0;  // 1 state change per second
        int cycle = (int)(ClockObject::get_global_clock()->get_frame_time() * flash_rate);
        if ((cycle & 1) == 0) {
          _state = _state->compose(cpu_animated ? get_flash_cpu_state() : get_flash_hardware_state());
        }
      }
    }
#endif

    // If we have prepared it for skinning via the shader generator, mark a
    // flag on the state so that the shader generator will do this.  We should
    // probably find a cleaner way to do this.
    const ShaderAttrib *sattr;
    _state->get_attrib_def(sattr);
    if (sattr->auto_shader()) {
      if (anim_spec.get_animation_type() == Geom::AT_hardware) {
        const RenderState *state;
        if (anim_spec.get_num_transforms() == 4) {
          static CPT(RenderState) state4 = RenderState::make(
            DCAST(ShaderAttrib, ShaderAttrib::make())->set_hardware_skinning(true, 4));
          state = state4;
        } else if (anim_spec.get_num_transforms() == 8) {
          static CPT(RenderState) state8 = RenderState::make(
            DCAST(ShaderAttrib, ShaderAttrib::make())->set_hardware_skinning(true, 8));
          state = state8;
        } else {
          state = RenderState::make_empty();
        }

        // Compose it backwards so the flag still gets picked up if the higher
        // ShaderAttrib has an override value.
        _state = state->compose(_state);
      }

      gsg->ensure_generated_shader(_state);
      if (_state->_generated_shader != nullptr) {
        sattr = DCAST(ShaderAttrib, _state->_generated_shader);
      }
    }

    if (sattr != nullptr) {
      int count = sattr->get_instance_count();
      _num_instances = (count > 0) ? (size_t)count : 1;
    } else {
      _num_instances = 1;
    }

#if 0
    if (sattr != nullptr) {
      if (_instances != nullptr &&
          sattr->get_flag(ShaderAttrib::F_hardware_instancing)) {
        // We are under an InstancedNode, and the shader implements hardware.
        // Munge the instance list into the vertex data.
        munge_instances(current_thread);
        _num_instances = _instances->size();
        _instances = nullptr;
      } else {
        // No, use the instance count from the ShaderAttrib.
        int count = sattr->get_instance_count();
        _num_instances = (count > 0) ? (size_t)count : 1;
      }
    } else {
      _num_instances = 1;
    }
#endif
  }

  return true;
}

/**
 *
 */
void CullableObject::
output(std::ostream &out) const {
  if (_geom != nullptr) {
    out << *_geom;
  } else {
    out << "(null)";
  }
}

#if 0
/**
 * Returns a GeomVertexData that represents the results of computing the
 * instance arrays for this data.
 */
void CullableObject::
munge_instances(Thread *current_thread) {
  PStatTimer timer(_munge_instances_pcollector, current_thread);

  PT(GeomVertexData) instanced_data = new GeomVertexData(*_munged_data);
  const GeomVertexArrayFormat *array_format = GeomVertexArrayFormat::get_instance_array_format();

  CPT(GeomVertexArrayData) new_array = _instances->get_array_data(array_format);
  instanced_data->insert_array((size_t)-1, new_array);
  _munged_data = instanced_data;
}
#endif

/**
 * Returns a RenderState for flashing the object red, to show it is animated
 * by the CPU when show-vertex-animation is on.
 */
CPT(RenderState) CullableObject::
get_flash_cpu_state() {
  static const LColor flash_cpu_color(0.8f, 0.2, 0.2, 1.0f);

  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) flash_cpu_state = nullptr;
  if (flash_cpu_state == nullptr) {
    flash_cpu_state = RenderState::make
      (ColorAttrib::make_flat(flash_cpu_color));
  }

  return flash_cpu_state;
}

/**
 * Returns a RenderState for flashing the object blue, to show it is animated
 * by the hardware when show-vertex-animation is on.
 */
CPT(RenderState) CullableObject::
get_flash_hardware_state() {
  static const LColor flash_hardware_color(0.2, 0.2, 0.8, 1.0);

  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) flash_hardware_state = nullptr;
  if (flash_hardware_state == nullptr) {
    flash_hardware_state = RenderState::make
      (ColorAttrib::make_flat(flash_hardware_color));
  }

  return flash_hardware_state;
}
