/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.cxx
 * @author lachbr
 * @date 2021-02-22
 */

#include "character.h"
#include "bamReader.h"
#include "config_anim.h"
#include "bamWriter.h"
#include "clockObject.h"
#include "loader.h"
#include "animBundleNode.h"
#include "bindAnimRequest.h"

TypeHandle Character::_type_handle;

/**
 *
 */
Character::
Character(const std::string &name) :
  Namable(name),
  _update_delay(0.0)
{
}

/**
 * Copies the contents of the other Character's preload table into this one.
 */
void Character::
merge_anim_preloads(const Character *other) {
  if (other->_anim_preload == nullptr ||
      _anim_preload == other->_anim_preload) {
    // No-op.
    return;
  }

  if (_anim_preload == nullptr) {
    // Trivial case.
    _anim_preload = other->_anim_preload;
    return;
  }

  // Copy-on-write.
  PT(AnimPreloadTable) anim_preload = _anim_preload.get_write_pointer();
  anim_preload->add_anims_from(other->_anim_preload.get_read_pointer());
}


/**
 * Creates a new CharacterJoint with the indicated name and parent
 * joint, and returns the index of the new joint.
 */
int Character::
make_joint(const std::string &name, int parent, const LMatrix4 &default_value) {
  int index = (int)_joints.size();

  CharacterJoint joint(name);
  joint._parent = parent;
  joint._index = index;
  joint._default_value = default_value;
  joint._value = default_value;

  if (parent != -1) {
    _joints[parent]._children.push_back(joint._index);
  }

  _joints.push_back(joint);

  recompute_joint_net_transform(index);

  CharacterJoint &vjoint = _joints[index];
  vjoint._initial_net_transform_inverse = invert(vjoint._net_transform);

  return index;
}

/**
 * Creates a new CharacterSlider with the indicated name, and returns the index
 * of the new slider.
 */
int Character::
make_slider(const std::string &name, PN_stdfloat default_value) {
  CharacterSlider slider(name);
  slider._default_value = default_value;
  slider._value = default_value;
  size_t slider_index = _sliders.size();
  _sliders.push_back(slider);
  return slider_index;
}

/**
 * Binds the animation to the bundle, if possible, and returns a new
 * AnimControl that can be used to start and stop the animation.  If the anim
 * hierarchy does not match the part hierarchy, returns NULL.
 *
 * If hierarchy_match_flags is 0, only an exact match is accepted; otherwise,
 * it may contain a union of PartGroup::HierarchyMatchFlags values indicating
 * conditions that will be tolerated (but warnings will still be issued).
 *
 * If subset is specified, it restricts the binding only to the named subtree
 * of joints.
 *
 * The AnimControl is not stored within the PartBundle; it is the user's
 * responsibility to maintain the pointer.  The animation will automatically
 * unbind itself when the AnimControl destructs (i.e.  its reference count
 * goes to zero).
 */
PT(AnimControl) Character::
bind_anim(AnimBundle *anim, int hierarchy_match_flags,
          const PartSubset &subset) {
  PT(AnimControl) control = new AnimControl(anim->get_name(), this, 1.0f, 0);
  if (do_bind_anim(control, anim, hierarchy_match_flags, subset)) {
    return control;
  }

  return nullptr;
}

/**
 * Binds an animation to the bundle.  The animation is loaded from the disk
 * via the indicated Loader object.  In other respects, this behaves similarly
 * to bind_anim(), with the addition of asynchronous support.
 *
 * If allow_aysnc is true, the load will be asynchronous if possible.  This
 * requires that the animation basename can be found in the PartBundle's
 * preload table (see get_anim_preload()).
 *
 * In an asynchronous load, the animation file will be loaded and bound in a
 * sub-thread.  This means that the animation will not necessarily be
 * available at the time this method returns.  You may still use the returned
 * AnimControl immediately, though, but no visible effect will occur until the
 * animation eventually becomes available.
 *
 * You can test AnimControl::is_pending() to see if the animation has been
 * loaded yet, or wait for it to finish with AnimControl::wait_pending() or
 * even PartBundle::wait_pending().  You can also set an event to be triggered
 * when the animation finishes loading with
 * AnimControl::set_pending_done_event().
 */
