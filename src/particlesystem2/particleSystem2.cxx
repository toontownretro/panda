/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem2.cxx
 * @author brian
 * @date 2022-04-04
 */

#include "particleSystem2.h"
#include "particleManager2.h"
#include "virtualFileSystem.h"
#include "datagramOutputFile.h"
#include "bam.h"
#include "modelRoot.h"
#include "pdxElement.h"
#include "pdxList.h"
#include "pdxValue.h"
#include "characterNode.h"
#include "character.h"

TypeHandle ParticleSystem2::_type_handle;

/**
 *
 */
ParticleSystem2::
ParticleSystem2(const std::string &name) :
  Namable(name),
  _elapsed(0.0),
  _start_time(0.0),
  _running(false),
  _soft_stopped(false),
  _pool_size(256),
  _num_alive_particles(0),
  _dt(0.0),
  _prev_dt(0.05)
{
}

/**
 *
 */
ParticleSystem2::
ParticleSystem2(const ParticleSystem2 &copy) :
  Namable(copy),
  _pool_size(copy._pool_size),
  _elapsed(0.0),
  _start_time(0.0),
  _running(false),
  _soft_stopped(false),
  _num_alive_particles(0),
  _dt(0.0),
  _prev_dt(0.05),
  _initializers(copy._initializers),
  _emitters(copy._emitters),
  _children(copy._children),
  _functions(copy._functions),
  _forces(copy._forces),
  _constraints(copy._constraints),
  _renderers(copy._renderers)
{
}

/**
 *
 */
ParticleSystem2::
~ParticleSystem2() {
  for (int i = 0; i < _inputs.size(); ++i) {
    // Remove nodes that should only exist for the lifetime of
    // the system.
    if (_input_lifetime[i]) {
      _inputs[i].remove_node();
    }
  }
}

/**
 *
 */
PT(ParticleSystem2) ParticleSystem2::
make_copy() const {
  PT(ParticleSystem2) sys = new ParticleSystem2(*this);

  // We only need to deep copy emitters, renderers, and children.
  // Everything else is stateless and can be shared between systems.

  for (size_t i = 0; i < _emitters.size(); ++i) {
    sys->_emitters[i] = _emitters[i]->make_copy();
  }

  for (size_t i = 0; i < _renderers.size(); ++i) {
    sys->_renderers[i] = _renderers[i]->make_copy();
  }

  for (size_t i = 0; i < _children.size(); ++i) {
    sys->_children[i] = _children[i]->make_copy();
  }

  return sys;
}

/**
 * Sets the maximum number of particles that can be simulated simultaneously in
 * the system.  Memory for `size` particles is pre-allocated when the system
 * starts.
 *
 * The pool should be big enough to hold the maximum number of particles that
 * may be simulated at the same time, at any point in the simulation of the
 * system.  For performance and memory considerations, it is important to keep
 * the pool size no bigger than actually needed.
 */
void ParticleSystem2::
set_pool_size(int size) {
  nassertv(!_running);
  _pool_size = size;
}

/**
 * Adds a new emitter to the particle system.  Emitters are responsible for
 * determining when to spawn new particles, how often, and how many.
 */
void ParticleSystem2::
add_emitter(ParticleEmitter2 *emitter) {
  _emitters.push_back(emitter);
}

/**
 * Adds a new renderer to the particle system.  Renderers create a visual
 * representation of the particle system.
 */
void ParticleSystem2::
add_renderer(ParticleRenderer2 *renderer) {
  _renderers.push_back(renderer);
}

/**
 * Adds a new initializer to the particle system.  Initializers are
 * responsible for setting up the initial values of particle attributes
 * when they spawn, such as position and velocity.
 */
void ParticleSystem2::
add_initializer(ParticleInitializer2 *initializer) {
  _initializers.push_back(initializer);
}

/**
 * Adds a new function to the particle system.  Functions define the behavior
 * of particles in the system, such as how they move and change appearance
 * over time.
 */
void ParticleSystem2::
add_function(ParticleFunction2 *func) {
  _functions.push_back(func);
}

/**
 * Adds a new physical force to the particle system.  The force will act
 * upon all particles in the system.  Note that the system needs a
 * LinearMotionParticleFunction for forces to have any effect.
 */
