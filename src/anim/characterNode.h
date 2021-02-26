/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterNode.h
 * @author lachbr
 * @date 2021-02-23
 */

#ifndef CHARACTERNODE_H
#define CHARACTERNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "character.h"
#include "pointerTo.h"
#include "pStatCollector.h"
#include "lightMutex.h"

/**
 * Node that contains a Character, similar to an AnimBundleNode.
 */
class EXPCL_PANDA_ANIM CharacterNode final : public PandaNode {
private:
  CharacterNode();
  CharacterNode(const CharacterNode &copy);

PUBLISHED:
  CharacterNode(const std::string &name);

  void update();
  void force_update();

  INLINE Character *get_character() const;

public:
  virtual PandaNode *make_copy() const override;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;
  virtual bool safe_to_flatten() const override;
  virtual bool safe_to_combine() const override;
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread) const override;

private:
  void do_update();

  PT(Character) _char;

  double _last_auto_update;

  // Statistics
  PStatCollector _joints_pcollector;
  PStatCollector _skinning_pcollector;
  static PStatCollector _animation_pcollector;

  LightMutex _lock;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "CharacterNode",
                  PandaNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "characterNode.I"

#endif // CHARACTERNODE_H