PT(AnimControl) Character::
load_bind_anim(Loader *loader, const Filename &filename,
               int hierarchy_match_flags, const PartSubset &subset,
               bool allow_async) {
  nassertr(loader != nullptr, nullptr);

  LoaderOptions anim_options(LoaderOptions::LF_search |
                             LoaderOptions::LF_report_errors |
                             LoaderOptions::LF_convert_anim);
  std::string basename = filename.get_basename_wo_extension();

  int anim_index = -1;
  CPT(AnimPreloadTable) anim_preload = _anim_preload.get_read_pointer();
  if (anim_preload != nullptr) {
    anim_index = anim_preload->find_anim(basename);
  }

  if (anim_index < 0 || !allow_async || !Thread::is_threading_supported()) {
    // The animation is not present in the table, or allow_async is false.
    // Therefore, perform an ordinary synchronous load-and-bind.

    PT(PandaNode) model = loader->load_sync(filename, anim_options);
    if (model == nullptr) {
      // Couldn't load the file.
      return nullptr;
    }
    AnimBundle *anim = AnimBundleNode::find_anim_bundle(model);
    if (anim == nullptr) {
      // No anim bundle.
      return nullptr;
    }
    PT(AnimControl) control = bind_anim(anim, hierarchy_match_flags, subset);
    if (control == nullptr) {
      // Couldn't bind.
      return nullptr;
    }
    control->set_anim_model(model);
    return control;
  }

  // The animation is present in the table, so we can perform an asynchronous
  // load-and-bind.
  PN_stdfloat frame_rate = anim_preload->get_base_frame_rate(anim_index);
  int num_frames = anim_preload->get_num_frames(anim_index);
  PT(AnimControl) control =
    new AnimControl(basename, this, frame_rate, num_frames);

  if (!subset.is_include_empty()) {
    // Figure out the actual subset of joints to be bound.
    BitArray bound_joints;
    find_bound_joints(0, false, bound_joints, subset);
    control->set_bound_joints(bound_joints);
  }

  PT(BindAnimRequest) request =
    new BindAnimRequest(std::string("bind:") + filename.get_basename(),
                        filename, anim_options, loader, control,
                        hierarchy_match_flags, subset);
  request->set_priority(async_bind_priority);
  loader->load_async(request);

  return control;
}

/**
 * Updates all joints and sliders in the character to reflect the animation for
 * the current frame.
 *
 * Returns true if something in the character changed as a result of this,
 * false otherwise.
 */
bool Character::
update() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, false, current_thread);

  if (cdata->_anim_graph == nullptr) {
    return false;
  }

  bool any_changed = false;

  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now > cdata->_last_update + _update_delay || cdata->_anim_changed) {

    AnimGraphEvalContext ctx(_joints.data(), (int)_joints.size(), cdata->_frame_blend_flag);
    cdata->_anim_graph->evaluate(ctx);

    any_changed = apply_pose(cdata, ctx, current_thread);

    cdata->_anim_changed = false;
    cdata->_last_update = now;
  }

  return any_changed;
}

/**
 * Forces the character to update all joints and sliders to reflect the
 * animation for the current frame, regardless of whether we think it needs to.
 */
bool Character::
force_update() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, false, current_thread);

  if (cdata->_anim_graph == nullptr) {
    return false;
  }

  double now = ClockObject::get_global_clock()->get_frame_time();

  AnimGraphEvalContext ctx(_joints.data(), (int)_joints.size(), cdata->_frame_blend_flag);
  cdata->_anim_graph->evaluate(ctx);

  bool any_changed = apply_pose(cdata, ctx, current_thread);

  cdata->_anim_changed = false;
  cdata->_last_update = now;

  return any_changed;
}