void ParticleSystem2::
add_force(ParticleForce2 *force) {
  _forces.push_back(force);
}

/**
 * Adds a new physical constraint to the particle system.  The constraint
 * will limit the motion of particles in some way.  Note that the system
 * needs a LinearMotionParticleFunction for constraints to have any
 * effect.
 */
void ParticleSystem2::
add_constraint(ParticleConstraint2 *constraint) {
  _constraints.push_back(constraint);
}

/**
 * Adds the given particle system as a child of this particle system.
 * The child will be started and stopped along with this system, and input
 * nodes set on this system will propagate down to the child.
 */
void ParticleSystem2::
add_child(ParticleSystem2 *child) {
  _children.push_back(child);
}

/**
 * Adds a new input node to the particle system.  Initializers and functions
 * may use the transform of this node to influence their behaviors.
 *
 * By convention, the first input node defines the emission coordinate space.
 *
 * If system_lifetime is true, the node will be removed along with the particle
 * system.
 */
void ParticleSystem2::
add_input(const NodePath &input, bool system_lifetime) {
  _inputs.push_back(input);
  _input_values.push_back(TransformState::make_identity());
  _input_lifetime.push_back(system_lifetime);
  _input_hitboxes.push_back(nullptr);

  // Push down to children.
  for (ParticleSystem2 *child : _children) {
    child->add_input(input, false);
  }
}

/**
 *
 */
void ParticleSystem2::
set_input(int n, const NodePath &input, bool system_lifetime) {
  if (n >= _inputs.size()) {
    _inputs.resize(n + 1);
    _input_values.resize(n + 1);
    _input_lifetime.resize(n + 1);
    _input_hitboxes.resize(n + 1);
  }
  _inputs[n] = input;
  _input_values[n] = TransformState::make_identity();
  _input_lifetime[n] = system_lifetime;
  _input_hitboxes[n] = nullptr;

  // Push down to children.
  for (ParticleSystem2 *child : _children) {
    child->set_input(n, input, false);
  }
}

/**
 *
 */
void ParticleSystem2::
start(const NodePath &parent, const NodePath &follow_parent, double time) {
  if (priv_start(parent, follow_parent, time)) {
    ParticleManager2::get_global_ptr()->add_system(this);
  }
}

/**
 *
 */
bool ParticleSystem2::
priv_start(const NodePath &parent, const NodePath &follow_parent, double time) {
  if (_running) {
    return false;
  }

  nassertr(_pool_size > 0, false);

  _parent = parent;
  _follow_parent = follow_parent;
  _np = _parent.attach_new_node(get_name());
  if (!follow_parent.is_empty() && follow_parent != parent) {
    _np.set_pos(follow_parent.get_pos(parent));
  }

  _soft_stopped = false;

  _elapsed = time;
  _start_time = ClockObject::get_global_clock()->get_frame_time() - time;

  _num_alive_particles = 0;

  // Resize to contain pool size particles.
  _particles.resize(_pool_size);
  _free_particles.clear();
  for (int i = 0; i < (int)_particles.size(); ++i) {
    Particle *p = &_particles[i];
    p->_pos.fill(0.0f);
    p->_prev_pos.fill(0.0f);
    p->_velocity.fill(0.0f);
    p->_duration = 0.0f;
    p->_scale.fill(1.0f);
    p->_rotation_speed = 0.0f;
    p->_rotation = 0.0f;
    p->_color.fill(1.0f);
    p->_spawn_time = 0.0f;
    p->_anim_index = 0;
    // Shouldn't be necessary, but just to be safe.
    p->_id = (size_t)i;
    p->_alive = false;

    // At the start, all particles are dead and free for use.
    _free_particles.push_back(i);
  }

  // Initialize our renderers.
  for (ParticleRenderer2 *renderer : _renderers) {
    renderer->initialize(parent, this);
  }

  for (ParticleEmitter2 *emitter : _emitters) {
    emitter->initialize();
  }

  _running = true;

  // Start children.
  for (ParticleSystem2 *child : _children) {
    // Parent children systems to our NodePath.
    child->priv_start(_np, NodePath(), time);
  }

  return true;
}

