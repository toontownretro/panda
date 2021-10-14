/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterNode.cxx
 * @author lachbr
 * @date 2021-02-23
 */

#include "characterNode.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pStatTimer.h"
#include "config_anim.h"
#include "nodePath.h"
#include "geom.h"
#include "geomNode.h"
#include "eyeballNode.h"
#include "characterVertexSlider.h"

TypeHandle CharacterNode::_type_handle;

PStatCollector CharacterNode::_animation_pcollector("*:Animation");

/**
 * Private constructor used during Bam reading only.
 */
CharacterNode::
CharacterNode() :
  PandaNode(""),
  _char(nullptr),
  _last_auto_update(-1.0f)
{
  set_cull_callback();
}

/**
 *
 */
CharacterNode::
CharacterNode(const CharacterNode &copy) :
  PandaNode(copy),
  _joints_pcollector(copy._joints_pcollector),
  _skinning_pcollector(copy._skinning_pcollector),
  _last_auto_update(-1.0f)
{
  set_cull_callback();

  //LightMutexHolder holder(copy._lock);
  // Copy the underlying character.
  _char = copy._char->copy_subgraph();
  _char->add_node(this);
}

/**
 *
 */
CharacterNode::
CharacterNode(const std::string &name) :
  PandaNode(name),
  _joints_pcollector(PStatCollector(_animation_pcollector, name), "Joints"),
  _skinning_pcollector(PStatCollector(_animation_pcollector, name), "Vertices"),
  _char(new Character(name)),
  _last_auto_update(-1.0f)
{
  set_cull_callback();

  //LightMutexHolder holder(_lock);
  _char->add_node(this);
}

/**
 *
 */
CharacterNode::
~CharacterNode() {
  //LightMutexHolder holder(_lock);
  if (_char != nullptr) {
    _char->remove_node(this);
    _char = nullptr;
  }
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool CharacterNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // For now, we update the character during the cull traversal; this prevents
  // us from needlessly updating characters that aren't in the view frustum.
  // We may need a better way to do this optimization later, to handle
  // characters that might animate themselves in front of the view frustum.

  update();
  return true;
}

/**
 *
 */
bool CharacterNode::
safe_to_flatten() const {
  return false;
}

/**
 *
 */
bool CharacterNode::
safe_to_combine() const {
  return false;
}

/**
 *
 */
void CharacterNode::
xform(const LMatrix4 &mat) {
  if (_char != nullptr) {
    _char->xform(mat);
  }
}

/**
 *
 */
PandaNode *CharacterNode::
make_copy() const {
  return new CharacterNode(*this);
}

/**
 * This is used to support NodePath::calc_tight_bounds().  It is not intended
 * to be called directly, and it has nothing to do with the normal Panda
 * bounding-volume computation.
 *
 * If the node contains any geometry, this updates min_point and max_point to
 * enclose its bounding box.  found_any is to be set true if the node has any
 * geometry at all, or left alone if it has none.  This method may be called
 * over several nodes, so it may enter with min_point, max_point, and
 * found_any already set.
 *
 * This function is recursive, and the return value is the transform after it
 * has been modified by this node's transform.
 */
CPT(TransformState) CharacterNode::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  // This method is overridden by Character solely to provide a hook to force
  // the joints to update before computing the bounding volume.
  ((CharacterNode *)this)->update();

  // Unfortunately, calling update_to_now() will invalidate the node's cached
  // bounding volume, which causes a problem when this is called during the
  // traversal, e.g.  due to a ShowBoundsEffect.  As a hacky fix to work
  // around this, we will force-recompute all of the bounding volumes of our
  // parent nodes immediately.
  Parents parents = get_parents();
  for (size_t i = 0; i < parents.get_num_parents(); ++i) {
    PandaNode *parent = parents.get_parent(i);
    parent->get_bounds();
  }

  return PandaNode::calc_tight_bounds(min_point, max_point,
                                      found_any, transform, current_thread);
}

/**
 * This is called by r_copy_subgraph(); the copy has already been made of this
 * particular node (and this is the copy); this function's job is to copy all
 * of the children from the original.
 *
 * Note that it includes the parameter inst_map, which is a map type, and is
 * not (and cannot be) exported from PANDA.DLL.  Thus, any derivative of
 * PandaNode that is not also a member of PANDA.DLL *cannot* access this map,
 * and probably should not even override this function.
 */