/**
 * Recomputes the net transforms for all joints in the character.
 */
void Character::
recompute_joint_net_transforms() {
  for (size_t i = 0; i < _joints.size(); i++) {
    recompute_joint_net_transform(i);
  }
}

/**
 * Recomputes the net transforms for the indicated joint.
 */
void Character::
recompute_joint_net_transform(int i) {
  CharacterJoint &joint = _joints[i];

  if (joint._parent != -1) {
    joint._net_transform = joint._value * _joints[joint._parent]._net_transform;

  } else {
    joint._net_transform = joint._value * get_root_xform();
  }
}

/**
 *
 */

/**
 * Applies the final pose computed by the animation graph to each joint.
 */
bool Character::
apply_pose(CData *cdata, const AnimGraphEvalContext &context, Thread *current_thread) {
  //BitArray changed_mask;

  bool any_changed = false;

  size_t joint_count = _joints.size();
  for (size_t i = 0; i < joint_count; i++) {
    const JointTransform &xform = context._joints[i];
    CharacterJoint &joint = _joints[i];

    joint._value = LMatrix4::scale_mat(xform._scale) * xform._rotation;
    joint._value.set_row(3, xform._position);

    bool self_changed = false;
    bool net_changed = false;

    //if (value != joint._value) {
      //changed_mask.set_bit(i);
      //joint._value = std::move(value);
      self_changed = true;
      any_changed = true;
    //}

    if (joint._parent != -1) {
      bool parent_changed = true;//changed_mask.get_bit(joint._parent);

      if (parent_changed || self_changed) {
        CharacterJoint &parent_joint = _joints[joint._parent];
        joint._net_transform = joint._value * parent_joint._net_transform;
        net_changed = true;
      }
    } else {
      if (self_changed) {
        joint._net_transform = joint._value * cdata->_root_xform;
        net_changed = true;
      }
    }

    if (net_changed) {
      // TODO: Net expose joints!!!

      // Recompute the transform used by any vertices animated by this joint.
      joint._skinning_matrix = joint._initial_net_transform_inverse * joint._net_transform;

      // Also tell our related JointVertexTransforms that we've changed their
      // underlying matrix.
      CharacterJoint::VertexTransforms::iterator vti;
      for (vti = joint._vertex_transforms.begin(); vti != joint._vertex_transforms.end(); ++vti) {
        (*vti)->mark_modified(current_thread);
      }
    }

    if (self_changed) {
      // TODO: Local expose joints
    }

  }

  return any_changed;
}

/**
 * The internal implementation of bind_anim(), this receives a pointer to an
 * uninitialized AnimControl and fills it in if the bind is successful.
 * Returns true if successful, false otherwise.
 */
bool Character::
do_bind_anim(AnimControl *control, AnimBundle *anim,
             int hierarchy_match_flags, const PartSubset &subset) {
  nassertr(Thread::get_current_pipeline_stage() == 0, false);

  // Make sure this pointer doesn't destruct during the lifetime of this
  // method.
  PT(AnimBundle) ptanim = anim;

  if ((hierarchy_match_flags & HMF_ok_wrong_root_name) == 0) {
    // Make sure the root names match.
    if (get_name() != ptanim->get_name()) {
      if (anim_cat.is_error()) {
        anim_cat.error()
          << "Root name of part (" << get_name()
          << ") does not match that of anim (" << ptanim->get_name()
          << ")\n";
      }
      return false;
    }
  }

  if (!check_hierarchy(anim, hierarchy_match_flags)) {
    return false;
  }

  plist<int> holes;
  int channel_index = 0;
  pick_channel_index(0, holes, channel_index);

  if (!holes.empty()) {
    channel_index = holes.front();
  }

  BitArray bound_joints;
  if (subset.is_include_empty()) {
    bound_joints = BitArray::all_on();
  }
  bind_hierarchy(ptanim, channel_index, 0,
                 subset.is_include_empty(), bound_joints, subset);
  control->setup_anim(this, anim, channel_index, bound_joints);

  //CDReader cdata(_cycler);
  //determine_effective_channels(cdata);

  return true;
}