/**
 *
 */
void ParticleSystem2::
soft_stop() {
  if (!_running) {
    return;
  }

  _soft_stopped = true;

  for (ParticleSystem2 *child : _children) {
    if (child->is_running()) {
      child->soft_stop();
    }
  }
}

/**
 *
 */
void ParticleSystem2::
stop() {
  if (!_running) {
    return;
  }

  priv_stop();
  ParticleManager2::get_global_ptr()->remove_system(this);
}

/**
 *
 */
void ParticleSystem2::
priv_stop() {
  nassertv(_running);

  // Shutdown our renderers.
  for (ParticleRenderer2 *renderer : _renderers) {
    renderer->shutdown(this);
  }

  _parent.clear();
  _follow_parent.clear();
  _np.remove_node();

  _running = false;
  _num_alive_particles = 0;
  _free_particles.clear();
  _soft_stopped = false;

  // Stop children.
  for (ParticleSystem2 *child : _children) {
    if (child->is_running()) {
      child->priv_stop();
    }
  }
}

/**
 * Main particle system update routine.
 */
bool ParticleSystem2::
update(double dt) {
  nassertr(_running, false);

  _dt = dt;

  nassertr(!_np.is_empty(), false);

  // If we have a follow parent, synchronize our position with the
  // follow parent, relative to our scene graph parent.
  if (!_follow_parent.is_empty() && _follow_parent != _parent) {
    _np.set_pos(_follow_parent.get_pos(_parent));
  }

  // Fetch current values of all dynamic input nodes.
  // This is the transform of the input node relative to the particle
  // system's render parent, or world-space as far as the particle system
  // is concerned.
  for (size_t i = 0; i < _inputs.size(); ++i) {
    if (!_inputs[i].is_empty()) {
      _input_values[i] = _inputs[i].get_transform(_np);
    } else {
      _input_values[i] = TransformState::make_identity();
    }
  }

  // Don't update emitters if the system was soft-stopped.
  if (!_soft_stopped) {
    // First update the emitter so they can birth new particles if necessary.
    for (ParticleEmitter2 *emitter : _emitters) {
      int birth_count = emitter->update(_elapsed);
      if (birth_count > 0) {
        // Emitter wants to birth some particles.
        birth_particles(birth_count);
      }
    }
  }

  // Run all functions over each alive particle.
  for (ParticleFunction2 *func : _functions) {
    func->update(_elapsed, dt, this);
  }

  // TEMPORARY: update renderers.  This should be deferred to a
  // cull callback or something.
  for (ParticleRenderer2 *renderer : _renderers) {
    renderer->update(this);
  }

  // Update children.
  for (ParticleSystem2 *child : _children) {
    if (child->is_running()) {
      child->update(dt);
    }
  }

  // Accumulate time.
  _elapsed += dt;

  _prev_dt = dt;

  if (_soft_stopped && _num_alive_particles == 0) {

    // Our soft-stop is complete, but don't actually stop
    // until all of our children have also completed their
    // soft-stops.
    bool all_children_done = true;
    for (ParticleSystem2 *child : _children) {
      if (child->is_running()) {
        all_children_done = false;
        break;
      }
    }

    if (all_children_done) {
      // Soft-stop complete.
      priv_stop();
      return false;
    }
  }

  return true;
}

/**
 * Kills the particle at the indicated index.
 */
void ParticleSystem2::
kill_particle(int n) {
  // Sanity check index.
  nassertv(n >= 0 && n < (int)_particles.size());

  // Shouldn't already be killed.
  nassertv(std::find(_free_particles.begin(), _free_particles.end(), n) == _free_particles.end());

  Particle *p = &_particles[n];
  nassertv(p->_alive);
  p->_alive = false;

  --_num_alive_particles;

  // Throw index into free queue to reuse this particle for later
  // births.
  _free_particles.push_back(n);
}

/**
 * Births/spawns the given number of particles into the system.  Reuses
 * particles from the free queue.
 *
 * Returns true if the given number of particles were available to be spawned,
 * or false otherwise.
 *
 * Runs each initializer of the system on the new particles.
 */
