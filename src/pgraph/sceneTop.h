/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneTop.h
 * @author brian
 * @date 2021-11-17
 */

#ifndef SCENETOP_H
#define SCENETOP_H

#include "pandabase.h"
#include "pandaNode.h"
#include "sceneVisibility.h"
#include "pointerTo.h"

/**
 * Intended to be used as the top or root node of the 3-D scene graph.
 * Contains data structures relating to the contents of the scene, such
 * as a visibility spatial search structure and lighting information.
 */
class EXPCL_PANDA_PGRAPH SceneTop : public PandaNode {
PUBLISHED:
  SceneTop(const std::string &name);

  INLINE void set_vis_info(SceneVisibility *vis_info);
  INLINE SceneVisibility *get_vis_info() const;

private:
  PT(SceneVisibility) _vis_info;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "SceneTop",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sceneTop.I"

#endif // SCENETOP_H
