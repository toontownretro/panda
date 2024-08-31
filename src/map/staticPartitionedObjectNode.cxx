/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file staticPartitionedObjectNode.cxx
 * @author brian
 * @date 2022-03-10
 */

#include "staticPartitionedObjectNode.h"
#include "spatialPartition.h"
#include "ordered_vector.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "mapCullTraverser.h"
#include "omniBoundingVolume.h"
#include "preparedGraphicsObjects.h"
#include "geomTransformer.h"
#include "shaderAttrib.h"
#include "textureAttrib.h"
#include "materialAttrib.h"
#include "material.h"
#include "materialParamTexture.h"
#include "jobSystem.h"

IMPLEMENT_CLASS(StaticPartitionedObjectNode);

/**
 *
 */
StaticPartitionedObjectNode::
StaticPartitionedObjectNode(const std::string &name) :
  PandaNode(name),
  _trav_counter(-1),
  _cam_geoms_lock("cam-geoms-lock")
{
  set_renderable();
  set_bounds(new OmniBoundingVolume);
}

/**
 *
 */
void StaticPartitionedObjectNode::
add_object(GeomNode *node) {
  Object obj;
  obj._bounds = node->get_bounds();
  obj._last_trav_counter = -1;
  for (int i = 0; i < node->get_num_geoms(); ++i) {
    GeomEntry geom;
    geom._geom = node->get_geom(i);
    geom._state = node->get_geom_state(i);
    obj._geoms.push_back(std::move(geom));
  }
  _objects.push_back(std::move(obj));
}

/**
 * Places each object into spatial partition leaves.
 */
void StaticPartitionedObjectNode::
partition_objects(int num_leaves, const SpatialPartition *tree) {
  _leaf_objects.resize(num_leaves);

  for (size_t i = 0; i < _objects.size(); ++i) {
    ov_set<int> leaves;
    Object *obj = &_objects[i];
    if (obj->_bounds->is_of_type(BoundingSphere::get_class_type())) {
      const BoundingSphere *bsphere = (const BoundingSphere *)obj->_bounds.p();
      tree->get_leaf_values_containing_sphere(bsphere->get_center(), bsphere->get_radius(), leaves);

    } else if (obj->_bounds->is_of_type(BoundingBox::get_class_type())) {
      const BoundingBox *bbox = (const BoundingBox *)obj->_bounds.p();
      tree->get_leaf_values_containing_box(bbox->get_minq(), bbox->get_maxq(), leaves);
    }

    for (int leaf : leaves) {
      _leaf_objects[leaf].push_back(obj);
    }
  }
}

/**
 * Adds the node's contents to the CullResult we are building up during the
 * cull traversal, so that it will be drawn at render time.  For most nodes
 * other than GeomNodes, this is a do-nothing operation.
 */
void StaticPartitionedObjectNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  MapCullTraverser *mtrav = (MapCullTraverser *)trav;

  if (mtrav->_data == nullptr) {
    // No map, invalid view cluster, or culling disabled.

    for (const Object &obj : _objects) {
      add_object_for_draw(trav, data, &obj);
    }

    return;
  }

  if (mtrav->_view_cluster < 0) {
    return;
  }

  Camera *camera = trav->get_scene()->get_camera_node();
  int view_cluster = mtrav->_view_cluster;

  // I'm not expecting the same camera to be used for different display
  // regions being traversed in parallel, so only hold the lock on the
  // map access.
  _cam_geoms_lock.acquire();
  CamData *cam_data;
  CamGeoms::const_iterator it = _cam_geoms.find(camera);
  if (it != _cam_geoms.end()) {
    cam_data = (*it).second;
  } else {
    cam_data = new CamData;
    _cam_geoms.insert({ camera, cam_data });
  }
  _cam_geoms_lock.release();

  //JobSystem *jsys = JobSystem::get_global_ptr();

  if (cam_data->_view_cluster == view_cluster) {
    const TransformState *trans = trav->get_scene()->get_cs_world_transform();
    Thread *current_thread = trav->get_current_thread();
    // Same view cluster as before.
    // Zoom through cached object list.

    //jsys->parallel_process(cam_data._geoms.size(),
    //  [&] (int i) {
      //for (int i = 0; i < (int)cam_data._geoms.size(); ++i) {
      for (const Object *obj : cam_data->_geoms) {
        //const Object *obj = cam_data._geoms[i];
        if (data._view_frustum != nullptr) {
          if (!obj->_bounds->contains(data._view_frustum)) {
            continue;
          }
        }

        for (const GeomEntry &geom : obj->_geoms) {
          CPT(RenderState) state = data._state->compose(geom._state);
          CullableObject cobj(geom._geom, std::move(state), trans, current_thread);
          trav->get_cull_handler()->record_object(&cobj, trav);
        }
      }
    //);

  } else {
    // Camera changed clusters.
    //++_trav_counter;

    cam_data->_geoms.clear();
    cam_data->_view_cluster = view_cluster;

    const BitArray &pvs = mtrav->_pvs;
    int num_visgroups = (int)_leaf_objects.size();

    pflat_hash_set<Object *, pointer_hash> traversed;

    for (int i = 0; i < num_visgroups; ++i) {
      if (!pvs.get_bit(i)) {
        // Not in PVS.
        continue;
      }

      const pvector<Object *> &objects = _leaf_objects[i];

      for (Object *obj : objects) {
        auto ret = traversed.insert(obj);
        if (ret.second) {
          add_object_for_draw(trav, data, obj);
          cam_data->_geoms.push_back(obj);
        }
      }
    }
  }
}