bool ParticleSystem2::
birth_particles(int count) {
  count = std::min(count, (int)_free_particles.size());
  if (count == 0) {
    return false;
  }

  int *indices = (int *)alloca(sizeof(int) * count);

  for (int i = 0; i < count; ++i) {
    // Grab first free particle from pool.
    int particle_index = _free_particles.back();
    _free_particles.pop_back();

    // Sanity check index.
    nassertr(particle_index >= 0 && particle_index < (int)_particles.size(), false);

    // Reset some data.
    Particle *p = &_particles[particle_index];
    p->_pos.set(0, 0, 0);
    p->_prev_pos.set(0, 0, 0);
    p->_velocity.set(0, 0, 0);
    p->_duration = 0.0f;
    p->_scale.set(1.0f, 1.0f);
    p->_rotation_speed = 0.0f;
    p->_rotation = 0.0f;
    p->_color.set(1, 1, 1, 1);
    p->_spawn_time = _elapsed;
    p->_anim_index = 0;
    // Shouldn't be necessary, but just to be safe.
    p->_id = (size_t)particle_index;
    p->_alive = true;

    ++_num_alive_particles;

    indices[i] = particle_index;
  }

  // Now run each initializer on the new particles.
  for (ParticleInitializer2 *init : _initializers) {
    init->init_particles(_elapsed, indices, count, this);
  }

  // Remember initial values for functions that want to lerp from
  // the chosen initial value, for instance.
  for (int i = 0; i < count; ++i) {
    Particle *p = &_particles[indices[i]];
    p->_initial_pos = p->_pos;
    p->_initial_vel = p->_velocity;
    p->_initial_scale = p->_scale;
    p->_initial_color = p->_color;
    p->_initial_rotation = p->_rotation;
    p->_initial_rotation_speed = p->_rotation_speed;
  }

  return true;
}

/**
 *
 */
void ParticleSystem2::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  me.add_int32(_pool_size);

  me.add_uint8(_emitters.size());
  for (ParticleEmitter2 *emitter : _emitters) {
    manager->write_pointer(me, emitter);
  }

  me.add_uint8(_initializers.size());
  for (ParticleInitializer2 *init : _initializers) {
    manager->write_pointer(me, init);
  }

  me.add_uint8(_functions.size());
  for (ParticleFunction2 *func : _functions) {
    manager->write_pointer(me, func);
  }

  me.add_uint8(_renderers.size());
  for (ParticleRenderer2 *renderer : _renderers) {
    manager->write_pointer(me, renderer);
  }

  me.add_uint8(_forces.size());
  for (ParticleForce2 *force : _forces) {
    manager->write_pointer(me, force);
  }

  me.add_uint8(_constraints.size());
  for (ParticleConstraint2 *constraint : _constraints) {
    manager->write_pointer(me, constraint);
  }

  me.add_uint8(_children.size());
  for (ParticleSystem2 *sys : _children) {
    manager->write_pointer(me, sys);
  }
}

/**
 *
 */
void ParticleSystem2::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  _pool_size = scan.get_int32();

  _emitters.resize(scan.get_uint8());
  manager->read_pointers(scan, _emitters.size());

  _initializers.resize(scan.get_uint8());
  manager->read_pointers(scan, _initializers.size());

  _functions.resize(scan.get_uint8());
  manager->read_pointers(scan, _functions.size());

  _renderers.resize(scan.get_uint8());
  manager->read_pointers(scan, _renderers.size());

  _forces.resize(scan.get_uint8());
  manager->read_pointers(scan, _forces.size());

  _constraints.resize(scan.get_uint8());
  manager->read_pointers(scan, _constraints.size());

  _children.resize(scan.get_uint8());
  manager->read_pointers(scan, _children.size());
}

/**
 *
 */