/**
 *
 */
bool Character::
check_hierarchy(const AnimBundle *anim, int hierarchy_match_flags) const {
  Thread::consider_yield();

  if (anim->get_num_joint_channels() > get_num_joints() &&
      (hierarchy_match_flags & HMF_ok_anim_extra) == 0) {

    anim_cat.error()
      << "AnimBundle " << anim->get_name() << " has more joint channels than joints on "
      << "Character " << get_name() << "\n";
    return false;

  } else if (get_num_joints() > anim->get_num_joint_channels() &&
             (hierarchy_match_flags & HMF_ok_part_extra) == 0) {
    anim_cat.error()
      << "Character " << get_name() << " has more joints than joint channels on "
      << "AnimBundle " << anim->get_name() << "\n";
    return false;
  }

  for (int i = 0; i < get_num_joints(); i++) {
    const CharacterJoint &joint = _joints[i];
    int chan = anim->find_joint_channel(joint.get_name());

    if (chan == -1) {
      anim_cat.warning()
        << "Joint " << joint.get_name() << " has no matching animation channel.\n";
    }
  }

  if (anim->get_num_slider_channels() > get_num_sliders() &&
      (hierarchy_match_flags & HMF_ok_anim_extra) == 0) {

    anim_cat.error()
      << "AnimBundle " << anim->get_name() << " has more slider channels than sliders on "
      << "Character " << get_name() << "\n";
    return false;

  } else if (get_num_sliders() > anim->get_num_slider_channels() &&
             (hierarchy_match_flags & HMF_ok_part_extra) == 0) {
    anim_cat.error()
      << "Character " << get_name() << " has more sliders than slider channels on "
      << "AnimBundle " << anim->get_name() << "\n";
    return false;
  }

  for (int i = 0; i < get_num_sliders(); i++) {
    const CharacterSlider &slider = _sliders[i];
    int chan = anim->find_slider_channel(slider.get_name());

    if (chan == -1) {
      anim_cat.warning()
        << "Slider " << slider.get_name() << " has no matching animation channel.\n";
    }
  }

  return true;
}

/**
 * Similar to bind_hierarchy, but does not actually perform any binding.  All
 * it does is compute the BitArray bount_joints according to the specified
 * subset.  This is useful in preparation for asynchronous binding--in this
 * case, we may need to know bound_joints immediately, without having to wait
 * for the animation itself to load and bind.
 */
void Character::
find_bound_joints(int n, bool is_included, BitArray &bound_joints, const PartSubset &subset) {
  CharacterJoint &joint = _joints[n];

  if (subset.matches_include(joint.get_name())) {
    is_included = true;
  } else if (subset.matches_exclude(joint.get_name())) {
    is_included = false;
  }

  bound_joints.set_bit_to(n, is_included);

  for (size_t i = 0; i < joint._children.size(); i++) {
    find_bound_joints(joint._children[i], is_included, bound_joints, subset);
  }
}

/**
 *
 */
void Character::
pick_channel_index(int n, plist<int> &holes, int &next) const {
  // Verify each of the holes.

  const CharacterJoint &joint = _joints[n];

  plist<int>::iterator ii, ii_next;
  ii = holes.begin();
  while (ii != holes.end()) {
    ii_next = ii;
    ++ii_next;

    int hole = (*ii);
    nassertv(hole >= 0 && hole < next);
    if (hole < (int)joint._channels.size() ||
        joint._channels[hole] != -1) {
      // We can't accept this hole; we're using it!
      holes.erase(ii);
    }
    ii = ii_next;
  }

  // Now do we have any more to restrict?
  if (next < (int)joint._channels.size()) {
    int i;
    for (i = next; i < (int)joint._channels.size(); i++) {
      if (joint._channels[i] == -1) {
        // Here's a hole we do have.
        holes.push_back(i);
      }
    }
    next = joint._channels.size();
  }

  for (size_t i = 0; i < joint._children.size(); i++) {
    pick_channel_index(joint._children[i], holes, next);
  }
}

