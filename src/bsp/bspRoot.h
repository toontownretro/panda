/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspRoot.h
 * @author lachbr
 * @date 2021-01-02
 */

#ifndef BSPROOT_H
#define BSPROOT_H

#include "config_bsp.h"
#include "bspData.h"
#include "pandaNode.h"
#include "pointerTo.h"

/**
 * The top-level node of a scene graph created from a BSP file.  The only thing
 * special about this node is that it contains a pointer to the underlying BSP
 * data structures that were loaded from the BSP file.
 */
class EXPCL_PANDA_BSP BSPRoot : public PandaNode {
PUBLISHED:
  INLINE BSPRoot(const std::string &name);

  INLINE void set_bsp_data(BSPData *data);
  INLINE BSPData *get_bsp_data() const;
  MAKE_PROPERTY(bsp_data, get_bsp_data, set_bsp_data);

  virtual PandaNode *make_copy() const override;

protected:
  BSPRoot(const BSPRoot &copy);

public:
  virtual bool safe_to_flatten() const override;
  virtual bool safe_to_combine() const override;

private:
  PT(BSPData) _data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "BSPRoot",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class BSPLoader;
};

#include "bspRoot.I"

#endif // BSPROOT_H
