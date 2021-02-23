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
#include "animPreloadTable.h"
#include "animChannel.h"

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
 * Creates and returns a new CharacterJoint with the indicated name and parent
 * joint.
 *
 * The pointer returned by this method may become invalid if another
 * joint is added to the character, so don't store it anywhere persistently.
 */
CharacterJoint *Character::
make_joint(const std::string &name, int parent) {
  CharacterJoint joint(name);
  joint._parent = parent;

  size_t joint_index = _joints.size();
  _joints.push_back(std::move(joint));

  return &_joints[joint_index];
}

/**
 * Creates and returns a new CharacterSlider with the indicated name.
 *
 * The pointer returned by this method may become invalid if another
 * slider is added to the character, so don't store it anywhere persistently.
 */
CharacterSlider *Character::
make_slider(const std::string &name) {
  CharacterSlider slider(name);
  size_t slider_index = _sliders.size();
  _sliders.push_back(std::move(slider));
  return &_sliders[slider_index];
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
    int joint_index = 0;
    BitArray bound_joints;
    find_bound_joints(joint_index, false, bound_joints, subset);
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

  bool any_changed = false;

  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now > cdata->_last_update + _update_delay || cdata->_anim_changed) {

    cdata->_anim_changed = false;
    cdata->_last_update = now;
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
  pick_channel_index(holes, channel_index);

  if (!holes.empty()) {
    channel_index = holes.front();
  }

  int joint_index = 0;
  BitArray bound_joints;
  if (subset.is_include_empty()) {
    bound_joints = BitArray::all_on();
  }
  bind_hierarchy(ptanim, channel_index, joint_index,
                 subset.is_include_empty(), bound_joints, subset);
  control->setup_anim(this, anim, channel_index, bound_joints);

  CDReader cdata(_cycler);
  determine_effective_channels(cdata);

  return true;
}

/**
 *
 */
bool Character::
check_hierarchy(const AnimBundle *anim, int hierarchy_match_flags) const {
  Thread::consider_yield();

  if (anim->get_num_joint_channels() != get_num_joints()) {
    anim_cat.error()
      << "Character " << get_name() << " and AnimBundle " << anim->get_name()
      << " have mismatching numbers of joints.\n";
    return false;
  }

  for (size_t i = 0; i < get_num_joints(); i++) {
    const CharacterJoint &joint = _joints[i];
    AnimChannelMatrix *chan = anim->get_joint_channel(i);

    if (joint.get_name() != chan->get_name()) {
      anim_cat.error()
        << "Joint " << i << " and channel " << i << "have mismatching names: "
        << joint.get_name() << " : " << chan->get_name() << "\n";
      return false;
    }
  }

  if (anim->get_num_slider_channels() != get_num_sliders()) {
    anim_cat.error()
      << "Character " << get_name() << " and AnimBundle " << anim->get_name()
      << " have mismatching numbers of sliders.\n";
    return false;
  }

  for (size_t i = 0; i < get_num_sliders(); i++) {
    const CharacterSlider &slider = _sliders[i];
    AnimChannelScalar *chan = anim->get_slider_channel(i);

    if (slider.get_name() != chan->get_name()) {
      anim_cat.error()
        << "Slider " << i << " and channel " << i << "have mismatching names: "
        << slider.get_name() << " : " << chan->get_name() << "\n";
      return false;
    }
  }

  return true;
}

/**
 *
 */
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
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

  manager->write_cdata(me, _cycler);
}

/**
 *
 */
Character::CData::
CData() :
  _frame_blend_flag(interpolate_frames),
  _anim_changed(false),
  _last_update(0.0),
  _root_xform(LMatrix4::ident_mat())
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
  _last_update(copy._last_update)
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