void CharacterNode::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map,
                Thread *current_thread) {
  // We assume there will be no instancing going on below the Character node.
  // If there is, too bad; it will get flattened out.

  // We preempt the node's r_copy_children() operation with our own function
  // that keeps track of the old vs.  new nodes and also updates any Geoms we
  // find with our new dynamic vertices.

  const CharacterNode *from_char;
  DCAST_INTO_V(from_char, from);
  NodeMap node_map;

  LightMutexHolder from_holder(from_char->_lock);
  LightMutexHolder holder(_lock);

  //size_t num_bundles = _bundles.size();
  //nassertv(from_char->_bundles.size() == num_bundles);
  //for (size_t i = 0; i < num_bundles; ++i) {
  //  fill_joint_map(joint_map, _bundles[i]->get_bundle(), from_char->_bundles[i]->get_bundle());
  //}

  GeomVertexMap gvmap;
  GeomJointMap gjmap;
  GeomSliderMap gsmap;
  r_copy_char(this, from_char, from_char, node_map,
              gvmap, gjmap, gsmap);

  copy_node_pointers(node_map, _char, from_char->_char);
}

/**
 * Recursively walks the scene graph hierarchy below the Character node,
 * duplicating it while noting the orig:copy node mappings, and also updates
 * any GeomNodes found.
 *
 * Assumes the lock is held.
 */
void CharacterNode::
r_copy_char(PandaNode *dest, const PandaNode *source,
            const CharacterNode *from, CharacterNode::NodeMap &node_map,
            CharacterNode::GeomVertexMap &gvmap,
            CharacterNode::GeomJointMap &gjmap, CharacterNode::GeomSliderMap &gsmap) {

  if (source->is_geom_node()) {
    const GeomNode *source_gnode;
    GeomNode *dest_gnode;
    DCAST_INTO_V(source_gnode, source);
    DCAST_INTO_V(dest_gnode, dest);

    dest_gnode->remove_all_geoms();
    int num_geoms = source_gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      const Geom *geom = source_gnode->get_geom(i);
      const RenderState *state = source_gnode->get_geom_state(i);
      dest_gnode->add_geom(copy_geom(geom, gvmap, gjmap, gsmap), state);
    }
  }

  int num_children = source->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PandaNode *source_child = source->get_child(i);
    int source_sort = source->get_child_sort(i);

    PT(PandaNode) dest_child;
    if (source_child->is_of_type(Character::get_class_type())) {
      // We make a special case for nodes of type Character.  If we encounter
      // one of these, we have a Character under a Character, and the nested
      // Character's copy should be called instead of ours.
      dest_child = source_child->copy_subgraph();

    } else {
      // Otherwise, we assume that make_copy() will make a suitable copy of
      // the node.  This does limit the sorts of things we can have parented
      // to a Character and expect copy_subgraph() to work correctly.  Too
      // bad.
      dest_child = source_child->make_copy();
      r_copy_char(dest_child, source_child, from, node_map,
                  gvmap, gjmap, gsmap);
    }
    dest->add_child(dest_child, source_sort);
    node_map[source_child] = dest_child;
  }
}

/**
 * Makes a new copy of the Geom with the dynamic vertex arrays replaced to
 * reference this Character instead of the other one.  If no arrays have
 * changed, simply returns the same Geom.
 *
 * Assumes the lock is held.
 */
PT(Geom) CharacterNode::
copy_geom(const Geom *source,
          CharacterNode::GeomVertexMap &gvmap, CharacterNode::GeomJointMap &gjmap,
          CharacterNode::GeomSliderMap &gsmap) {
  CPT(GeomVertexFormat) format = source->get_vertex_data()->get_format();
  if (format->get_animation().get_animation_type() == Geom::AT_none) {
    // Not animated, so never mind.
    return (Geom *)source;
  }

  PT(Geom) dest = source->make_copy();

  CPT(GeomVertexData) orig_vdata = source->get_vertex_data();
  PT(GeomVertexData) new_vdata;
  GeomVertexMap::iterator gvmi = gvmap.find(orig_vdata);
  if (gvmi != gvmap.end()) {
    new_vdata = (*gvmi).second;
  } else {
    new_vdata = new GeomVertexData(*orig_vdata);

    new_vdata->set_transform_table(redirect_transform_table(orig_vdata->get_transform_table(), gjmap));
    new_vdata->set_transform_blend_table(redirect_transform_blend_table(orig_vdata->get_transform_blend_table(), gjmap));
    new_vdata->set_slider_table(redirect_slider_table(orig_vdata->get_slider_table(), gsmap));

    gvmap.insert(GeomVertexMap::value_type(orig_vdata, new_vdata));
  }

  dest->set_vertex_data(new_vdata);

  return dest;
}