/**
 * Binds the indicated anim hierarchy to the part hierarchy, at the given
 * channel index number.
 */
void Character::
bind_hierarchy(const AnimBundle *anim, int channel_index, int n,
               bool is_included, BitArray &bound_joints, const PartSubset &subset) {
  CharacterJoint &joint = _joints[n];
  int chan = anim->find_joint_channel(joint.get_name());

  if (subset.matches_include(joint.get_name())) {
    is_included = true;
  } else if (subset.matches_exclude(joint.get_name())) {
    is_included = false;
  }

  while ((int)joint._channels.size() <= channel_index) {
    joint._channels.push_back(-1);
  }

  nassertv(joint._channels[channel_index] == -1);

  if (is_included) {
    joint._channels[channel_index] = chan;

    // Record that we have bound this joint in the bound_joints BitArray.
    bound_joints.set_bit(n);
  } else {
    // Record that we have *not* bound this particular joint.
    bound_joints.clear_bit(n);
  }

  for (size_t i = 0; i < joint._children.size(); i++) {
    bind_hierarchy(anim, channel_index, joint._children[i], is_included, bound_joints, subset);
  }
}

/**
 *
 */
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void Character::
finalize(BamReader *) {
  //Thread *current_thread = Thread::get_current_thread();
  //CDWriter cdata(_cycler, true);
  //do_update(this, cdata, nullptr, true, true, current_thread);
}

/**
 *
 */
void Character::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());

  me.add_int16((int)_joints.size());
  for (int i = 0; i < (int)_joints.size(); i++) {
    _joints[i].write_datagram(me);
  }

  me.add_int16((int)_sliders.size());
  for (int i = 0; i < (int)_sliders.size(); i++) {
    _sliders[i].write_datagram(me);
  }

  manager->write_pointer(me, _anim_preload.get_read_pointer());

  manager->write_cdata(me, _cycler);
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int Character::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  if (manager->get_file_minor_ver() >= 17) {
    _anim_preload = DCAST(AnimPreloadTable, p_list[pi++]);
  }

  return pi;
}

/**
 *
 */
TypedWritable *Character::
make_from_bam(const FactoryParams &params) {
  Character *object = new Character("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);
  return object;
}

/**
 *
 */
void Character::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());

  _joints.resize(scan.get_int16());
  for (size_t i = 0; i < _joints.size(); i++) {
    _joints[i].read_datagram(scan);
  }

  _sliders.resize(scan.get_int16());
  for (size_t i = 0; i < _sliders.size(); i++) {
    _sliders[i].read_datagram(scan);
  }

  manager->read_pointer(scan);

  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
Character::CData::
CData() :
  _frame_blend_flag(interpolate_frames),
  _anim_changed(false),
  _last_update(0.0),
  _root_xform(LMatrix4::ident_mat()),
  _anim_graph(nullptr)
{
}

/**
 *
 */
Character::CData::
CData(const Character::CData &copy) :
  _frame_blend_flag(copy._frame_blend_flag),
  _root_xform(copy._root_xform),
  _anim_changed(copy._anim_changed),
  _last_update(copy._last_update),
  _anim_graph(copy._anim_graph)
{
  // Note that this copy constructor is not used by the PartBundle copy
  // constructor!  Any elements that must be copied between PartBundles should
  // also be explicitly copied there.
}

/**
 *
 */
CycleData *Character::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Character::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_bool(_frame_blend_flag);
  _root_xform.write_datagram(dg);

  // The remaining members are strictly dynamic.
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PartBundle.
 */
void Character::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _frame_blend_flag = scan.get_bool();
  _root_xform.read_datagram(scan);
}