int ParticleSystem2::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _emitters.size(); ++i) {
    _emitters[i] = DCAST(ParticleEmitter2, p_list[pi++]);
  }

  for (size_t i = 0; i < _initializers.size(); ++i) {
    _initializers[i] = DCAST(ParticleInitializer2, p_list[pi++]);
  }

  for (size_t i = 0; i < _functions.size(); ++i) {
    _functions[i] = DCAST(ParticleFunction2, p_list[pi++]);
  }

  for (size_t i = 0; i < _renderers.size(); ++i) {
    _renderers[i] = DCAST(ParticleRenderer2, p_list[pi++]);
  }

  for (size_t i = 0; i < _forces.size(); ++i) {
    _forces[i] = DCAST(ParticleForce2, p_list[pi++]);
  }

  for (size_t i = 0; i < _constraints.size(); ++i) {
    _constraints[i] = DCAST(ParticleConstraint2, p_list[pi++]);
  }

  for (size_t i = 0; i < _children.size(); ++i) {
    _children[i] = DCAST(ParticleSystem2, p_list[pi++]);
  }

  return pi;
}

/**
 *
 */
TypedWritable *ParticleSystem2::
make_from_bam(const FactoryParams &params) {
  ParticleSystem2 *sys = new ParticleSystem2;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  sys->fillin(scan, manager);
  return sys;
}

/**
 *
 */
void ParticleSystem2::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
bool ParticleSystem2::
write_pto(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->delete_file(filename);

  DatagramOutputFile dout;
  if (!dout.open(filename)) {
    return false;
  }

  if (!dout.write_header(_bam_header)) {
    return false;
  }

  BamWriter writer(&dout);
  if (!writer.init()) {
    return false;
  }

  // Always write raw data if we're using this method.
  writer.set_file_material_mode(BamWriter::BTM_unchanged);

  if (!writer.write_object(this)) {
    return false;
  }

  return true;
}

/**
 *
 */
void ParticleSystem2::
update_input_hitboxes(int input) {
  nassertv(input >= 0 && input < (int)_input_hitboxes.size());

  PT(InputHitBoxCache) &cache = _input_hitboxes[input];
  if (cache == nullptr) {
    cache = load_input_hitboxes(input);
    if (cache == nullptr) {
      return;
    }
  }

  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now == cache->_last_update_time) {
    return;
  }

  cache->_last_update_time = now;

  CPT(TransformState) ts_char_to_ps_parent = cache->_character_np.get_transform(_np);
  const LMatrix4 &char_to_ps_parent = ts_char_to_ps_parent->get_mat();

  LMatrix4 ps_joint;

  for (size_t i = 0; i < cache->_hitboxes.size(); ++i) {
    HitBoxInfo &hbox = cache->_hitboxes[i];

    ps_joint = cache->_character->get_joint_net_transform(hbox._joint) * char_to_ps_parent;
    hbox._ps_mins = ps_joint.xform_point(hbox._mins);
    hbox._ps_maxs = ps_joint.xform_point(hbox._maxs);
  }
}

/**
 *
 */
PT(ParticleSystem2::InputHitBoxCache) ParticleSystem2::
load_input_hitboxes(int input) {
  const NodePath &np = get_input(input);
  nassertr(np.get_error_type() == NodePath::ET_ok, nullptr);
  ModelRoot *mdl_root;
  DCAST_INTO_R(mdl_root, np.node(), nullptr);

  PDXElement *data = mdl_root->get_custom_data();
  if (data == nullptr) {
    return nullptr;
  }

  if (!data->has_attribute("hit_boxes")) {
    return nullptr;
  }

  NodePath character_np = np.find("**/+CharacterNode");
  nassertr(!character_np.is_empty(), nullptr);
  Character *character = DCAST(CharacterNode, character_np.node())->get_character();

  PT(InputHitBoxCache) cache = new InputHitBoxCache;
  cache->_character = character;
  cache->_character_np = character_np;
  cache->_last_update_time = 0.0;

  PDXList *hit_boxes = data->get_attribute_value("hit_boxes").get_list();
  for (size_t i = 0; i < hit_boxes->size(); ++i) {
    PDXElement *hit_box = hit_boxes->get(i).get_element();

    HitBoxInfo hbox;
    hit_box->get_attribute_value("mins").to_vec3(hbox._mins);
    hit_box->get_attribute_value("maxs").to_vec3(hbox._maxs);
    hbox._joint = character->find_joint(hit_box->get_attribute_value("joint").get_string());
    cache->_hitboxes.push_back(hbox);
  }

  return cache;
}