/**
 * Creates _net_transform_nodes and _local_transform_nodes as appropriate in
 * each of the Character's joints, as copied from the other Character.
 */
void CharacterNode::
copy_node_pointers(const CharacterNode::NodeMap &node_map,
                   Character *dest, const Character *source) {
  nassertv(dest != source);

  // First handle any nodes below the character that link back up the the character.
  for (auto it = node_map.begin(); it != node_map.end(); ++it) {
    const PandaNode *source_node = (*it).first;
    PandaNode *dest_node = (*it).second;

    if (source_node->is_of_type(EyeballNode::get_class_type())) {
      nassertv(dest_node->is_of_type(EyeballNode::get_class_type()));
      const EyeballNode *source_eye = DCAST(EyeballNode, source_node);
      EyeballNode *dest_eye = DCAST(EyeballNode, dest_node);
      // Redirect the copied eye to the new character.
      dest_eye->set_character(dest, source_eye->get_parent_joint());
    }
  }

  // Handle attachment nodes.
  for (size_t i = 0; i < source->get_num_attachments(); i++) {
    PandaNode *source_node = source->get_attachment_node(i);
    if (source_node == nullptr) {
      continue;
    }
    auto it = node_map.find(source_node);
    if (it != node_map.end()) {
      PandaNode *dest_node = (*it).second;

      // Here's an internal joint that the source Character was animating
      // directly.  We'll animate our corresponding joint the same way.
      dest->set_attachment_node(i, dest_node);
    }
  }

  // Handle joint controller nodes.
  for (size_t i = 0; i < dest->get_num_joints(); i++) {
    CharacterJoint &dest_joint = dest->_joints[i];
    const CharacterJoint &src_joint = source->_joints[i];

    if (src_joint._controller != nullptr) {
      // Find the destination equivalent for the source controller node.
      auto it = node_map.find(src_joint._controller);
      if (it != node_map.end()) {
        dest_joint._controller = (*it).second;
      }
    }
  }
}

/**
 * Creates a new TransformTable, similar to the indicated one, with the joint
 * and slider pointers redirected into this object.
 */
CPT(TransformTable) CharacterNode::
redirect_transform_table(const TransformTable *source,
                         CharacterNode::GeomJointMap &gjmap) {
  if (source == nullptr) {
    return nullptr;
  }

  PT(TransformTable) dest = new TransformTable(*source);

  int num_transforms = dest->get_num_transforms();
  for (int i = 0; i < num_transforms; ++i) {
    const VertexTransform *vt = dest->get_transform(i);
    PT(JointVertexTransform) new_jvt = redirect_joint(vt, gjmap);
    if (new_jvt != nullptr) {
      dest->set_transform(i, new_jvt);
    }
  }

  return TransformTable::register_table(dest);
}

/**
 * Creates a new TransformBlendTable, similar to the indicated one, with the
 * joint and slider pointers redirected into this object.
 */
CPT(TransformBlendTable) CharacterNode::
redirect_transform_blend_table(const TransformBlendTable *source,
                               CharacterNode::GeomJointMap &gjmap) {
  if (source == nullptr) {
    return nullptr;
  }

  PT(TransformBlendTable) dest = new TransformBlendTable(*source);

  int num_blends = dest->get_num_blends();
  for (int i = 0; i < num_blends; ++i) {
    TransformBlend blend = dest->get_blend(i);
    int num_transforms = blend.get_num_transforms();
    for (int j = 0; j < num_transforms; ++j) {
      const VertexTransform *vt = blend.get_transform(j);
      PT(JointVertexTransform) new_jvt = redirect_joint(vt, gjmap);
      if (new_jvt != nullptr) {
        blend.set_transform(j, new_jvt);
      }
    }
    dest->set_blend(i, blend);
  }

  return dest;
}

/**
 * Creates a new SliderTable, similar to the indicated one, with the joint and
 * slider pointers redirected into this object.
 */