/**
 *
 */
void StaticPartitionedObjectNode::
add_object_for_draw(CullTraverser *trav, CullTraverserData &data, const Object *obj) {
  if (data._view_frustum != nullptr) {
    if (!obj->_bounds->contains(data._view_frustum)) {
      return;
    }
  }

  Thread *current_thread = trav->get_current_thread();

  const TransformState *trans = trav->get_scene()->get_cs_world_transform();

  for (const GeomEntry &geom : obj->_geoms) {
    CPT(RenderState) state = data._state->compose(geom._state);
    CullableObject cobj(geom._geom, std::move(state), trans, current_thread);
    trav->get_cull_handler()->record_object(&cobj, trav);
  }
}

/**
 * The recursive implementation of prepare_scene(). Don't call this directly;
 * call PandaNode::prepare_scene() or NodePath::prepare_scene() instead.
 */
void StaticPartitionedObjectNode::
r_prepare_scene(GraphicsStateGuardianBase *gsg, const RenderState *node_state,
                GeomTransformer &transformer, Thread *current_thread) {
  PreparedGraphicsObjects *prepared_objects = gsg->get_prepared_objects();

  // Prepare the Geoms of each object.
  for (const Object &obj : _objects) {
    for (size_t gi = 0; gi < obj._geoms.size(); ++gi) {
      CPT(RenderState) geom_state = node_state->compose(obj._geoms[gi]._state);
      CPT(Geom) geom = obj._geoms[gi]._geom;

      // Munge the geom as required by the GSG.
      //PT(GeomMunger) munger = gsg->get_geom_munger(geom_state, current_thread);
      //geom = transformer.premunge_geom(geom, munger);

      // Prepare each of the vertex arrays in the munged Geom.
      CPT(GeomVertexData) vdata = geom->get_animated_vertex_data(false, current_thread);
      GeomVertexDataPipelineReader vdata_reader(vdata, current_thread);
      int num_arrays = vdata_reader.get_num_arrays();
      for (int i = 0; i < num_arrays; ++i) {
        CPT(GeomVertexArrayData) array = vdata_reader.get_array(i);
        prepared_objects->enqueue_vertex_buffer((GeomVertexArrayData *)array.p());
      }

      // And also each of the index arrays.
      int num_primitives = geom->get_num_primitives();
      for (int i = 0; i < num_primitives; ++i) {
        CPT(GeomPrimitive) prim = geom->get_primitive(i);
        prepared_objects->enqueue_index_buffer((GeomIndexArrayData *)prim->get_vertices().p());
      }

      // And the material's textures.
      const MaterialAttrib *mattr;
      if (geom_state->get_attrib(mattr)) {
        Material *mat = mattr->get_material();
        if (mat != nullptr) {
          for (size_t i = 0; i < mat->get_num_params(); ++i) {
            MaterialParamBase *param = mat->get_param(i);
            if (param->is_of_type(MaterialParamTexture::get_class_type())) {
              Texture *tex = DCAST(MaterialParamTexture, param)->get_value();
              if (tex != nullptr) {
                prepared_objects->enqueue_texture(tex);
                prepared_objects->enqueue_sampler(tex->get_default_sampler());
              }
            }
          }
        }
      }

      // And textures specified through TextureAttrib.
      const TextureAttrib *ta;
      if (geom_state->get_attrib(ta)) {
        int num_stages = ta->get_num_on_stages();
        for (int i = 0; i < num_stages; ++i) {
          Texture *texture = ta->get_on_texture(ta->get_on_stage(i));
          if (texture != nullptr) {
            prepared_objects->enqueue_texture(texture);
            prepared_objects->enqueue_sampler(texture->get_default_sampler());
          }
        }
      }

      // Determine the shader for this state, and prepare it.
      const ShaderAttrib *sa;
      geom_state->get_attrib_def(sa);
      if (sa->auto_shader()) {
        gsg->ensure_generated_shader(geom_state);
        if (geom_state->_generated_shader != nullptr) {
          sa = DCAST(ShaderAttrib, geom_state->_generated_shader);
        }
      }
      Shader *shader = (Shader *)sa->get_shader();
      if (shader != nullptr) {
        prepared_objects->enqueue_shader(shader);
      }
      // Prepare the texture shader inputs of the shader.
      for (auto it = sa->_texture_inputs.begin(); it != sa->_texture_inputs.end(); ++it) {
        const SamplerState *samp = &SamplerState::get_default();
        Texture *tex = sa->get_shader_input_texture((*it).first, samp);
        if (tex != nullptr) {
          prepared_objects->enqueue_texture(tex);
        }
        prepared_objects->enqueue_sampler(*samp);
      }
    }
  }

  PandaNode::r_prepare_scene(gsg, node_state, transformer, current_thread);
}