CPT(SliderTable) CharacterNode::
redirect_slider_table(const SliderTable *source,
                      CharacterNode::GeomSliderMap &gsmap) {
  if (source == nullptr) {
    return nullptr;
  }

  PT(SliderTable) dest = new SliderTable(*source);

  int num_sliders = dest->get_num_sliders();
  for (int i = 0; i < num_sliders; ++i) {
    const VertexSlider *vs = dest->get_slider(i);
    PT(CharacterVertexSlider) new_cvs = redirect_slider(vs, gsmap);
    if (new_cvs != nullptr) {
      dest->set_slider(i, new_cvs);
    }
  }

  return SliderTable::register_table(dest);
}

/**
 * Creates a new JointVertexTransform that is similar to the indicated one,
 * but points into this character.  If one was already created (in the
 * GeomJointMap), returns it instead.
 */
PT(JointVertexTransform) CharacterNode::
redirect_joint(const VertexTransform *vt,
               CharacterNode::GeomJointMap &gjmap) {
  GeomJointMap::iterator ji;
  ji = gjmap.find(vt);
  if (ji != gjmap.end()) {
    return (*ji).second;
  }

  PT(JointVertexTransform) new_jvt;

  if (vt->is_of_type(JointVertexTransform::get_class_type())) {
    const JointVertexTransform *jvt = DCAST(JointVertexTransform, vt);
    int orig_joint = jvt->get_joint();
    new_jvt = new JointVertexTransform(_char, orig_joint);
  }

  gjmap[vt] = new_jvt;
  return new_jvt;
}

/**
 * Creates a new CharacterVertexSlider that is similar to the indicated one,
 * but points into this character.  If one was already created (in the
 * GeomSliderMap), returns it instead.
 */
PT(CharacterVertexSlider) CharacterNode::
redirect_slider(const VertexSlider *vs, CharacterNode::GeomSliderMap &gsmap) {
  GeomSliderMap::iterator ji;
  ji = gsmap.find(vs);
  if (ji != gsmap.end()) {
    return (*ji).second;
  }

  PT(CharacterVertexSlider) new_cvs;

  if (vs->is_of_type(CharacterVertexSlider::get_class_type())) {
    const CharacterVertexSlider *cvs = DCAST(CharacterVertexSlider, vs);
    int slider = cvs->get_char_slider();
    new_cvs = new CharacterVertexSlider(_char, slider);
  }

  gsmap[vs] = new_cvs;
  return new_cvs;
}

/**
 * Recalculates the Character's joints and vertices for the current frame.
 * Normally this is performed automatically during the render and need not be
 * called explicitly.
 */
void CharacterNode::
update() {
  //LightMutexHolder holder(_lock);
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now != _last_auto_update) {
    _last_auto_update = now;

    if (anim_cat.is_spam()) {
      anim_cat.spam()
        << "Animating " << NodePath::any_path(this)
        << " at time " << now << "\n";
    }

    PStatTimer timer(_joints_pcollector);
    do_update();
  }
}

/**
 * Recalculates the character even if we think it doesn't need it.
 */
void CharacterNode::
force_update() {
  //LightMutexHolder holder(_lock);

  // Statistics
  PStatTimer timer(_joints_pcollector);

  _char->force_update();
}

/**
 * The actual implementation of update().  Assumes the appropriate
 * PStatCollector has already been started, and that the lock is held.
 */
void CharacterNode::
do_update() {
  // Update all the joints and sliders.
  if (even_animation) {
    _char->force_update();
  } else {
    _char->update();
  }
}

/**
 *
 */
void CharacterNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void CharacterNode::
write_datagram(BamWriter *manager, Datagram &me) {
  PandaNode::write_datagram(manager, me);
  manager->write_pointer(me, _char);
}

/**
 *
 */
int CharacterNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  _char = DCAST(Character, p_list[pi++]);

  return pi;
}

/**
 *
 */
TypedWritable *CharacterNode::
make_from_bam(const FactoryParams &params) {
  CharacterNode *object = new CharacterNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 *
 */
void CharacterNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);

#ifdef DO_PSTATS
  // Reinitialize our collectors with our name, now that we know it.
  if (has_name()) {
    _joints_pcollector =
      PStatCollector(PStatCollector(_animation_pcollector, get_name()), "Joints");
    _skinning_pcollector =
      PStatCollector(PStatCollector(_animation_pcollector, get_name()), "Vertices");
  }
#endif
}
